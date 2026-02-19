#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <unistd.h>
#include <sys/mman.h>

#include "common.cpp"
#include "jpeglib.h"

// Your generated test data header
#include "../test_bytes.h"   // adjust if needed: "test_bytes.h"

// gRPC client wrappers (the file I gave earlier)
#include "libjpeg_client_grpc.cpp"

using namespace std::chrono;

#define TEST_ITERATIONS 100

struct jpeg_parsed_data {
    JSAMPLE* image_buffer;
    size_t image_buffer_size;
    int image_height;
    int image_width;
};

// --------- These two functions are almost identical to your socket version,
// except they call client.ipc_* methods instead of ipc_*(server_fd, ...).

static struct jpeg_parsed_data read_jpeg(LibjpegGrpcClient &client,
                                        mspace shared_heap,
                                        unsigned char *fileBuff,
                                        unsigned long fsize) {
    unsigned char* shared_input = (unsigned char*)mspace_malloc(shared_heap, fsize);
    memcpy(shared_input, fileBuff, fsize);

    struct jpeg_parsed_data ret = {0};

    auto* cinfo = (struct jpeg_decompress_struct*)mspace_malloc(shared_heap, sizeof(struct jpeg_decompress_struct));
    memset(cinfo, 0, sizeof(struct jpeg_decompress_struct));

    auto* jerr = (struct jpeg_error_mgr*)mspace_malloc(shared_heap, sizeof(struct jpeg_error_mgr));
    memset(jerr, 0, sizeof(struct jpeg_error_mgr));

    cinfo->err = client.ipc_jpeg_std_error(jerr);

    client.ipc_jpeg_create_decompress(cinfo);

    client.ipc_jpeg_mem_src(cinfo, shared_input, fsize);
    client.ipc_jpeg_read_header(cinfo, TRUE);
    client.ipc_jpeg_start_decompress(cinfo);

    int row_stride = cinfo->output_width * cinfo->output_components;

    ret.image_height = cinfo->output_height;
    ret.image_width  = cinfo->output_width;
    ret.image_buffer_size = ret.image_width * ret.image_height * 3 * sizeof(JSAMPLE);
    ret.image_buffer = (JSAMPLE*)mspace_malloc(shared_heap, ret.image_buffer_size);

    RELEASE_ASSERT(ret.image_buffer, "Memory alloc failure");

    int curr_image_row = 0;

    JSAMPLE** row_pointer = (JSAMPLE**)mspace_malloc(shared_heap, sizeof(JSAMPLE*));

    while (cinfo->output_scanline < cinfo->output_height) {
        JSAMPLE* target = &(ret.image_buffer[curr_image_row * row_stride]);
        *row_pointer = target;
        client.ipc_jpeg_read_scanlines(cinfo, row_pointer, 1);
        curr_image_row++;
    }

    client.ipc_jpeg_finish_decompress(cinfo);
    client.ipc_jpeg_destroy_decompress(cinfo);

    mspace_free(shared_heap, row_pointer);
    mspace_free(shared_heap, jerr);
    mspace_free(shared_heap, cinfo);
    mspace_free(shared_heap, shared_input);

    return ret;
}

