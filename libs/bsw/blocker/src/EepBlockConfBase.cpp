// Copyright 2025 Accenture.

#include "blocker/EepBlockConfBase.h"

#include <estd/assert.h>

namespace blocker
{

bool EepBlockConfBase::isValidId(uint16_t const blockId) const
{
    if (blockId < fBlockCount)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint32_t EepBlockConfBase::getAddress(uint16_t const blockId) const
{
    estd_assert(isValidId(blockId));
    return getBlockPrefs(blockId).address;
}

size_t EepBlockConfBase::getBlockSize(uint16_t const blockId) const
{
    estd_assert(isValidId(blockId));
    return getBlockPrefs(blockId).blockSize;
}

} /* namespace blocker */
