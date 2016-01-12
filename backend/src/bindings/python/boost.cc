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
#include <unordered_set>
#include <boost/python.hpp>

namespace diamond {
    
using namespace boost::python;

BOOST_PYTHON_MODULE(libpydiamond)
{
    void (*DiamondInitNoArgs)() = &DiamondInit;
    void (*DiamondInitWithArgs)(const std::string &) = &DiamondInit;
    def("DiamondInit", DiamondInitNoArgs);
    def("DiamondInit", DiamondInitWithArgs);

    void (*TransactionBegin)(void) = &DObject::TransactionBegin;
    class_<DObject, boost::noncopyable>("DObject", no_init)
        .def("TransactionBegin", TransactionBegin)
        .def("TransactionCommit", &DObject::TransactionCommit)
        .def("TransactionRetry", &DObject::TransactionRetry)
        .def("Map", &DObject::Map)
    ;

    class_<DString, bases<DObject> >("DString")
        .def("Map", &DString::Map)
        .staticmethod("Map")
        .def("Value", &DString::Value)
        .def("Set", &DString::Set)
    ;

    class_<DLong, bases<DObject> >("DLong")         
        .def("Map", &DLong::Map)
        .staticmethod("Map")
        .def("Value", &DLong::Value)
        .def("Set", &DLong::Set)
    ;
    class_<DCounter, bases<DObject> >("DCounter")
        .def("Map", &DCounter::Map)
        .staticmethod("Map")
        .def("Value", &DCounter::Value)
        .def("Set", &DCounter::Set)
    ;

    class_<DBoolean, bases<DObject> >("DBoolean")
        .def("Map", &DBoolean::Map)
        .staticmethod("Map")
        .def("Value", &DBoolean::Value)
        .def("Set", &DBoolean::Set)
    ;

    // Note from Niel: I wrote these bindings for the DSet class, since I needed
    // them for PyScrabble.

    // In order to write a wrapper for Members() (and the set version of
    // Add()), I needed a to-python converter for std::unordered_set. This
    // code defines a converter that packs the elements of the unordered_set
    // into a list. I wrote it by following this blog post:
    //
    //https://misspent.wordpress.com/2009/09/27/how-to-write-boost-python-converters/
    //
    // I made the converter return a tuple because there is no boost::python::set
    // object (apparently because Boost.Python was developed before sets were
    // added to Python).

    struct unordered_set_to_tuple {
        static PyObject * convert(const std::unordered_set<uint64_t> & orig_set) {
            boost::python::list result;
            for (std::unordered_set<uint64_t>::const_iterator it = orig_set.begin();
                    it != orig_set.end(); it++) {
                result.append(boost::python::object(*it));
            }
            return boost::python::incref(boost::python::tuple(result).ptr());
        }
    };

    boost::python::to_python_converter<std::unordered_set<uint64_t>,
            unordered_set_to_tuple>();

    // About the bindings for Add(): binding an overloaded method does not work
    // without some way of telling Boost.Python how to distinguish between
    // them. I followed the Stack Overflow answer here:
    //
    //http://stackoverflow.com/questions/7577410/boost-python-select-between-overloaded-methods
    //
    // I have not yet implemented bindings for the other version of Add(),
    // which takes an unordered_set as an argument. Using this approach, I
    // would have to give the method a different name in the Python class.
    // There are macros in Boost.Python that you can use to disambiguate
    // overloaded methods and still maintain one name in the Python class, but
    // I think they might require that the different overloaded versions of the
    // method have different numbers of parameters.
    //
    // Also, in order to implement the other version of Add(), I think I need
    // to write a from-python converter for unordered_set (the equivalent of
    // the to_python converter above).

    void (DSet::*Add)(const uint64_t) = &DSet::Add;
    class_<DSet, bases<DObject> >("DSet")
        .def("Map", &DSet::Map)
        .staticmethod("Map")
        .def("Members", &DSet::Members)
        .def("InSet", &DSet::InSet)
        .def("Add", Add)
        .def("Remove", &DSet::Remove)
        .def("Clear", &DSet::Clear)
    ;

    struct uint64_vector_to_list {
        static PyObject * convert(const std::vector<uint64_t> & orig_vec) {
            boost::python::list result;
            for (std::vector<uint64_t>::const_iterator it = orig_vec.begin();
                    it != orig_vec.end(); it++) {
                result.append(boost::python::object(*it));
            }
            return boost::python::incref(result.ptr());
        }
    };

    boost::python::to_python_converter<std::vector<uint64_t>, uint64_vector_to_list>();

    void (DList::*Append)(const uint64_t) = &DList::Append;
    class_<DList, bases<DObject> >("DList")
        .def("Map", &DList::Map)
        .staticmethod("Map")
        .def("Members", &DList::Members)
        .def("Value", &DList::Value)
        .def("Index", &DList::Index)
        .def("Append", Append)
        .def("Insert", &DList::Insert)
        .def("Erase", &DList::Erase)
        .def("Remove", &DList::Remove)
        .def("Clear", &DList::Clear)
        .def("Size", &DList::Size)
    ;

    struct string_vector_to_list {
        static PyObject * convert(const std::vector<std::string> & orig_vec) {
            boost::python::list result;
            for (std::vector<std::string>::const_iterator it = orig_vec.begin();
                    it != orig_vec.end(); it++) {
                result.append(boost::python::object(*it));
            }
            return boost::python::incref(result.ptr());
        }
    };

    boost::python::to_python_converter<std::vector<std::string>, string_vector_to_list>();

    void (DStringList::*StringAppend)(const std::string) = &DStringList::Append;
    class_<DStringList, bases<DObject> >("DStringList")
        .def("Map", &DStringList::Map)
        .staticmethod("Map")
        .def("Members", &DStringList::Members)
        .def("Value", &DStringList::Value)
        .def("Index", &DStringList::Index)
        .def("Append", StringAppend)
        .def("Insert", &DStringList::Insert)
        .def("Erase", &DStringList::Erase)
        .def("Remove", &DStringList::Remove)
        .def("Clear", &DStringList::Clear)
        .def("Size", &DStringList::Size)
    ;
}

    
    
} // namespace diamond
