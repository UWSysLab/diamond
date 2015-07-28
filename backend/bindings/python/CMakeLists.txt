# Python bindings

find_package(PythonLibs 2.7)
find_package(Boost)
find_package(Boost COMPONENTS python)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

add_library (pydiamond SHARED
	    boost.cc)

target_link_libraries(pydiamond
	diamond
	${Boost_LIBRARIES}
	${DIAMOND_LIB_DEPS})
	    
add_test(NAME pybindings
         COMMAND python ${CMAKE_CURRENT_DIR}/test/test.py)
