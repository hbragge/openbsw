// Copyright 2025 Accenture.

#pragma once

#include <estd/slice.h>
#include <platform/estdint.h>

namespace blocker
{

class IBlockerDriver
{
public:
    IBlockerDriver& operator=(IBlockerDriver const& other) = delete;

    virtual char const* getName() const                                                   = 0;
    virtual uint8_t getId() const                                                         = 0;
    virtual bool init(bool wakeUp = false)                                                = 0;
    virtual bool read(uint16_t blockId, size_t offset, ::estd::slice<uint8_t>& dst) const = 0;
    virtual bool write(uint16_t blockId, size_t offset, ::estd::slice<uint8_t const> const& src)
        = 0;
};

} /* namespace blocker */
