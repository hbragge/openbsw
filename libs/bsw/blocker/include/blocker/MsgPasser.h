// Copyright 2025 Accenture.

#pragma once

#include "blocker/Queue.h"

#include <bsp/timer/SystemTimer.h>

namespace blocker
{
class MsgPasser
{
public:
    void init();
    void shutdown();
    void loop();
    bool send(Message& msg, uint32_t delayInMs);
    void clear();

private:
    static uint32_t get_timestamp();
    bool handle_next_message();

    Queue _queue;
};

inline void MsgPasser::init() {}

inline void MsgPasser::shutdown() { clear(); }

inline void MsgPasser::loop()
{
    while (handle_next_message()) {}
}

inline bool MsgPasser::send(Message& msg, uint32_t delay)
{
    if (!::estd::is_in_use(msg))
    {
        uint32_t const now = get_timestamp();
        msg.set_timestamp(now + delay);
        _queue.send(now, msg);
        return true;
    }
    else
    {
        return false;
    }
}

inline void MsgPasser::clear() { _queue.clear(); }

inline bool MsgPasser::handle_next_message()
{
    Message* const msg = _queue.try_dequeue(get_timestamp());
    if (msg != nullptr)
    {
        msg->get_hnd().handle_msg(*msg);
        return true;
    }
    else
    {
        return false;
    }
}

inline uint32_t MsgPasser::get_timestamp()
{
    return static_cast<uint32_t>(getSystemTimeNs() / 1000000UL);
}

} // namespace blocker
