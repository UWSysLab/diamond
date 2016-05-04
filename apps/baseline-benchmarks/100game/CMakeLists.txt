cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_100GAME_REDIS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -lhiredis -lev")

add_executable(game game-redis.cc benchmark_common.cc)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/../redox/include")

target_link_libraries(game ${Boost_LIBRARIES})
target_link_libraries(game "${PROJECT_SOURCE_DIR}/../redox/libredox.so")
