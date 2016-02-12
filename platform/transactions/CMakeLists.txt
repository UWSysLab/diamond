# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

# Diamond transaction code
file(GLOB SRC_DIAMOND_TRANSACTIONS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

set(SRC_DIAMOND_TRANSACTIONS ${SRC_DIAMOND_TRANSACTIONS} PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
