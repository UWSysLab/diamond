# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

set(SRC_DIAMOND_DATA_TYPES PARENT_SCOPE)

# Diamond data types
file(GLOB SRC_DIAMOND_DATA_TYPES
  "${DIAMOND_SOURCE_DIR}/data_types/*.cc")
