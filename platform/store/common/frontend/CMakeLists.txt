# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

set(SRC_DIAMOND_STORE_COMMON_CLIENT PARENT_SCOPE)
# Files common to the Diamond data store client
file(GLOB SRC_DIAMOND_STORE_COMMON_CLIENT
  "${DIAMOND_SOURCE_DIR}/store/common/frontend/*.cc")

include_directories(${DIAMOND_SOURCE_DIR}/store/common/frontend)