# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

set(SRC_DIAMOND_STORE_COMMON_SERVER PARENT_SCOPE)

# Files common to the Diamond data store server
file(GLOB SRC_DIAMOND_STORE_COMMON_SERVER 
  "${DIAMOND_SOURCE_DIR}/store/common/backend/*.cc")

include_directories(${DIAMOND_SOURCE_DIR}/store/common/backend)


