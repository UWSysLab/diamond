# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Diamond data types
file(GLOB SRC_DIAMOND_TIMESERVER
  "${DIAMOND_SOURCE_DIR}/timeserver/*.cc")

set(SRC_DIAMOND_TIMESERVER ${SRC_DIAMOND_TIMESERVER} PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/timeserver/)