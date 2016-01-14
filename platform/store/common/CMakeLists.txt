# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Files common to the store, mostly data structures
file(GLOB SRC_DIAMOND_STORE_COMMON 
  "${DIAMOND_SOURCE_DIR}/store/common/*.cc")

add_subdirectory("${DIAMOND_SOURCE_DIR}/store/common/frontend/")
add_subdirectory("${DIAMOND_SOURCE_DIR}/store/common/backend/")

set(SRC_DIAMOND_STORE_COMMON_PROTO PARENT_SCOPE)
set(HDR_DIAMOND_STORE_COMMON_PROTO PARENT_SCOPE)
PROTOBUF_GENERATE_CPP(SRC_DIAMOND_STORE_COMMON_PROTO HDR_DIAMOND_STORE_COMMON_PROTO
  ${DIAMOND_SOURCE_DIR}/store/common/common-proto.proto)

# Don't add the protobuf targets for now
set(SRC_DIAMOND_STORE_COMMON_CLIENT
  ${SRC_DIAMOND_STORE_COMMON_CLIENT}
  ${SRC_DIAMOND_STORE_COMMON}
  PARENT_SCOPE)

set(SRC_DIAMOND_STORE_COMMON_SERVER
  ${SRC_DIAMOND_STORE_COMMON_SERVER}
  ${SRC_DIAMOND_STORE_COMMON}
  PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/store/common)
include_directories(${CMAKE_CURRENT_BINARY_DIR})