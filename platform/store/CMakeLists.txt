# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

add_subdirectory(${DIAMOND_SOURCE_DIR}/store/common)

PROTOBUF_GENERATE_CPP(SRC_DIAMOND_STORE_PROTO HDR_DIAMOND_STORE_PROTO
  ${DIAMOND_SOURCE_DIR}/store/store-proto.proto)

set(SRC_DIAMOND_STORE_PROTO
  ${SRC_DIAMOND_STORE_PROTO}
  ${SRC_DIAMOND_STORE_COMMON_PROTO}
  PARENT_SCOPE)
set(HDR_DIAMOND_STORE_PROTO
  ${HDR_DIAMOND_STORE_PROTO}
  ${HDR_DIAMOND_STORE_COMMON_PROTO}
  PARENT_SCOPE)

set(SRC_DIAMOND_STORE_CLIENT
    ${SRC_DIAMOND_STORE_COMMON_CLIENT}
    ${SRC_DIAMOND_STORE_PROTO}
    ${HDR_DIAMOND_STORE_PROTO}
    ${DIAMOND_SOURCE_DIR}/store/client.cc
    ${DIAMOND_SOURCE_DIR}/store/shardclient.cc
    PARENT_SCOPE)

set(SRC_DIAMOND_STORE_SERVER
    ${SRC_DIAMOND_STORE_COMMON_SERVER}
    ${SRC_DIAMOND_STORE_PROTO}
    ${HDR_DIAMOND_STORE_PROTO}
    ${DIAMOND_SOURCE_DIR}/store/server.cc
    ${DIAMOND_SOURCE_DIR}/store/occstore.cc
    PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/store/)
include_directories(${CMAKE_CURRENT_BINARY_DIR})