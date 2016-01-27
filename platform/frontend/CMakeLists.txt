# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Diamond data types
file(GLOB SRC_DIAMOND_FRONT_END
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

set(SRC_DIAMOND_FRONT_END ${SRC_DIAMOND_FRONT_END} PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})