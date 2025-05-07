// Copyright 2025 Accenture.

#pragma once

#include "blocker/Handler.h"
#include "blocker/Message.h"

#include <platform/estdint.h>

namespace blocker
{
class Queue
{
public:
    bool send(uint32_t now, Message& msg);
    bool get_next_timeout(uint32_t now, uint32_t& timeout) const;
    Message* try_dequeue(uint32_t now);
    void clear();

private:
    using message_list = ::estd::forward_list<Message>;

    message_list _messages;
};

inline bool Queue::send(uint32_t const now, Message& msg)
{
    int32_t const diff            = static_cast<int32_t>(msg.timestamp() - now);
    message_list::iterator prevIt = _messages.before_begin();
    for (message_list::iterator it = _messages.begin(); it != _messages.end(); ++it)
    {
        int32_t const nextDiff = static_cast<int32_t>(it->timestamp() - now);
        if (nextDiff > diff)
        {
            break;
        }
        prevIt = it;
    }
    (void)_messages.insert_after(prevIt, msg);
    return prevIt == _messages.before_begin();
}

inline bool Queue::get_next_timeout(uint32_t const now, uint32_t& timeout) const
{
    if (!_messages.empty())
    {
        Message const& msg = _messages.front();
        int32_t const diff = static_cast<int32_t>(msg.timestamp() - now);
        timeout            = (diff <= 0) ? 0U : static_cast<uint32_t>(diff);
        return true;
    }
    return false;
}

inline Message* Queue::try_dequeue(uint32_t const now)
{
    if (!_messages.empty())
    {
        Message& msg = _messages.front();
        if (static_cast<int32_t>(msg.timestamp() - now) <= 0)
        {
            _messages.pop_front();
            return &msg;
        }
    }
    return nullptr;
}

inline void Queue::clear() { _messages.clear(); }

} // namespace blocker
