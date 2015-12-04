# Diamond back-end code

cmake_minimum_required (VERSION 2.8.11)
project (DIAMOND)


# ----------------------------------------------------
# Additional CMake modules 
# ----------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# ----------------------------------------------------
# C Flags
# ----------------------------------------------------

if (NOT ANDROID)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fstack-protector -Wall -Wno-uninitialized")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fstack-protector -std=c++0x -fPIC -Wall -pthread")
else()
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -fstack-protector -g -Wall -Wno-uninitialized")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DANDROID -fstack-protector -g -std=c++0x -fPIC -Wall")
endif()

# ---------------------------------------------------------
# Download dependencies
# ---------------------------------------------------------

# Download the hiredis client
# set up the hiredis redis client that redox needs
INCLUDE(ExternalProject)

set(STDLIBNAME "libhiredis.a")
if(APPLE)
  set(DYLIBNAME "libhiredis.dylib")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk -mmacosx-version-min=10.10")
else()
  set(DYLIBNAME "libhiredis.so")
endif()

set(HIREDIS_MAKE make "CC=${CMAKE_C_COMPILER}" "CFLAGS=${CMAKE_C_FLAGS}")
    
if(ANDROID)
    set(HIREDIS_MAKE make "CC=${CMAKE_C_COMPILER}"
    "CFLAGS=${CMAKE_C_FLAGS}" "DYLIB_MAKE_CMD=${CMAKE_C_COMPILER} -shared ${ANDROID_NDK_GLOBAL_LDFLAGS} -o ${DYLIBNAME}"
    "STLIB_MAKE_CMD=${CMAKE_AR} rcs ${STDLIBNAME}"
    "LDFLAGS=${ANDROID_NDK_GLOBAL_LDFLAGS}")
endif()

# Use downloaded hiredis src
ExternalProject_Add(
  hiredis-redisclient
    URL https://github.com/redis/hiredis/archive/v0.13.1.tar.gz
    PREFIX ${PROJECT_BINARY_DIR}/hiredis
    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/hiredis/src
    SOURCE_DIR ${PROJECT_BINARY_DIR}/hiredis/src
    BINARY_DIR ${PROJECT_BINARY_DIR}/hiredis/src
    STAMP_DIR ${PROJECT_BINARY_DIR}/hiredis/stamp
    TMP_DIR ${PROJECT_BINARY_DIR}/hiredis/tmp
    BUILD_COMMAND ${HIREDIS_MAKE}
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)
include_directories(${PROJECT_BINARY_DIR}/hiredis/src)
link_directories(${PROJECT_BINARY_DIR}/hiredis/src) 

# Use local hiredis source
#ExternalProject_Add(
#  hiredis-redisclient
#    PREFIX ${PROJECT_SOURCE_DIR}/external/hiredis
#    SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/hiredis
#    BINARY_DIR ${PROJECT_SOURCE_DIR}/external/hiredis
#    BUILD_COMMAND ${HIREDIS_MAKE}
#    DOWNLOAD_COMMAND ""
#    CONFIGURE_COMMAND ""
#    INSTALL_COMMAND ""
#    UPDATE_COMMAND ""
#    LOG_BUILD ON)
#include_directories(${PROJECT_SOURCE_DIR}/external/hiredis)
#link_directories(${PROJECT_SOURCE_DIR}/external/hiredis) 

if(ANDROID)
  include_directories("${PROJECT_SOURCE_DIR}/external/libuv/include")
  link_directories("${PROJECT_SOURCE_DIR}/external/libuv/android")
endif()

# ---------------------------------------------------------
# Include and link directories
# ---------------------------------------------------------

# Diamond include directory
set(DIAMOND_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

include_directories(${DIAMOND_SOURCE_DIR})

if(APPLE)
    include_directories("/opt/local/include" "/usr/local/include")
    link_directories("/usr/local/lib" "/opt/local/lib")
    cmake_policy(SET CMP0042 NEW)
endif()

# ---------------------------------------------------------
# Source files
# ---------------------------------------------------------

# UNQLITE code
set(SRC_UNQLITE ${DIAMOND_SOURCE_DIR}/lib/unqlite.c)

# Diamond general library code
set(SRC_DIAMOND_LIB
    ${DIAMOND_SOURCE_DIR}/lib/lookup3.cc
    ${DIAMOND_SOURCE_DIR}/lib/message.cc)

# Diamond data store client
set(SRC_DIAMOND_CLOUD_STORE 
    ${DIAMOND_SOURCE_DIR}/storage/cloud.cc
    ${DIAMOND_SOURCE_DIR}/storage/stalestorage.cc)

# Diamond data structure implementations
set(SRC_DIAMOND_DATA_TYPES
    ${DIAMOND_SOURCE_DIR}/data_types/object.cc
    ${DIAMOND_SOURCE_DIR}/data_types/string.cc
    ${DIAMOND_SOURCE_DIR}/data_types/counter.cc
    ${DIAMOND_SOURCE_DIR}/data_types/long.cc
    ${DIAMOND_SOURCE_DIR}/data_types/set.cc
    ${DIAMOND_SOURCE_DIR}/data_types/list.cc
    ${DIAMOND_SOURCE_DIR}/data_types/stringlist.cc
    ${DIAMOND_SOURCE_DIR}/data_types/redisstringlist.cc
    ${DIAMOND_SOURCE_DIR}/data_types/stringset.cc)

# ---------------------------------------------------------
# Build targets
# ---------------------------------------------------------

add_library (diamondnotifycpp SHARED
            ${DIAMOND_SOURCE_DIR}/bindings/cpp/diamondnotifycpp.cc)

# Set up library dependencies for linking into the Diamond library

set(DIAMOND_LIB_DEPS hiredis)

add_library (diamond SHARED
            ${SRC_DIAMOND_LIB}
            ${SRC_DIAMOND_CLOUD_STORE}
            ${SRC_DIAMOND_DATA_TYPES}
            ${SRC_UNQLITE})
target_link_libraries(diamond ${DIAMOND_LIB_DEPS} diamondnotifycpp)
add_dependencies(diamond hiredis-redisclient)

# only create and test for x86
if (NOT ANDROID)
  # Add python bindings for Linux and Mac OSX
  add_subdirectory(${DIAMOND_SOURCE_DIR}/bindings/python)

  # Adding Diamond tests
  add_subdirectory(${DIAMOND_SOURCE_DIR}/test)

  # Adding Diamond parallel tests
  add_subdirectory(${DIAMOND_SOURCE_DIR}/test-parallel)

  # Add the Libevent
  find_package (LibEvent)
  if (LibEvent_FOUND)
    include_directories(${LIBEVENT_INCLUDE_DIR})
    target_link_libraries (diamond ${LIBEVENT_LIB})
  endif (LibEvent_FOUND)

  # Add the Libevent
  find_package (LibUV)
  if (LIBUV_FOUND)
    include_directories(${LIBUV_INCLUDE_DIRS})
    target_link_libraries (diamond ${LIBUV_LIBRARIES})
  endif (LIBUV_FOUND)


endif()

if(ANDROID)
  target_link_libraries(diamond -luv)
endif()
