cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(DIAMOND_SRC_DIR "/Users/irene/uw/diamond/src/platform")
add_executable(demo demo.cc)

find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(${DIAMOND_SRC_DIR})
include_directories(${Boost_INCLUDE_DIRS})
if (APPLE)
   target_link_libraries(demo "${DIAMOND_SRC_DIR}/build/libdiamond.dylib")
else ()
   target_link_libraries(demo "${DIAMOND_SRC_DIR}/build/libdiamond.so")
endif (APPLE)
target_link_libraries(demo ${Boost_LIBRARIES})
