# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

file(GLOB SRC_DIAMOND_REPLICATION_COMMON
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

set(SRC_DIAMOND_REPLICATION_COMMON ${SRC_DIAMOND_REPLICATION_COMMON} PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/replication/common)