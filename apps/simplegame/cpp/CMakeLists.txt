cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(game game.cc)

find_package(Boost REQUIRED COMPONENTS program_options)

include_directories("${PROJECT_SOURCE_DIR}/../../../platform")
include_directories(${Boost_INCLUDE_DIRS})
if (APPLE)
   target_link_libraries(game "${PROJECT_SOURCE_DIR}/../../../platform/build/libdiamond.dylib")
else ()
   target_link_libraries(game "${PROJECT_SOURCE_DIR}/../../../platform/build/libdiamond.so")
endif (APPLE)
target_link_libraries(game ${Boost_LIBRARIES})
