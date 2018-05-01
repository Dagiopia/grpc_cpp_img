cmake_minimum_required(VERSION 3.3)
project(grpc_img_proc)

find_package(Protobuf REQUIRED)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set(PROTO_PATH "${CMAKE_SOURCE_DIR}/protos")
set(PROTO_FILE_NAME "cv_img")
set(CV_IMG_PROTO "${PROTO_PATH}/${PROTO_FILE_NAME}.proto")
set(GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(CV_IMG_PB_CPP_FILE "${GENERATED_PROTOBUF_PATH}/${PROTO_FILE_NAME}.pb.cc")
set(CV_IMG_PB_H_FILE "${GENERATED_PROTOBUF_PATH}/${PROTO_FILE_NAME}.pb.h")
set(CV_IMG_GRPC_PB_CPP_FILE "${GENERATED_PROTOBUF_PATH}/${PROTO_FILE_NAME}.grpc.pb.cc")
set(CV_IMG_GRPC_PB_H_FILE "${GENERATED_PROTOBUF_PATH}/${PROTO_FILE_NAME}.grpc.pb.h")
add_custom_command(
	OUTPUT "${CV_IMG_PB_H_FILE}"
	"${CV_IMG_PB_CPP_FILE}"
	"${CV_IMG_GRPC_PB_H_FILE}"
	"${CV_IMG_GRPC_PB_CPP_FILE}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS "--proto_path=${PROTO_PATH}"
         "--cpp_out=${GENERATED_PROTOBUF_PATH}"
	 "${CV_IMG_PROTO}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS "--proto_path=${PROTO_PATH}"
         "--grpc_out=${GENERATED_PROTOBUF_PATH}"
         "--plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin"
	 "${CV_IMG_PROTO}"
    )
set(GENERATED_PROTOBUF_FILES ${CV_IMG_PB_H_FILE} ${CV_IMG_PB_CPP_FILE}
	${CV_IMG_GRPC_PB_H_FILE} ${CV_IMG_GRPC_PB_CPP_FILE})

include_directories(${GENERATED_PROTOBUF_PATH})
link_libraries(grpc++_unsecure grpc gpr ${PROTOBUF_LIBRARY})

add_executable(img_proc_server src/img_proc_server.cpp ${GENERATED_PROTOBUF_FILES})
add_executable(img_proc_client src/img_proc_client.cpp ${GENERATED_PROTOBUF_FILES})

target_link_libraries(img_proc_server ${OpenCV_LIBS})
target_link_libraries(img_proc_client ${OpenCV_LIBS})

configure_file(data/haarcascade_frontalface_alt.xml haarcascade_frontalface_alt.xml COPYONLY)
