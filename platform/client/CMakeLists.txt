# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Diamond data types
file(GLOB SRC_DIAMOND_CLIENT
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

set(SRC_DIAMOND_CLIENT ${SRC_DIAMOND_CLIENT} PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})