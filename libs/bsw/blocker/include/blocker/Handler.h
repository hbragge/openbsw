// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace blocker
{
class MsgPasser;
class Message;

class Handler
{
public:
    explicit Handler(MsgPasser& l);
    MsgPasser& get_blocker() const;
    virtual void handle_msg(Message const&) = 0;

private:
    MsgPasser& _blocker;
};

inline Handler::Handler(MsgPasser& b) : _blocker(b) {}

inline MsgPasser& Handler::get_blocker() const { return _blocker; }

} // namespace blocker
