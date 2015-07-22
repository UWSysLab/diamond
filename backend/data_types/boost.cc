// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/boost.cc
 *   Diamond boost python bindings
 *
 **********************************************************************/

#include "includes/data_types.h"
#include <string>
#include <boost/python.hpp>

namespace diamond {
    
using namespace boost::python;

BOOST_PYTHON_MODULE(libdiamond)
{
    class_<DString>("DString")
        .def("Map", &DString::Map)
        .staticmethod("Map")
        .def("Value", &DString::Value)
        .def("Set", &DString::Set)
    ;

    class_<DLong>("DLong")         
        .def("Map", &DLong::Map)
        .staticmethod("Map")
        .def("Value", &DLong::Value)
        .def("Set", &DLong::Set)
    ;
    class_<DCounter>("DCounter")
        .def("Map", &DCounter::Map)
        .staticmethod("Map")
        .def("Value", &DCounter::Value)
        .def("Set", &DCounter::Set)
    ;
}

    
    
} // namespace diamond
