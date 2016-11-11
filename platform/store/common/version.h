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

#include "interval.h"
#include <string>
#include "common-proto.pb.h"

#define WRITE 0
#define INCREMENT 1
#define APPEND 2

class Version {
    Interval valid;
    // Operation that created this version
    std::string value;
    int op = WRITE;
public:
    Version() : valid(), value("") { };
    Version(const Timestamp commit) : valid(commit), value("") { };
    Version(const std::string &val) : valid(), value(val) { };
    Version(const Timestamp commit,
	    const std::string &val) : valid(commit), value(val) { };
    Version(const Timestamp commit,
	    const std::string &val,
	    const int op) : valid(commit), value(val), op(op) { };
    Version(const ReadReply &msg);
    
    std::string GetValue() const { return value; };
    Timestamp GetTimestamp() const { return valid.Start(); };
    Interval GetInterval() const { return valid; };
    void SetEnd(const Timestamp &commit) { valid.SetEnd(commit); };
    void SetValue(const std::string &val) { value = val; };
    
    friend bool operator> (const Version &v1, const Version &v2) {
        return v1.valid.Start() > v2.valid.Start();
    };
    friend bool operator< (const Version &v1, const Version &v2) {
        return v1.valid.Start() < v2.valid.Start();
    };
    void Serialize(ReadReply *msg) const;
    void Deserialize(ReadReply *msg);
};

#endif /* _VERSION_H_ */
