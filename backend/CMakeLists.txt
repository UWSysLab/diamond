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

# Set up Redox Redis client for accessing backend storage
INCLUDE(ExternalProject)

if (APPLE)
ExternalProject_Add(
    redox-redisclient    
    PREFIX ${PROJECT_SOURCE_DIR}/redox
    GIT_REPOSITORY git@github.com:hmartiro/redox.git
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    CMAKE_CACHE_ARGS -DCMAKE_INCLUDE_PATH:string="/opt/local/include:/usr/local/include"
    BINARY_DIR ${PROJECT_SOURCE_DIR}/redox/build
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)
else()
# Set default ExternalProject root directory
ExternalProject_Add(
    redox-redisclient
    PREFIX ${PROJECT_SOURCE_DIR}/redox
    GIT_REPOSITORY git@github.com:hmartiro/redox.git
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    BINARY_DIR ${PROJECT_SOURCE_DIR}/redox/build
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)
endif()
# ---------------------------------------------------------
# Include and link directories
# ---------------------------------------------------------

# Diamond include directory
include_directories(${PROJECT_SOURCE_DIR})

# Redox Redis client includes
ExternalProject_Get_Property(redox-redisclient source_dir)
include_directories(${source_dir}/includes)
link_directories(${source_dir}/build)

if(APPLE)
    include_directories("/usr/local/include" "/usr/local/Frameworks")
    link_directories("/usr/local/lib")
    cmake_policy(SET CMP0042 NEW)
endif()

# ---------------------------------------------------------
# Source files
# ---------------------------------------------------------

# Diamond general library code
set(SRC_DIAMOND_LIB
    ${PROJECT_SOURCE_DIR}/lib/lookup3.cc
    ${PROJECT_SOURCE_DIR}/lib/message.cc)

# Diamond data store client
set(SRC_DIAMOND_CLIENT ${PROJECT_SOURCE_DIR}/client/client.cc)

# Diamond data structure implementations
set(SRC_DIAMOND_DATA_TYPES
    ${PROJECT_SOURCE_DIR}/data_types/string.cc
    ${PROJECT_SOURCE_DIR}/data_types/counter.cc
    ${PROJECT_SOURCE_DIR}/data_types/long.cc
    ${PROJECT_SOURCE_DIR}/data_types/set.cc)

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

# Adding Diamond tests
add_subdirectory(test)
