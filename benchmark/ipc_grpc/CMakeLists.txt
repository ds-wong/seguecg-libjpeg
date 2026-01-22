cmake_minimum_required(VERSION 3.16)
project(jpeg_ipc_grpc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Paths
set(BENCH_DIR ${CMAKE_CURRENT_LIST_DIR}/..)            # .../benchmark
set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)         # .../seguecg-libjpeg
set(JPEG_BUILD ${REPO_ROOT}/build_nosimd_release)
set(JPEG_LIB   ${JPEG_BUILD}/libjpeg.a)

# ---- Protobuf/gRPC codegen ----
set(PROTO ${CMAKE_CURRENT_LIST_DIR}/jpeg_ipc.proto)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(PB_CC  ${GEN_DIR}/jpeg_ipc.pb.cc)
set(PB_H   ${GEN_DIR}/jpeg_ipc.pb.h)
set(GRPC_CC ${GEN_DIR}/jpeg_ipc.grpc.pb.cc)
set(GRPC_H  ${GEN_DIR}/jpeg_ipc.grpc.pb.h)

add_custom_command(
  OUTPUT ${PB_CC} ${PB_H} ${GRPC_CC} ${GRPC_H}
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
  ARGS --proto_path=${CMAKE_CURRENT_LIST_DIR}
       --cpp_out=${GEN_DIR}
       --grpc_out=${GEN_DIR}
       --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
       ${PROTO}
  DEPENDS ${PROTO}
  VERBATIM
)

add_library(jpeg_ipc_proto
  ${PB_CC}
  ${GRPC_CC}
)
target_include_directories(jpeg_ipc_proto PUBLIC ${GEN_DIR})
target_link_libraries(jpeg_ipc_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

# ---- Server ----
add_executable(jpeg_ipc_server jpeg_ipc_server.cc)
target_link_libraries(jpeg_ipc_server PRIVATE jpeg_ipc_proto ${JPEG_LIB})
target_include_directories(jpeg_ipc_server PRIVATE
  ${GEN_DIR}
  ${REPO_ROOT}          # for jpeglib.h (in source tree)
  ${JPEG_BUILD}         # for jconfig.h (generated)
)
target_link_libraries(jpeg_ipc_server PRIVATE m pthread)

# ---- Client (benchmark driver) ----
add_executable(jpeg_ipc_client jpeg_ipc_client.cc)
target_link_libraries(jpeg_ipc_client PRIVATE jpeg_ipc_proto)
target_include_directories(jpeg_ipc_client PRIVATE
  ${GEN_DIR}
  ${BENCH_DIR}          # for ../test_bytes.h include
)
target_link_libraries(jpeg_ipc_client PRIVATE pthread)
