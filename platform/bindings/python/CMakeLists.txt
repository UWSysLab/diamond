# Python bindings

if (APPLE)
   find_path(pypath
	NAMES "pyconfig.h"
	PATHS
	"/opt/local/Library/Frameworks/Python.Framework/Headers"
	"/usr/local/Frameworks/Python.framework/Headers/"
	NO_DEFAULT_PATH)

   #message(${pypath})
   set(PYTHON_INCLUDE_DIR ${pypath})

   find_library(pylibpath
	NAMES "python2.7"
	PATHS
	"/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib"
	"/usr/local/Frameworks/Python.framework/Versions/Current/lib"
	NO_DEFAULT_PATH)
   #message(${pylibpath})
   set(PYTHON_LIBRARY ${pylibpath})
endif()

# finds the wrong version for Boost on Ubuntu
# find_package(PythonLibs)
set(PYTHON_INCLUDE_DIRS "/usr/include/python2.7")
set(PYTHON_LIBRARIES "//usr/lib/x86_64-linux-gnu/libpython2.7.so")

find_package(Boost COMPONENTS python REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

add_library (pydiamond MODULE
	    boost.cc)

target_link_libraries(pydiamond
	diamond
	${PYTHON_LIBRARIES}
	${Boost_LIBRARIES}
	${DIAMOND_LIB_DEPS}
	boost_python)
      
add_test(NAME pybindings
         COMMAND python ${CMAKE_CURRENT_DIR}/test/test.py)
