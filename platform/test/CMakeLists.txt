# Diamond tests
set(SRC_DIAMOND_GTEST
    backend-test.cc)

add_executable(diamondtest
    ${SRC_DIAMOND_GTEST})
    
target_link_libraries(diamondtest diamond ${DIAMOND_LIB_DEPS} ${GTEST_BOTH_LIBRARIES})
add_test(DiamondPlatformTests diamondtest)

add_executable(testlist
    testlist.cc)
target_link_libraries(testlist diamond ${DIAMOND_LIB_DEPS})

# add_executable(testlongallocation
#     testlongallocation.cc)
# target_link_libraries(testlongallocation diamond ${DIAMOND_LIB_DEPS})

add_executable(testallocation
    testallocation.cc)
target_link_libraries(testallocation diamond ${DIAMOND_LIB_DEPS})
