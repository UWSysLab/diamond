cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND_BENCHMARKS C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(benchmarkclient benchmarkclient.cc)
add_executable(settest settest.cc)
add_executable(scalability scalability.cc benchmark_common.cc)
add_executable(caching caching.cc benchmark_common.cc)
add_executable(docc docc.cc benchmark_common.cc)
add_executable(notification_publisher notification_publisher.cc benchmark_common.cc)
add_executable(notification_subscriber notification_subscriber.cc benchmark_common.cc)
add_executable(game game.cc benchmark_common.cc)
add_executable(debug debug.cc benchmark_common.cc)
add_executable(game-redis game-redis.cc benchmark_common.cc)

find_package(Boost REQUIRED COMPONENTS program_options)


include_directories("${PROJECT_SOURCE_DIR}/../../platform")
include_directories(${Boost_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/redox")

target_link_libraries(benchmarkclient "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(benchmarkclient ${Boost_LIBRARIES})

target_link_libraries(settest "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(settest ${Boost_LIBRARIES})

target_link_libraries(scalability "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(scalability ${Boost_LIBRARIES})

target_link_libraries(caching "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(caching ${Boost_LIBRARIES})

target_link_libraries(docc "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(docc ${Boost_LIBRARIES})

target_link_libraries(notification_publisher "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(notification_publisher ${Boost_LIBRARIES})

target_link_libraries(notification_subscriber "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(notification_subscriber ${Boost_LIBRARIES})

target_link_libraries(game "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(game ${Boost_LIBRARIES})

target_link_libraries(game "${PROJECT_SOURCE_DIR}/redox/libredox.so")
target_link_libraries(debug "${PROJECT_SOURCE_DIR}/../../platform/build/libdiamond.so")
target_link_libraries(debug ${Boost_LIBRARIES})
