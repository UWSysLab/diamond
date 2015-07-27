# Set up testing infrastructure
enable_testing()
set(GTEST_ROOT /usr/src/gtest)
find_package(GTest)

if (NOT GTEST_FOUND)
   INCLUDE(ExternalProject)

   # Set default ExternalProject root directory
   ExternalProject_Add(
        googletest
        PREFIX ${PROJECT_SOURCE_DIR}/gtest
        URL http://googletest.googlecode.com/files/gtest-1.7.0.zip
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON)

   # Specify include dir
   ExternalProject_Get_Property(googletest source_dir)
   set(GTEST_INCLUDE_DIR ${source_dir}/include)

   # Library
   ExternalProject_Get_Property(googletest binary_dir)
   set(GTEST_LIBRARY_PATH
       ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
   set(GTEST_LIBRARY gtest)
   add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
   set_property(TARGET ${GTEST_LIBRARY} PROPERTY IMPORTED_LOCATION
                ${GTEST_LIBRARY_PATH} )
   add_dependencies(${GTEST_LIBRARY} googletest)
endif()

# add gtest include directory
include_directories(${GTEST_INCLUDE_DIRS})

# Diamond tests
set(SRC_DIAMOND_GTEST
    ${PROJECT_SOURCE_DIR}/test/backend-test.cc)
add_executable(diamondtest
    ${SRC_DIAMOND_GTEST}
    ${SRC_DIAMOND_LIB}
    ${SRC_DIAMOND_CLIENT}
    ${SRC_DIAMOND_DATA_TYPES})
target_link_libraries(diamondtest ${DIAMOND_LIB_DEPS} ${GTEST_BOTH_LIBRARIES})
add_test(DiamondBackendTests diamondtest)
