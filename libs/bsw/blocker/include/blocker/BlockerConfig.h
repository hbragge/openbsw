// Copyright 2025 Accenture.

#pragma once

#include "blocker/BlockerTypes.h"
#include "blocker/IBlockerDriver.h"

#include <util/crc/Crc.h>
#include <util/crc/Crc16.h>
#include <util/crc/Crc32.h>
#include <util/string/ConstString.h>

#include <platform/estdint.h>

namespace blocker
{

enum BlockerChecksum
{
    BLOCKER_CRC_NONE,
    BLOCKER_CRC_8,
    BLOCKER_CRC_16,
    BLOCKER_CRC_32
};

class BlockerConfig
{
public:
    static uint32_t const EepBlock1 = 0U;
    static uint32_t const EepBlock2 = 1U;

    static uint32_t const BLOCKCOUNT     = 2U;
    static uint32_t const INTBLOCKCOUNT  = 2U;
    static size_t const MAX_BLOCKLEN     = 10U;
    static size_t const MAX_LAYOUTLEN    = 11U;
    static uint8_t const INVALID_PATTERN = 0xFFU;

    static bool isValidIntId(uint32_t blockId);
    static bool isValidId(uint32_t blockId);
    static uint16_t getDriverBlockId(uint32_t blockId);
    static size_t getLayoutSize(uint32_t blockId);
    static util::string::ConstString getStorageType(uint32_t blockId);
    static uint8_t getStorageTypeId(uint32_t blockId);
    static void setStorageTypeId(uint32_t blockId, uint8_t storageTypeId);
    static size_t getCrcLength(uint32_t blockId);
    static bool hasCrc(uint32_t blockId);

    static size_t getCrcVersion() { return 0U; }

    static size_t getBlockSize(uint32_t blockId);

    static bool isInputValid(
        uint32_t const blockId,
        size_t const offset,
        uint8_t const* const buf,
        size_t const length,
        bool const allowEmpty = false)
    {
        size_t const total = offset + length;
        if (total < offset)
        {
            return false;
        }
        if ((((buf == nullptr) || (length == 0U)) && (!allowEmpty)) || (!isValidId(blockId))
            || ((offset + length) > getBlockSize(blockId)))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    typedef ::util::crc::CrcRegister<uint8_t, 0x1DU, 0x3CU> BlockerCrc8;
    typedef ::util::crc::Crc16::Ccitt BlockerCrc16;
    typedef ::util::crc::Crc32::Ethernet BlockerCrc32;

private:
    struct BlockerPrefs
    {
        size_t layoutSize;
        BlockerChecksum checksum;
        char const* storageType;
        uint16_t driverBlockId;
    };

    static BlockerPrefs const fBlockPrefs[INTBLOCKCOUNT];

    static uint8_t fStorageTypeId[INTBLOCKCOUNT];
};

} /* namespace blocker */
