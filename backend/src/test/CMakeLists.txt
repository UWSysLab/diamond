# Set up testing infrastructure
enable_testing()

if ((NOT GTEST_FOUND) OR APPLE)
   INCLUDE(ExternalProject)

   # Set default ExternalProject root directory
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
   set(GTEST_ROOT /usr/src/gtest)
   find_package(GTest)
endif()

# add gtest include directory
include_directories(${GTEST_INCLUDE_DIRS})

# Diamond tests
set(SRC_DIAMOND_GTEST
    backend-test.cc)

add_executable(diamondtest
    ${SRC_DIAMOND_GTEST})
    
target_link_libraries(diamondtest diamond ${DIAMOND_LIB_DEPS} ${GTEST_BOTH_LIBRARIES})
add_test(DiamondBackendTests diamondtest)
