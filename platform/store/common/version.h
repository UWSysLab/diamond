// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/version.h:
 *   Our version definition
 *
 **********************************************************************/

#ifndef _VERSION_H_
#define _VERSION_H_

#include <string>

class Version {
    Timestamp write;
    std::string value;
public:
    Version() : write(0), value("tmp") { };
    Version(Timestamp commit) : write(commit), value("tmp") { };
    Version(std::string val) : write(0), value(val) { };
    Version(Timestamp commit, std::string val) : write(commit), value(val) { };

    std::string GetValue() const { return value; };
    Timestamp GetTimestamp() const { return write; }; 
    friend bool operator> (const Version &v1, const Version &v2) {
        return v1.write > v2.write;
    };
    friend bool operator< (const Version &v1, const Version &v2) {
        return v1.write < v2.write;
    };
};

#endif /* _VERSION_H_ */
