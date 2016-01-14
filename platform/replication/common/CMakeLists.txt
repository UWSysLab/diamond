# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

set(SRC_DIAMOND_REPLICATION_COMMON_PROTO PARENT_SCOPE)
set(HDR_DIAMOND_REPLICATION_COMMON_PROTO PARENT_SCOPE)
set(SRC_DIAMOND_REPLICATION_COMMON PARENT_SCOPE)

file(GLOB SRC_DIAMOND_REPLICATION_COMMON
  "${DIAMOND_SOURCE_DIR}/replication/common/*.cc")

PROTOBUF_GENERATE_CPP(SRC_DIAMOND_REPLICATION_COMMON_PROTO HDR_DIAMOND_REPLICATION_COMMON_PROTO
  ${DIAMOND_SOURCE_DIR}/replication/common/request.proto)


include_directories(${DIAMOND_SOURCE_DIR}/replication/common)
include_directories(${CMAKE_CURRENT_BINARY_DIR})