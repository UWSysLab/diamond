# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Diamond data types
file(GLOB SRC_DIAMOND_DATA_TYPES
  "${DIAMOND_SOURCE_DIR}/data_types/*.cc")

set(SRC_DIAMOND_DATA_TYPES ${SRC_DIAMOND_DATA_TYPES} PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/data_types/)