# Diamond platform code

cmake_minimum_required (VERSION 2.8.7)
project (DIAMOND C CXX)

# ----------------------------------------------------
# Additional CMake modules 
# ----------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# ----------------------------------------------------
# C Flags
# ----------------------------------------------------

if (NOT ANDROID)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fstack-protector -Wall -Wno-uninitialized")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fstack-protector -std=c++0x -fPIC -Wall")
   set(CMAKE_LD_FLAGS "-levent_pthreads")
else()
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -fstack-protector -g -Wall -Wno-uninitialized")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DANDROID -fstack-protector -g -std=c++0x -fPIC -Wall")
endif()

if(ANDROID)
  include_directories("${PROJECT_SOURCE_DIR}/external/libuv/include")
  link_directories("${PROJECT_SOURCE_DIR}/external/libuv/android")
endif()

# ---------------------------------------------------------
# Include and link directories
# ---------------------------------------------------------


set(DIAMOND_SOURCE_DIR ${PROJECT_SOURCE_DIR})
include_directories(${DIAMOND_SOURCE_DIR})

# Add pthreads
find_package(Threads REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

# Add protobufs
if (NOT ANDROID)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(SRC_DIAMOND_REPLICATION_PROTO HDR_DIAMOND_REPLICATION_PROTO
  ${DIAMOND_SOURCE_DIR}/request.proto
  ${DIAMOND_SOURCE_DIR}/vr-proto.proto
  ${DIAMOND_SOURCE_DIR}/latency-format.proto)

PROTOBUF_GENERATE_CPP(SRC_DIAMOND_STORE_PROTO HDR_DIAMOND_STORE_PROTO
  ${DIAMOND_SOURCE_DIR}/common-proto.proto
  ${DIAMOND_SOURCE_DIR}/store-proto.proto
  ${DIAMOND_SOURCE_DIR}/diamond-proto.proto
  ${DIAMOND_SOURCE_DIR}/latency-format.proto
  ${DIAMOND_SOURCE_DIR}/notification-proto.proto)
endif()

include_directories(${PROJECT_BINARY_DIR})

if(APPLE)
    include_directories("/opt/local/include" "/usr/local/include")
    link_directories("/usr/local/lib" "/opt/local/lib")
    cmake_policy(SET CMP0042 NEW)
endif()

# ---------------------------------------------------------
# Source files
# ---------------------------------------------------------

add_subdirectory(data_types)
add_subdirectory(lib)
add_subdirectory(replication)
add_subdirectory(store)
add_subdirectory(frontend)
add_subdirectory(client)
add_subdirectory(transactions)

# ---------------------------------------------------------
# Build targets
# ---------------------------------------------------------

if (ANDROID)
    include_directories("${DIAMOND_SOURCE_DIR}/../external/libevent/include")
    include_directories("${DIAMOND_SOURCE_DIR}/../external/protobuf-2.5.0/src")
    include_directories("${DIAMOND_SOURCE_DIR}/build")
    link_directories("${DIAMOND_SOURCE_DIR}/../external/libevent/.libs")
    link_directories("${DIAMOND_SOURCE_DIR}/../external/protobuf-2.5.0/src/.libs")
endif()

# Set up library dependencies for linking into the Diamond library

if (ANDROID)
add_library(diamond SHARED
            ${SRC_DIAMOND_LIB}
	    ${SRC_DIAMOND_DATA_TYPES}
	    ${SRC_DIAMOND_TRANSACTIONS}
        ${SRC_DIAMOND_STORE_COMMON}
        ${SRC_DIAMOND_CLIENT}
        ${SRC_DIAMOND_FRONT_END_CLIENT}
        ${DIAMOND_SOURCE_DIR}/build/common-proto.pb.cc
        ${DIAMOND_SOURCE_DIR}/build/store-proto.pb.cc
        ${DIAMOND_SOURCE_DIR}/build/diamond-proto.pb.cc
        ${DIAMOND_SOURCE_DIR}/build/latency-format.pb.cc
        ${DIAMOND_SOURCE_DIR}/build/notification-proto.pb.cc)
  target_link_libraries(diamond -lprotobuf -levent_core -levent_pthreads -lgnustl_shared)
else()
add_library(diamond SHARED
            ${SRC_DIAMOND_LIB}
	    ${SRC_DIAMOND_DATA_TYPES}
	    ${SRC_DIAMOND_TRANSACTIONS}
	    ${SRC_DIAMOND_STORE_PROTO}
	    ${HDR_DIAMOND_STORE_PROTO}
        ${SRC_DIAMOND_STORE_COMMON}
        ${SRC_DIAMOND_CLIENT}
        ${SRC_DIAMOND_FRONT_END_CLIENT})
target_link_libraries(diamond ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} event_pthreads)

  find_package (OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})

  # Set up the executable for the Diamond storage server
  add_executable(storeserver
    ${SRC_DIAMOND_LIB}
    ${SRC_DIAMOND_REPLICATION_PROTO}
    ${HDR_DIAMOND_REPLICATION_PROTO}
    ${SRC_DIAMOND_STORE_PROTO}
    ${HDR_DIAMOND_STORE_PROTO}
    ${SRC_DIAMOND_REPLICATION_SERVER}
    ${SRC_DIAMOND_STORE_SERVER})
  target_link_libraries(storeserver ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} event_pthreads)

  add_executable(frontserver
    ${SRC_DIAMOND_LIB}
	${SRC_DIAMOND_STORE_PROTO}
	${HDR_DIAMOND_STORE_PROTO}
    ${SRC_DIAMOND_REPLICATION_PROTO}
    ${HDR_DIAMOND_REPLICATION_PROTO}
	${SRC_DIAMOND_REPLICATION_CLIENT}
    ${SRC_DIAMOND_STORE_CLIENT}
    ${SRC_DIAMOND_STORE_COMMON}
    ${SRC_DIAMOND_FRONT_END_SERVER})
  target_link_libraries(frontserver ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} event_pthreads)


  # compile the timestamp server
  add_subdirectory(timeserver)
  add_executable(tss
    ${SRC_DIAMOND_TIMESERVER}
    ${SRC_DIAMOND_LIB}
    ${SRC_DIAMOND_REPLICATION_PROTO}
    ${HDR_DIAMOND_REPLICATION_PROTO}
    ${SRC_DIAMOND_REPLICATION_SERVER})
  target_link_libraries(tss ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} event_pthreads)

  # Add python bindings for Linux and Mac OSX
  add_subdirectory(bindings/python)
 
  # Set up testing infrastructure
  enable_testing()

  # look for GTest
  find_package(GTest)
  if (NOT GTEST_FOUND)
    INCLUDE(ExternalProject)

    # Pull gtest from the web
    ExternalProject_Add(
        googletest
        PREFIX ${PROJECT_BINARY_DIR}/gtest
        URL http://googletest.googlecode.com/files/gtest-1.7.0.zip
	UPDATE_COMMAND ""
        INSTALL_COMMAND ""
#        CMAKE_ARGS BUILD_SHARED_LIBS=ON
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON)

      # Specify include dir
      ExternalProject_Get_Property(googletest source_dir)
      set(GTEST_INCLUDE_DIRS ${source_dir}/include)
      set(GTEST_ROOT ${source_dir})

      # Library
      ExternalProject_Get_Property(googletest binary_dir)
      set(GTEST_LIBRARY_PATH ${binary_dir})

      set(GTEST_LIBRARIES gtest)
      set(GTEST_MAIN_LIBRARIES gtest_main)
      set(GTEST_BOTH_LIBRARIES gtest gtest_main)
   
      add_library(${GTEST_LIBRARIES} UNKNOWN IMPORTED)
      add_library(${GTEST_MAIN_LIBRARIES} UNKNOWN IMPORTED)

      set_property(TARGET ${GTEST_LIBRARIES} PROPERTY IMPORTED_LOCATION
        ${GTEST_LIBRARY_PATH}/libgtest.a )
      set_property(TARGET ${GTEST_MAIN_LIBRARIES} PROPERTY IMPORTED_LOCATION
        ${GTEST_LIBRARY_PATH}/libgtest_main.a )

      add_dependencies(${GTEST_LIBRARIES} googletest)
      add_dependencies(${GTEST_MAIN_LIBRARIES} googletest)
    else()
      set(GTEST_ROOT "/usr/src/")
    endif()

    include_directories(${GTEST_INCLUDE_DIRS})
    # Add tests
    #add_subdirectory(test)
    # Build the terminal client
    add_subdirectory(terminalclient)    

    # Add Libevent
  find_package (LibEvent)
  if (LibEvent_FOUND)
    include_directories(${LIBEVENT_INCLUDE_DIR})
    target_link_libraries (storeserver ${LIBEVENT_LIB})
    target_link_libraries (frontserver ${LIBEVENT_LIB})
    target_link_libraries (diamond ${LIBEVENT_LIB})
    target_link_libraries (tss ${LIBEVENT_LIB})
  else()
    find_package (LibUV REQUIRED)
    include_directories(${LIBUV_INCLUDE_DIRS})
    target_link_libraries (diamond ${LIBUV_LIBRARIES})
    target_link_libraries (storeserver ${LIBUV_LIBRARIES})
    target_link_libraries (frontserver ${LIBEVENT_LIB})
    target_link_libraries (tss ${LIBUV_LIBRARIES})
  endif (LibEvent_FOUND)

endif()
