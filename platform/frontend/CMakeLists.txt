# ---------------------------------------------------------
# Directory Source files
# ---------------------------------------------------------

set(SRC_DIAMOND_FRONT_END_SERVER "${CMAKE_CURRENT_SOURCE_DIR}/server.cc" PARENT_SCOPE)
set(SRC_DIAMOND_FRONT_END_CLIENT "${CMAKE_CURRENT_SOURCE_DIR}/client.cc" PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
