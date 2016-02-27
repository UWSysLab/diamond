cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(client client.cc)
add_executable(publisher publisher.cc)

find_package(Boost REQUIRED COMPONENTS program_options)

include_directories("${PROJECT_SOURCE_DIR}/../../../platform")
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(client "${PROJECT_SOURCE_DIR}/../../../platform/build/libdiamond.so")
target_link_libraries(client ${Boost_LIBRARIES})
target_link_libraries(publisher "${PROJECT_SOURCE_DIR}/../../../platform/build/libdiamond.so")
target_link_libraries(publisher ${Boost_LIBRARIES})
