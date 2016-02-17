// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/interval.h:
 *   Our interval definition
 *
 **********************************************************************/

#ifndef _INTERVAL_H
#define _INTERVAL_H_

#include <algorithm>
#include "timestamp.h"

class Interval {
    Timestamp start;
    Timestamp end;

public:
    Interval() : start(INVALID_TIMESTAMP), end(MAX_TIMESTAMP) { };
    Interval(Timestamp s) : start(s), end(MAX_TIMESTAMP) { };
    Interval(Timestamp s, Timestamp e) : start(s), end(e) { };

    void SetStart(const Timestamp s) { start = s; };
    void SetEnd(const Timestamp e) {end = e; };
    Timestamp Start() const { return start; };
    Timestamp End() const { return end; };
    friend Interval Intersect(const Interval &i1, const Interval &i2) {
        return Interval(std::max(i1.start, i2.start), std::min(i1.end, i2.end));
    };
};

#endif /* _INTERVAL_H_ */
