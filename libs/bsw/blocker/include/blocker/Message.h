// Copyright 2025 Accenture.

#pragma once

#include <util/types/Enum.h>

#include <estd/chrono.h>
#include <estd/forward_list.h>
#include <platform/estdint.h>

namespace blocker
{
class Handler;

class Message : public ::estd::forward_list_node<Message>
{
public:
    explicit Message(Handler& h);
    Message(Handler& h, uint16_t w);
    Handler& get_hnd() const;
    void set_timestamp(uint32_t ts);
    Message* prepare(uint16_t w, void* o);
    Message* repurpose(Handler& h);
    bool fire();
    uint32_t timestamp() const;
    uint16_t what() const;
    void* obj() const;

private:
    Handler* _handler;
    void* _obj;
    uint32_t _timestamp;
    uint16_t _what;
};

inline Message::Message(Handler& h) : _handler(&h), _obj(nullptr), _timestamp(0U), _what(0U) {}

inline Message::Message(Handler& h, uint16_t const w)
: _handler(&h), _obj(nullptr), _timestamp(0U), _what(w)
{}

inline Handler& Message::get_hnd() const { return *_handler; }

inline void Message::set_timestamp(uint32_t const ts) { _timestamp = ts; }

inline uint32_t Message::timestamp() const { return _timestamp; }

inline uint16_t Message::what() const { return _what; }

inline void* Message::obj() const { return _obj; }

} // namespace blocker
