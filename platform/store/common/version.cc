// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/version.cc:
 *   Our version definition
 *
 **********************************************************************/

#ifndef _VERSION_H_
#include "version.h"

Version::Version(const ReadReply &msg) {
    value = msg.value();
    valid = Interval(msg.timestamp(), msg.end());
    op = msg.op();
}

void
Version::Serialize(ReadReply *msg) const {
    msg->set_value(value);
    msg->set_timestamp(valid.Start());
    msg->set_end(valid.End());
    msg->set_op(op);
}
void
Version::Deserialize(ReadReply *msg) {
    value = msg->value();
    valid = Interval(msg->timestamp(), msg->end());
    op = msg->op();
}

#endif /* _VERSION_H_ */
