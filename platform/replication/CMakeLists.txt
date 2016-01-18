# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

add_subdirectory("${DIAMOND_SOURCE_DIR}/replication/common")

set(SRC_DIAMOND_REPLICATION_CLIENT 
  ${SRC_DIAMOND_REPLICATION_COMMON}
  ${DIAMOND_SOURCE_DIR}/replication/client.cc
  PARENT_SCOPE)
set(SRC_DIAMOND_REPLICATION_SERVER
  ${SRC_DIAMOND_REPLICATION_COMMON}
  ${DIAMOND_SOURCE_DIR}/replication/replica.cc
  PARENT_SCOPE)

include_directories(${DIAMOND_SRC_DIR}/replication/)
