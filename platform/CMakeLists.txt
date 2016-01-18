# Diamond platform code

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

if(ANDROID)
  include_directories("${PROJECT_SOURCE_DIR}/external/libuv/include")
  link_directories("${PROJECT_SOURCE_DIR}/external/libuv/android")
endif()

# ---------------------------------------------------------
# Include and link directories
# ---------------------------------------------------------


set(DIAMOND_SOURCE_DIR ${PROJECT_SOURCE_DIR})
include_directories(${DIAMOND_SOURCE_DIR})

# Add protobufs
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(SRC_DIAMOND_PROTO HDR_DIAMOND_PROTO
  ${DIAMOND_SOURCE_DIR}/request.proto
  ${DIAMOND_SOURCE_DIR}/vr-proto.proto
  ${DIAMOND_SOURCE_DIR}/common-proto.proto
  ${DIAMOND_SOURCE_DIR}/store-proto.proto
    ${DIAMOND_SOURCE_DIR}/latency-format.proto)
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

# ---------------------------------------------------------
# Build targets
# ---------------------------------------------------------

# Set up library dependencies for linking into the Diamond library
add_library(diamond SHARED
            ${SRC_DIAMOND_LIB}
	    ${SRC_DIAMOND_DATA_TYPES}
	    ${SRC_DIAMOND_PROTO}
	    ${HDR_DIAMOND_PROTO}
	    ${SRC_DIAMOND_REPLICATION_CLIENT}
            ${SRC_DIAMOND_STORE_CLIENT})
target_link_libraries(diamond ${PROTOBUF_LIBRARIES})


# Set up the executable for the Diamond server
add_executable(server
  ${SRC_DIAMOND_LIB}
  ${SRC_DIAMOND_DATA_TYPES}
  ${SRC_DIAMOND_PROTO}
  ${HDR_DIAMOND_PROTO}
  ${SRC_REPLICATION_SERVER}
  ${SRC_DIAMOND_STORE_SERVER})
target_link_libraries(server ${PROTOBUF_LIBRARIES})


# only create and test for x86
if (NOT ANDROID)
  # Add python bindings for Linux and Mac OSX
  add_subdirectory(bindings/python)
endif()

# if(ANDROID)
#   target_link_libraries(diamond -luv)
# endif()

