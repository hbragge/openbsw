// Copyright 2025 Accenture.

#include "blocker/BlockerConfig.h"

#include <estd/assert.h>

namespace blocker
{

using ::util::string::ConstString;

BlockerConfig::BlockerPrefs const BlockerConfig::fBlockPrefs[] = {
    {10U, BLOCKER_CRC_NONE, "eep", 0U},
    {11U, BLOCKER_CRC_8, "eep", 1U},
};

uint8_t BlockerConfig::fStorageTypeId[BlockerConfig::INTBLOCKCOUNT];

bool BlockerConfig::isValidIntId(uint32_t const blockId) { return (blockId < INTBLOCKCOUNT); }

bool BlockerConfig::isValidId(uint32_t const blockId) { return (blockId < BLOCKCOUNT); }

uint16_t BlockerConfig::getDriverBlockId(uint32_t const blockId)
{
    estd_assert(isValidIntId(blockId));
    return (fBlockPrefs[blockId].driverBlockId);
}

size_t BlockerConfig::getLayoutSize(uint32_t const blockId)
{
    if (isValidIntId(blockId))
    {
        return (fBlockPrefs[blockId].layoutSize);
    }
    else
    {
        return 0U;
    }
}

ConstString BlockerConfig::getStorageType(uint32_t const blockId)
{
    estd_assert(isValidIntId(blockId));
    return ConstString(fBlockPrefs[blockId].storageType);
}

uint8_t BlockerConfig::getStorageTypeId(uint32_t const blockId)
{
    estd_assert(isValidIntId(blockId));
    return (fStorageTypeId[blockId]);
}

void BlockerConfig::setStorageTypeId(uint32_t const blockId, uint8_t const storageTypeId)
{
    estd_assert(isValidIntId(blockId));
    fStorageTypeId[blockId] = storageTypeId;
}

size_t BlockerConfig::getCrcLength(uint32_t const blockId)
{
    if (!isValidIntId(blockId))
    {
        return 0U;
    }
    switch (fBlockPrefs[blockId].checksum)
    {
        case BLOCKER_CRC_8:
        {
            return 1U;
        }
        case BLOCKER_CRC_16:
        {
            return 2U;
        }
        case BLOCKER_CRC_32:
        {
            return 4U;
        }
        default:
        {
            return 0U;
        }
    }
}

bool BlockerConfig::hasCrc(uint32_t const blockId) { return (getCrcLength(blockId) > 0U); }

size_t BlockerConfig::getBlockSize(uint32_t const blockId)
{
    return (getLayoutSize(blockId) - getCrcLength(blockId));
}

} /* namespace blocker */
