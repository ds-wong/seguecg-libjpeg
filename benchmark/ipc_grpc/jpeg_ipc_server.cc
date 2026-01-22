#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>

#include <grpcpp/grpcpp.h>

#include "jpeg_ipc.pb.h"
#include "jpeg_ipc.grpc.pb.h"

// libjpeg headers: jpeglib.h is in repo root, jconfig.h is generated into build dir.
// We'll set include paths in CMake.
#include "jpeglib.h"

#define RELEASE_ASSERT(cond, msg) \
  do { if(!(cond)) { std::cerr << "FAILED: " << #cond << ". " << msg << "\n"; std::exit(1);} } while(0)

static void my_error_exit(j_common_ptr) {
  RELEASE_ASSERT(false, "my_error_exit exit handler called");
}

struct jpeg_parsed_data {
  JSAMPLE* image_buffer = nullptr;
  size_t   image_buffer_size = 0;
  int      image_height = 0;
  int      image_width = 0;
};

static jpeg_parsed_data read_jpeg(unsigned char* fileBuff, unsigned long fsize) {
  jpeg_parsed_data ret{};

  jpeg_decompress_struct cinfo{};
  jpeg_error_mgr jerr{};
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = my_error_exit;

  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, fileBuff, fsize);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  int row_stride = (int)(cinfo.output_width * cinfo.output_components);

  ret.image_height = (int)cinfo.output_height;
  ret.image_width  = (int)cinfo.output_width;
  ret.image_buffer_size = (size_t)ret.image_width * (size_t)ret.image_height * 3 * sizeof(JSAMPLE);
  ret.image_buffer = (JSAMPLE*) std::malloc(ret.image_buffer_size);
  RELEASE_ASSERT(ret.image_buffer, "Memory alloc failure");

  int curr_image_row = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    JSAMPLE* target = &(ret.image_buffer[curr_image_row * row_stride]);
    jpeg_read_scanlines(&cinfo, &target, 1);
    curr_image_row++;
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  return ret;
}

static jpeg_parsed_data write_jpeg(int quality, const jpeg_parsed_data& input) {
  jpeg_parsed_data ret{};

  jpeg_compress_struct cinfo{};
  jpeg_error_mgr jerr{};
  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);

  unsigned char* outbuffer = nullptr;
  unsigned long outsize = 0;
  jpeg_mem_dest(&cinfo, &outbuffer, &outsize);

  cinfo.image_width = input.image_width;
  cinfo.image_height = input.image_height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  int row_stride = input.image_width * 3;
  while (cinfo.next_scanline < cinfo.image_height) {
    JSAMPROW row_pointer[1];
    row_pointer[0] = &input.image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  ret.image_width = (int)cinfo.image_width;
  ret.image_height = (int)cinfo.image_height;
  ret.image_buffer_size = (size_t)outsize;
  ret.image_buffer = (JSAMPLE*)outbuffer; // libjpeg allocated this

  jpeg_destroy_compress(&cinfo);
  return ret;
}

class JpegRecodeService final : public jpegipc::JpegRecode::Service {
public:
  grpc::Status Recode(grpc::ServerContext*,
                      const jpegipc::RecodeRequest* req,
                      jpegipc::RecodeReply* rep) override
  {
    // Copy request bytes into a mutable buffer for libjpeg
    std::string in = req->input();
    auto* inbuf = (unsigned char*) std::malloc(in.size());
    RELEASE_ASSERT(inbuf, "malloc failed");
    std::memcpy(inbuf, in.data(), in.size());

    jpeg_parsed_data decoded = read_jpeg(inbuf, (unsigned long)in.size());
    std::free(inbuf);

    jpeg_parsed_data recoded = write_jpeg((int)req->quality(), decoded);

    // Reply
    rep->set_width((uint32_t)recoded.image_width);
    rep->set_height((uint32_t)recoded.image_height);
    rep->set_output((const char*)recoded.image_buffer, recoded.image_buffer_size);

    // Cleanup
    if (decoded.image_buffer) std::free(decoded.image_buffer);
    if (recoded.image_buffer) std::free(recoded.image_buffer); // outbuffer from jpeg_mem_dest

    return grpc::Status::OK;
  }
};

int main(int argc, char** argv) {
  std::string addr = "unix:///tmp/jpeg_ipc.sock";
  if (argc == 3 && std::string(argv[1]) == "--addr") addr = argv[2];

  JpegRecodeService svc;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&svc);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "jpeg_ipc_server listening on " << addr << "\n";
  server->Wait();
  return 0;
}
