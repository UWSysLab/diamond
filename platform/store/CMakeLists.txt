# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

add_subdirectory(${DIAMOND_SOURCE_DIR}/store/common)

set(SRC_DIAMOND_STORE_CLIENT
    ${SRC_DIAMOND_STORE_COMMON_CLIENT}
    ${DIAMOND_SOURCE_DIR}/store/client.cc
    ${DIAMOND_SOURCE_DIR}/store/shardclient.cc
    PARENT_SCOPE)

set(SRC_DIAMOND_STORE_SERVER
    ${SRC_DIAMOND_STORE_COMMON_SERVER}
    ${DIAMOND_SOURCE_DIR}/store/server.cc
    ${DIAMOND_SOURCE_DIR}/store/occstore.cc
    PARENT_SCOPE)

set(SRC_DIAMOND_STORE_COMMON
  ${SRC_DIAMOND_STORE_COMMON_SERVER}
    ${SRC_DIAMOND_STORE_COMMON_CLIENT}
  PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/store/)
