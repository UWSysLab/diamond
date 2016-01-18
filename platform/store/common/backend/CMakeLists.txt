# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Files common to the Diamond data store server
file(GLOB SRC_DIAMOND_STORE_COMMON_SERVER 
  "${DIAMOND_SOURCE_DIR}/store/common/backend/*.cc")

set(SRC_DIAMOND_STORE_COMMON_SERVER ${SRC_DIAMOND_STORE_COMMON_SERVER} PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/store/common/backend)


