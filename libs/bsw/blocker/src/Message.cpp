// Copyright 2025 Accenture.

#include "blocker/Message.h"

#include "blocker/Handler.h"
#include "blocker/MsgPasser.h"

#include <async/Types.h>

namespace blocker
{
bool Message::fire() { return _handler->get_blocker().send(*this, 0U); }

Message* Message::prepare(uint16_t const w, void* const o)
{
    ::async::LockType const lock;
    if (::estd::is_in_use(*this))
    {
        return nullptr;
    }
    _what = w;
    _obj  = o;
    return this;
}

Message* Message::repurpose(Handler& h)
{
    ::async::LockType const lock;
    if (::estd::is_in_use(*this))
    {
        return nullptr;
    }
    _handler = &h;
    return this;
}

} // namespace blocker
