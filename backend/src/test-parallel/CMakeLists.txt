
# Test1
add_executable(test1
    test1.cc)
target_link_libraries(test1 diamond ${DIAMOND_LIB_DEPS})


# Test2
add_executable(test2
    test2.cc)
target_link_libraries(test2 diamond ${DIAMOND_LIB_DEPS})


