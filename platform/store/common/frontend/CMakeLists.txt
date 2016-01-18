# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Files common to the Diamond data store client
file(GLOB SRC_DIAMOND_STORE_COMMON_CLIENT
  "${DIAMOND_SOURCE_DIR}/store/common/frontend/*.cc")

set(SRC_DIAMOND_STORE_COMMON_CLIENT ${SRC_DIAMOND_STORE_COMMON_CLIENT} PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/store/common/frontend)