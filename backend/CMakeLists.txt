# Diamond back-end code

cmake_minimum_required (VERSION 2.8.11)
project (DIAMOND)

# ----------------------------------------------------
# C Flags
# ----------------------------------------------------

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-uninitialized")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC -Wall")

# ---------------------------------------------------------
# Download dependencies
# ---------------------------------------------------------

set(PROJECT_LIBRARIES_DIR "${PROJECT_SOURCE_DIR}/libraries")

# Download the hiredis client
# set up the hiredis redis client that redox needs
INCLUDE(ExternalProject)

ExternalProject_Add(
    hiredis-redisclient
    GIT_REPOSITORY git@github.com:redis/hiredis.git
    PREFIX ${PROJECT_BINARY_DIR}/hiredis
    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/hiredis/src
    SOURCE_DIR ${PROJECT_BINARY_DIR}/hiredis/src
    BINARY_DIR ${PROJECT_BINARY_DIR}/hiredis/src
    STAMP_DIR ${PROJECT_BINARY_DIR}/hiredis/stamp
    TMP_DIR ${PROJECT_BINARY_DIR}/hiredis/tmp
    BUILD_COMMAND "\"${PROJECT_SOURCE_DIR}/setup_env.sh\" \" ${CMAKE_C_COMPILER}\" \"\\\"${CMAKE_C_FLAGS}\\\"\""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

    include_directories(${PROJECT_BINARY_DIR}/hiredis/src)
    link_directories(${PROJECT_BINARY_DIR}/hiredis/src) 
  
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

# Diamond general library code
set(SRC_DIAMOND_LIB
    ${DIAMOND_SOURCE_DIR}/lib/lookup3.cc
    ${DIAMOND_SOURCE_DIR}/lib/message.cc)

# Diamond data store client
set(SRC_DIAMOND_CLIENT ${DIAMOND_SOURCE_DIR}/client/client.cc)

# Diamond data structure implementations
set(SRC_DIAMOND_DATA_TYPES
    ${DIAMOND_SOURCE_DIR}/data_types/string.cc
    ${DIAMOND_SOURCE_DIR}/data_types/counter.cc
    ${DIAMOND_SOURCE_DIR}/data_types/long.cc
    ${DIAMOND_SOURCE_DIR}/data_types/set.cc
    ${DIAMOND_SOURCE_DIR}/data_types/list.cc
    ${DIAMOND_SOURCE_DIR}/data_types/stringlist.cc)

# ---------------------------------------------------------
# Build targets
# ---------------------------------------------------------

# Set up library dependencies for linking into the Diamond library

if (ANDROID)
   set(DIAMOND_LIB_DEPS hiredis)
else()
   set(DIAMOND_LIB_DEPS hiredis pthread)
endif()

add_library (diamond SHARED
            ${SRC_DIAMOND_LIB}
            ${SRC_DIAMOND_CLIENT}
            ${SRC_DIAMOND_DATA_TYPES})
target_link_libraries(diamond ${DIAMOND_LIB_DEPS})
add_dependencies(diamond hiredis-redisclient)

# only create and test for x86
if (NOT ANDROID)
  # Add python bindings for Linux and Mac OSX
  add_subdirectory(${DIAMOND_SOURCE_DIR}/bindings/python)

# Adding Diamond tests
add_subdirectory(${DIAMOND_SOURCE_DIR}/test)
endif()
