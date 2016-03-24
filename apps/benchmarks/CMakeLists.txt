cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(benchmarkclient benchmarkclient.cc)
add_executable(settest settest.cc)
add_executable(scalability scalability.cc)

find_package(Boost REQUIRED COMPONENTS program_options)

include_directories("${PROJECT_SOURCE_DIR}/../../platform")
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(benchmarkclient "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(benchmarkclient ${Boost_LIBRARIES})

target_link_libraries(settest "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(settest ${Boost_LIBRARIES})

target_link_libraries(scalability "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(scalability ${Boost_LIBRARIES})
