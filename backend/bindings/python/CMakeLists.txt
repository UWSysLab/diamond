# Python bindings

add_library (pydiamond SHARED boost.cc ${SRC_DIAMOND_LIB} ${SRC_DIAMOND_CLIENT} ${SRC_DIAMOND_DATA_TYPES}))

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_DIRS})

target_link_libraries(pydiamond python2.7 boost_python ${DIAMOND_LIB_DEPS}))

add_test(NAME pybindings
         COMMAND python ${CMAKE_CURRENT_DIR}/test/test.py)
