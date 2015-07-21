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

BOOST_PYTHON_MODULE(diamond)
{
    class_<DString>("DString", init<const std::string &, const std::string &>())
        .def("Value", &DString::Value)
        .def("Set", &DString::Value)
    ;
}

    
} // namespace diamond
