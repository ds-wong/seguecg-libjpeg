#include <memory>
#include <string>
#include <stdint.h>

#include <grpcpp/grpcpp.h>

#include "jpeglib.h"

// Generated headers
#include "gen/grpc/libjpeg_grpc.pb.h"
#include "gen/grpc/libjpeg_grpc.grpc.pb.h"

// Mirror your RELEASE_ASSERT style if you want
#include <cstdio>
#include <cstdlib>

#define RELEASE_ASSERT(cond, fmt, ...)                                  \
  do {                                                                  \
    if (!(cond)) {                                                      \
      fprintf(stderr, "FAILED: %s. " fmt "\n", #cond, ##__VA_ARGS__);    \
      exit(1);                                                          \
    }                                                                   \
  } while (0)


static inline uint64_t ptr_to_u64(const void* p) {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(p));
}

class LibjpegGrpcClient {
 public:
  explicit LibjpegGrpcClient(const std::shared_ptr<grpc::Channel>& ch)
      : stub_(libjpeg_grpc::LibjpegService::NewStub(ch)) {}

  jpeg_error_mgr* ipc_jpeg_std_error(jpeg_error_mgr* err) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegStdErrorRequest req;
    libjpeg_grpc::JpegStdErrorReply rep;

    req.set_err_addr(ptr_to_u64(err));

    auto status = stub_->JpegStdError(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());

    return reinterpret_cast<jpeg_error_mgr*>(static_cast<uintptr_t>(rep.result_addr()));
  }

  void ipc_jpeg_create_decompress(j_decompress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegCreateDecompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegCreateDecompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_mem_src(j_decompress_ptr cinfo, unsigned char* buffer, unsigned long size) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegMemSrcRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_buffer_addr(ptr_to_u64(buffer));
    req.set_size(static_cast<uint64_t>(size));

    auto status = stub_->JpegMemSrc(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_read_header(j_decompress_ptr cinfo, boolean require_image) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegReadHeaderRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_require_image(require_image ? true : false);

    auto status = stub_->JpegReadHeader(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_start_decompress(j_decompress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegStartDecompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegStartDecompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY buffer, JDIMENSION max_lines) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegReadScanlinesRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_buffer_addr(ptr_to_u64(buffer));
    req.set_max_lines(static_cast<uint32_t>(max_lines));

    auto status = stub_->JpegReadScanlines(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_finish_decompress(j_decompress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegFinishDecompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegFinishDecompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_destroy_decompress(j_decompress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegDestroyDecompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegDestroyDecompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_create_compress(j_compress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegCreateCompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegCreateCompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_mem_dest(j_compress_ptr cinfo, unsigned char** outbuffer, unsigned long* outsize) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegMemDestRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_outbuffer_addr(ptr_to_u64(outbuffer));
    req.set_outsize_addr(ptr_to_u64(outsize));

    auto status = stub_->JpegMemDest(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_set_defaults(j_compress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegSetDefaultsRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegSetDefaults(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegSetQualityRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_quality(quality);
    req.set_force_baseline(force_baseline ? true : false);

    auto status = stub_->JpegSetQuality(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegStartCompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_write_all_tables(write_all_tables ? true : false);

    auto status = stub_->JpegStartCompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  JDIMENSION ipc_jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegWriteScanlinesRequest req;
    libjpeg_grpc::JpegWriteScanlinesReply rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));
    req.set_scanlines_addr(ptr_to_u64(scanlines));
    req.set_num_lines(static_cast<uint32_t>(num_lines));

    auto status = stub_->JpegWriteScanlines(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());

    return static_cast<JDIMENSION>(rep.result());
  }

  void ipc_jpeg_finish_compress(j_compress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegFinishCompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegFinishCompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_jpeg_destroy_compress(j_compress_ptr cinfo) {
    grpc::ClientContext ctx;
    libjpeg_grpc::JpegDestroyCompressRequest req;
    libjpeg_grpc::Ack rep;

    req.set_cinfo_addr(ptr_to_u64(cinfo));

    auto status = stub_->JpegDestroyCompress(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

  void ipc_terminate() {
    grpc::ClientContext ctx;
    libjpeg_grpc::TerminateRequest req;
    libjpeg_grpc::Ack rep;

    std::cout<<"in ipc_terminate\n";

    auto status = stub_->Terminate(&ctx, req, &rep);
    RELEASE_ASSERT(status.ok(), "%s", status.error_message().c_str());
  }

 private:
  std::unique_ptr<libjpeg_grpc::LibjpegService::Stub> stub_;
};

// Helper to create a unix-domain-socket channel
static inline std::shared_ptr<grpc::Channel> MakeUdsChannel(const std::string& uds_path) {
  // uds_path should look like: "unix:/tmp/whatever.sock"
  grpc::ChannelArguments args;
  return grpc::CreateCustomChannel(uds_path, grpc::InsecureChannelCredentials(), args);
}
