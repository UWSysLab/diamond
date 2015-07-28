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

# Set up Redox Redis client for accessing backend storage
INCLUDE(ExternalProject)

# set up redox client
ExternalProject_Add(
    redox-redisclient
    PREFIX ${PROJECT_LIBRARIES_DIR}/redox
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    SOURCE_DIR ${PROJECT_LIBRARIES_DIR}/redox/
    BINARY_DIR ${PROJECT_LIBRARIES_DIR}/redox/build
    LOG_BUILD ON)
  
# ---------------------------------------------------------
# Include and link directories
# ---------------------------------------------------------

# Diamond include directory
set(DIAMOND_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

include_directories(${DIAMOND_SOURCE_DIR})

# Redox Redis client includes
include_directories(${PROJECT_LIBRARIES_DIR}/redox/include)
link_directories(${PROJECT_LIBRARIES_DIR}/redox)

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
    ${DIAMOND_SOURCE_DIR}/data_types/set.cc)

# ---------------------------------------------------------
# Build targets
# ---------------------------------------------------------

# Set up library dependencies for linking into the Diamond library

set(DIAMOND_LIB_DEPS ev pthread hiredis redox)
add_library (diamond SHARED
            ${SRC_DIAMOND_LIB}
            ${SRC_DIAMOND_CLIENT}
            ${SRC_DIAMOND_DATA_TYPES})
target_link_libraries(diamond ${DIAMOND_LIB_DEPS})
add_dependencies(diamond redox-redisclient)

# Add python bindings for Linux and Mac OSX
if (NOT CMAKE_SYSTEM_NAME MATCHES "Android")
  add_subdirectory(${DIAMOND_SOURCE_DIR}/bindings/python)
endif()

# Adding Diamond tests
add_subdirectory(${DIAMOND_SOURCE_DIR}/test)
