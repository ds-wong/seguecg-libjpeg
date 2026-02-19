#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include <thread>

#include "common.cpp"

// This is the service implementation I provided earlier.
// It includes its own main in that snippet — for this file, we’ll
// embed the service class but control the lifecycle here.
#include <grpcpp/grpcpp.h>
#include "jpeglib.h"
#include "gen/grpc/libjpeg_grpc.pb.h"
#include "gen/grpc/libjpeg_grpc.grpc.pb.h"

static inline void* u64_to_ptr(uint64_t v) {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(v));
}

class LibjpegServiceImpl final : public libjpeg_grpc::LibjpegService::Service {
 public:
  void SetServer(grpc::Server* server) { server_ = server; }

  grpc::Status JpegStdError(grpc::ServerContext*,
                            const libjpeg_grpc::JpegStdErrorRequest* req,
                            libjpeg_grpc::JpegStdErrorReply* rep) override {
    auto* err = reinterpret_cast<jpeg_error_mgr*>(u64_to_ptr(req->err_addr()));
    auto* result = jpeg_std_error(err);
    rep->set_result_addr(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(result)));
    return grpc::Status::OK;
  }

  grpc::Status JpegCreateDecompress(grpc::ServerContext*,
                                    const libjpeg_grpc::JpegCreateDecompressRequest* req,
                                    libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_create_decompress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegMemSrc(grpc::ServerContext*,
                          const libjpeg_grpc::JpegMemSrcRequest* req,
                          libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    auto buffer = reinterpret_cast<unsigned char*>(u64_to_ptr(req->buffer_addr()));
    unsigned long size = static_cast<unsigned long>(req->size());
    jpeg_mem_src(cinfo, buffer, size);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegReadHeader(grpc::ServerContext*,
                              const libjpeg_grpc::JpegReadHeaderRequest* req,
                              libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    boolean require_image = req->require_image() ? TRUE : FALSE;
    jpeg_read_header(cinfo, require_image);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegStartDecompress(grpc::ServerContext*,
                                   const libjpeg_grpc::JpegStartDecompressRequest* req,
                                   libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_start_decompress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegReadScanlines(grpc::ServerContext*,
                                 const libjpeg_grpc::JpegReadScanlinesRequest* req,
                                 libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    auto buffer = reinterpret_cast<JSAMPARRAY>(u64_to_ptr(req->buffer_addr()));
    JDIMENSION max_lines = static_cast<JDIMENSION>(req->max_lines());
    jpeg_read_scanlines(cinfo, buffer, max_lines);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegFinishDecompress(grpc::ServerContext*,
                                    const libjpeg_grpc::JpegFinishDecompressRequest* req,
                                    libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_finish_decompress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegDestroyDecompress(grpc::ServerContext*,
                                     const libjpeg_grpc::JpegDestroyDecompressRequest* req,
                                     libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_decompress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_destroy_decompress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegCreateCompress(grpc::ServerContext*,
                                  const libjpeg_grpc::JpegCreateCompressRequest* req,
                                  libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_create_compress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegMemDest(grpc::ServerContext*,
                           const libjpeg_grpc::JpegMemDestRequest* req,
                           libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    auto outbuffer = reinterpret_cast<unsigned char**>(u64_to_ptr(req->outbuffer_addr()));
    auto outsize = reinterpret_cast<unsigned long*>(u64_to_ptr(req->outsize_addr()));
    jpeg_mem_dest(cinfo, outbuffer, outsize);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegSetDefaults(grpc::ServerContext*,
                               const libjpeg_grpc::JpegSetDefaultsRequest* req,
                               libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_set_defaults(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegSetQuality(grpc::ServerContext*,
                              const libjpeg_grpc::JpegSetQualityRequest* req,
                              libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    int quality = req->quality();
    boolean force_baseline = req->force_baseline() ? TRUE : FALSE;
    jpeg_set_quality(cinfo, quality, force_baseline);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegStartCompress(grpc::ServerContext*,
                                 const libjpeg_grpc::JpegStartCompressRequest* req,
                                 libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    boolean write_all_tables = req->write_all_tables() ? TRUE : FALSE;
    jpeg_start_compress(cinfo, write_all_tables);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegWriteScanlines(grpc::ServerContext*,
                                  const libjpeg_grpc::JpegWriteScanlinesRequest* req,
                                  libjpeg_grpc::JpegWriteScanlinesReply* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    auto scanlines = reinterpret_cast<JSAMPARRAY>(u64_to_ptr(req->scanlines_addr()));
    JDIMENSION num_lines = static_cast<JDIMENSION>(req->num_lines());
    JDIMENSION result = jpeg_write_scanlines(cinfo, scanlines, num_lines);
    rep->set_result(static_cast<uint32_t>(result));
    return grpc::Status::OK;
  }

  grpc::Status JpegFinishCompress(grpc::ServerContext*,
                                  const libjpeg_grpc::JpegFinishCompressRequest* req,
                                  libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_finish_compress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status JpegDestroyCompress(grpc::ServerContext*,
                                   const libjpeg_grpc::JpegDestroyCompressRequest* req,
                                   libjpeg_grpc::Ack* rep) override {
    auto cinfo = reinterpret_cast<j_compress_ptr>(u64_to_ptr(req->cinfo_addr()));
    jpeg_destroy_compress(cinfo);
    rep->set_ok(1);
    return grpc::Status::OK;
  }

  grpc::Status Terminate(grpc::ServerContext*,
                         const libjpeg_grpc::TerminateRequest*,
                         libjpeg_grpc::Ack* rep) override {
    rep->set_ok(1);

    std::cout<<"in terminate\n";
    if (server_) {
      std::cout << "requesting shutdown\n";
      std::thread([srv = server_]() {
        srv->Shutdown();   // runs after handler returns
      }).detach();
    }
    return grpc::Status::OK;
  }

 private:
  grpc::Server* server_ = nullptr;
};

int main(int argc, char** argv) {
    // Accept either:
    //   --sock /tmp/libjpeg_grpc.sock
    // or:
    //   /tmp/libjpeg_grpc.sock
    std::string sock_path = "/tmp/libjpeg_grpc.sock";

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--sock" && i + 1 < argc) {
            sock_path = argv[++i];
        } else if (!a.empty() && a[0] != '-') {
            // positional path
            sock_path = a;
        }
    }

    // gRPC wants unix: prefix
    std::string addr = "unix:" + sock_path;

    // remove stale socket file
    ::unlink(sock_path.c_str());

    // Set up shared memory first (same as your socket server)
    int shm_fd;
    void* shm_ptr;
    mspace shared_heap = shared_memory_setup(shm_fd, shm_ptr, true);
    (void)shared_heap;

    LibjpegServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout<<"after build and start\n";
    if (!server) {
        fprintf(stderr, "Failed to start gRPC server on %s\n", addr.c_str());
        munmap(shm_ptr, SHARED_MEM_SIZE);
        shm_unlink(SHARED_MEM_NAME);
        close(shm_fd);
        return 1;
    }
    service.SetServer(server.get());

    fprintf(stderr, "gRPC server listening on %s\n", addr.c_str());
    server->Wait();

    std::cout<<"server: after wait\n";

    // Cleanup (same as your socket server)
    munmap(shm_ptr, SHARED_MEM_SIZE);
    shm_unlink(SHARED_MEM_NAME);
    close(shm_fd);

    return 0;
}