static struct jpeg_parsed_data write_jpeg(LibjpegGrpcClient &client,
                                         mspace shared_heap,
                                         int quality,
                                         struct jpeg_parsed_data input) {
    struct jpeg_parsed_data ret = {0};

    auto* cinfo = (struct jpeg_compress_struct*)mspace_malloc(shared_heap, sizeof(struct jpeg_compress_struct));
    memset(cinfo, 0, sizeof(struct jpeg_compress_struct));

    auto* jerr = (struct jpeg_error_mgr*)mspace_malloc(shared_heap, sizeof(struct jpeg_error_mgr));
    memset(jerr, 0, sizeof(struct jpeg_error_mgr));

    cinfo->err = client.ipc_jpeg_std_error(jerr);
    client.ipc_jpeg_create_compress(cinfo);

    // Hardcoded based on known test output size (same as your current code)
    const int output_data_size = 126705;

    unsigned char** outbuffer_ptr = (unsigned char**)mspace_malloc(shared_heap, sizeof(unsigned char*));
    unsigned long* outsize_ptr    = (unsigned long*)mspace_malloc(shared_heap, sizeof(unsigned long));
    *outbuffer_ptr = (unsigned char*)mspace_malloc(shared_heap, output_data_size);
    *outsize_ptr   = output_data_size;

    client.ipc_jpeg_mem_dest(cinfo, outbuffer_ptr, outsize_ptr);

    cinfo->image_width = input.image_width;
    cinfo->image_height = input.image_height;
    cinfo->input_components = 3;
    cinfo->in_color_space = JCS_RGB;

    client.ipc_jpeg_set_defaults(cinfo);
    client.ipc_jpeg_set_quality(cinfo, quality, TRUE);
    client.ipc_jpeg_start_compress(cinfo, TRUE);

    JSAMPLE** row_pointer = (JSAMPLE**)mspace_malloc(shared_heap, sizeof(JSAMPLE*));
    int row_stride = input.image_width * 3;

    while (cinfo->next_scanline < cinfo->image_height) {
        *row_pointer = &input.image_buffer[cinfo->next_scanline * row_stride];
        client.ipc_jpeg_write_scanlines(cinfo, row_pointer, 1);
    }

    client.ipc_jpeg_finish_compress(cinfo);

    ret.image_width = cinfo->image_width;
    ret.image_height = cinfo->image_height;
    ret.image_buffer_size = *outsize_ptr;
    ret.image_buffer = *outbuffer_ptr;

    client.ipc_jpeg_destroy_compress(cinfo);

    mspace_free(shared_heap, row_pointer);
    mspace_free(shared_heap, jerr);
    mspace_free(shared_heap, cinfo);
    mspace_free(shared_heap, outsize_ptr);
    mspace_free(shared_heap, outbuffer_ptr);

    return ret;
}

// ---------------- main ----------------

int main(int argc, char** argv) {
    std::string sock_path = "/tmp/libjpeg_grpc.sock";

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--sock" && i + 1 < argc) {
            sock_path = argv[++i];
        } else if (!a.empty() && a[0] != '-') {
            sock_path = a;
        }
    }

    std::string addr = "unix:" + sock_path;

    auto channel = MakeUdsChannel(addr);
    LibjpegGrpcClient client(channel);

    unsigned long input_size  = sizeof(inputData) - 1;
    unsigned long output_size = sizeof(outputData) - 1;

    int shm_fd;
    void* shm_ptr;
    mspace shared_heap = shared_memory_setup(shm_fd, shm_ptr, false);

    struct jpeg_parsed_data in_jpeg_data {0}, out_jpeg_data {0};

    auto enter_time = high_resolution_clock::now();

    std::cout<<"before loop\n";

    for (int i = 0; i < TEST_ITERATIONS; i++) {
        // std::cout<<"here " << i <<"\n";
        if (i > 0) {
            mspace_free(shared_heap, in_jpeg_data.image_buffer);
            mspace_free(shared_heap, out_jpeg_data.image_buffer);
        }
        // std::cout<<"before read_jpeg\n";
        in_jpeg_data  = read_jpeg(client, shared_heap, inputData, input_size);
        // std::cout<<"before write_jpeg\n";
        out_jpeg_data = write_jpeg(client, shared_heap, 30, in_jpeg_data);
    }

    auto exit_time = high_resolution_clock::now();

    std::cout<<"exited loop\n";

    client.ipc_terminate();

    std::cout<<"after terminate\n";

    // Validation (same as your socket version)
    RELEASE_ASSERT(output_size == out_jpeg_data.image_buffer_size, "Size mismatch");
    for (unsigned long i = 0; i < output_size; i++) {
        if (out_jpeg_data.image_buffer[i] != outputData[i]) {
            printf("Output data doesn't match at index: %lu!\n", i);
            exit(1);
        }
    }

    std::cout<<"after terminate\n";

    int64_t ns = duration_cast<nanoseconds>(exit_time - enter_time).count();
    printf("IPC JPEG recoding time (gRPC): %lld\n", (long long)(ns / TEST_ITERATIONS));

    std::cout<<"after printing jpeg\n";
    // Cleanup
    close(shm_fd);
    munmap(shm_ptr, SHARED_MEM_SIZE);

    return 0;
}
