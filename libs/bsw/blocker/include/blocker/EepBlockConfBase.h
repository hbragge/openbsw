// Copyright 2025 Accenture.

#pragma once

#include "blocker/IBlockerDriver.h"

#include <platform/estdint.h>

namespace blocker
{
class EepBlockConfBase
{
public:
    explicit EepBlockConfBase(size_t const blockCount) : fBlockCount(blockCount) {}

    bool isValidId(uint16_t blockId) const;
    uint32_t getAddress(uint16_t blockId) const;
    size_t getBlockSize(uint16_t blockId) const;

    size_t getBlockCount() const { return fBlockCount; }

    virtual char const* getName() const = 0;
    virtual uint8_t getId() const       = 0;

protected:
    struct EepBlockPrefs
    {
        size_t blockSize;
        uint32_t address;
    };

    virtual EepBlockPrefs const& getBlockPrefs(uint16_t blockId) const = 0;

private:
    size_t const fBlockCount;
};

} /* namespace blocker */
