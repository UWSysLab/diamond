cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(DIAMOND_SRC_DIR "${CMAKE_SOURCE_DIR}/../../../platform")
add_executable(game game.cc)

find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(${DIAMOND_SRC_DIR})
include_directories(${Boost_INCLUDE_DIRS})
if (APPLE)
   target_link_libraries(game "${DIAMOND_SRC_DIR}/build/libdiamond.dylib")
else ()
   target_link_libraries(game "${DIAMOND_SRC_DIR}/build/libdiamond.so")
endif (APPLE)
target_link_libraries(game ${Boost_LIBRARIES})
