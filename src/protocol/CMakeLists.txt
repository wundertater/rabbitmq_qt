cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

find_package(Protobuf REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Messages.proto)
set(PROTO_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Messages.pb.cc)
set(PROTO_HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Messages.pb.h)

add_custom_command(
  OUTPUT ${PROTO_SRC_FILES} ${PROTO_HDR_FILES}
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${PROTO_FILE}
)

add_library(messages_protocol
    ${PROTO_SRC_FILES}
    ${PROTO_HDR_FILES}
)