cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


include_directories("${PROJECT_SOURCE_DIR}/../../platform")
include_directories("${PROJECT_SOURCE_DIR}/../../platform/build")

add_executable(benchClient benchClient.cc)
target_link_libraries(benchClient "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")

add_executable(retwisClient retwisClient.cc)
target_link_libraries(retwisClient "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
