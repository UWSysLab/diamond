# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Diamond general library code
file(GLOB SRC_DIAMOND_LIB "${DIAMOND_SOURCE_DIR}/lib/*.cc")

set(SRC_DIAMOND_LIB ${SRC_DIAMOND_LIB} PARENT_SCOPE)

include_directories(${DIAMOND_SOURCE_DIR}/lib/)