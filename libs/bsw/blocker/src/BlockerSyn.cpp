// Copyright 2025 Accenture.

#include "blocker/BlockerSyn.h"

#include "blocker/BlockerConfig.h"
#include "blocker/IBlockerDriver.h"

#include <util/crc/Crc16.h>
#include <util/crc/Crc32.h>
#include <util/crc/Crc8.h>

#include <estd/assert.h>
#include <estd/big_endian.h>
#include <estd/memory.h>

namespace
{
uint8_t sTmpData[::blocker::BlockerConfig::MAX_LAYOUTLEN];
::estd::slice<uint8_t> const sTmpBuffer(sTmpData);
} // namespace

namespace blocker
{
using ::util::crc::Crc16;
using ::util::crc::Crc32;
using ::util::crc::Crc8;
using ::util::string::ConstString;

BlockerSyn::BlockerSyn(BlockerDrivers& blockerDrivers)
: ::estd::singleton<BlockerSyn>(*this), fBlockerDrivers(blockerDrivers)
{
    for (uint32_t blockId = 0U; BlockerConfig::isValidIntId(blockId); ++blockId)
    {
        BlockerConfig::setStorageTypeId(blockId, INVALID_DRIVER_ID);
    }
}

BlockerResult BlockerSyn::storeBlock(
    uint32_t const blockId,
    size_t const offset,
    uint8_t const* const writeData,
    size_t const length)
{
    if (!BlockerConfig::isInputValid(blockId, offset, writeData, length))
    {
        return BLOCKER_FAILED;
    }

    auto* const driverPtr = getBlockerDriver(blockId);
    if (driverPtr == nullptr)
    {
        return BLOCKER_FAILED;
    }

    auto const writeBuffer = ::estd::slice<uint8_t const>::from_pointer(writeData, length);
    StoreOperation op(offset, writeBuffer, *driverPtr);
    return op.execute(blockId);
}

BlockerResult BlockerSyn::loadBlock(
    uint32_t const blockId, size_t const offset, uint8_t* const readData, size_t const length)
{
    if (!BlockerConfig::isInputValid(blockId, offset, readData, length))
    {
        return BLOCKER_FAILED;
    }

    auto* const driverPtr = getBlockerDriver(blockId);
    if (driverPtr == nullptr)
    {
        return BLOCKER_FAILED;
    }

    auto readBuffer = ::estd::slice<uint8_t>::from_pointer(readData, length);
    LoadOperation op(offset, readBuffer, *driverPtr);
    return op.execute(blockId);
}

void BlockerSyn::addBlockerDriver(IBlockerDriver& driver)
{
    estd_assert(driver.getId() != INVALID_DRIVER_ID);
    for (auto itr = fBlockerDrivers.cbegin(); itr != fBlockerDrivers.cend(); ++itr)
    {
        estd_assert((*itr)->getId() != driver.getId());
    }

    for (uint32_t blockId = 0U; BlockerConfig::isValidIntId(blockId); ++blockId)
    {
        if (BlockerConfig::getStorageType(blockId).compare(ConstString(driver.getName())) == 0)
        {
            BlockerConfig::setStorageTypeId(blockId, driver.getId());
        }
    }

    fBlockerDrivers.push_back(&driver);
}

IBlockerDriver* BlockerSyn::getBlockerDriver(uint32_t const blockId) const
{
    auto const storageTypeId = BlockerConfig::getStorageTypeId(blockId);
    for (auto itr = fBlockerDrivers.cbegin(); itr != fBlockerDrivers.cend(); ++itr)
    {
        if ((*itr)->getId() == storageTypeId)
        {
            return *itr;
        }
    }
    return nullptr;
}

bool BlockerSyn::isCrcValid(
    ::estd::slice<uint8_t const> const& payload, ::estd::slice<uint8_t const> const& checksum)
{
    switch (checksum.size())
    {
        case 1U:
        {
            BlockerConfig::BlockerCrc8 c;
            (void)c.update(payload.data(), payload.size());
            return (c.digest() == checksum[0U]);
        }
        case 2U:
        {
            BlockerConfig::BlockerCrc16 c;
            (void)c.update(payload.data(), payload.size());
            return (c.digest() == ::estd::read_be<uint16_t>(checksum.data()));
        }
        case 4U:
        {
            BlockerConfig::BlockerCrc32 c;
            (void)c.update(payload.data(), payload.size());
            return (c.digest() == ::estd::read_be<uint32_t>(checksum.data()));
        }
        default:
        {
            return true;
        }
    }
}

void BlockerSyn::calcCrc(
    ::estd::slice<uint8_t const> const& payload, ::estd::slice<uint8_t> const& result)
{
    switch (result.size())
    {
        case 1U:
        {
            BlockerConfig::BlockerCrc8 c;
            (void)c.update(payload.data(), payload.size());
            result[0U] = c.digest();
            break;
        }
        case 2U:
        {
            BlockerConfig::BlockerCrc16 c;
            (void)c.update(payload.data(), payload.size());
            ::estd::write_be<uint16_t>(result.data(), c.digest());
            break;
        }
        case 4U:
        {
            BlockerConfig::BlockerCrc32 c;
            (void)c.update(payload.data(), payload.size());
            ::estd::write_be<uint32_t>(result.data(), c.digest());
            break;
        }
        default:
        {
            break;
        }
    }
}

BlockerResult BlockerSyn::StoreOperation::execute(uint32_t const blockId)
{
    if (BlockerConfig::hasCrc(blockId))
    {
        return storeWithCrc(blockId);
    }
    else
    {
        return storeData(blockId, fWriteBuffer);
    }
}

BlockerResult BlockerSyn::StoreOperation::storeData(
    uint32_t const blockId, ::estd::slice<uint8_t const> const& data)
{
    auto const driverBlockId = BlockerConfig::getDriverBlockId(blockId);
    auto const offset        = getOffset();
    if (getDriver().write(driverBlockId, offset, data))
    {
        return BLOCKER_OK;
    }
    else
    {
        return BLOCKER_FAILED;
    }
}

BlockerResult BlockerSyn::StoreOperation::storeWithCrc(uint32_t const blockId)
{
    auto const layoutSize = BlockerConfig::getLayoutSize(blockId);
    if ((fWriteBuffer.size() < BlockerConfig::getBlockSize(blockId)))
    {
        auto const res = readExistingData(blockId);
        if (res != BLOCKER_OK)
        {
            return res;
        }
    }
    auto checksum     = sTmpBuffer.subslice(layoutSize);
    auto payload      = ::estd::memory::split(checksum, BlockerConfig::getBlockSize(blockId));
    auto const offset = getOffset();
    (void)::estd::memory::copy(payload.offset(offset), fWriteBuffer);
    calcCrc(payload, checksum);
    auto const offsetBuffer = sTmpBuffer.subslice(layoutSize).advance(offset);

    if (storeData(blockId, offsetBuffer) == BLOCKER_OK)
    {
        return BLOCKER_OK;
    }
    else
    {
        return BLOCKER_FAILED;
    }
}

BlockerResult BlockerSyn::StoreOperation::readExistingData(uint32_t const blockId)
{
    auto tmpBuffer = sTmpBuffer.subslice(BlockerConfig::getLayoutSize(blockId));
    if (getDriver().read(BlockerConfig::getDriverBlockId(blockId), 0U, tmpBuffer))
    {
        return BLOCKER_OK;
    }
    else
    {
        return BLOCKER_FAILED;
    }
}

BlockerResult BlockerSyn::LoadOperation::execute(uint32_t const blockId)
{
    if (BlockerConfig::hasCrc(blockId))
    {
        return loadWithCrc(blockId);
    }
    else
    {
        return loadNoCrc(blockId);
    }
}

BlockerResult BlockerSyn::LoadOperation::loadWithCrc(uint32_t const blockId)
{
    auto const layoutSize    = BlockerConfig::getLayoutSize(blockId);
    auto checksum            = sTmpBuffer.subslice(layoutSize);
    auto const driverBlockId = BlockerConfig::getDriverBlockId(blockId);
    if (getDriver().read(driverBlockId, 0U, checksum))
    {
        auto const payload = ::estd::memory::split(checksum, BlockerConfig::getBlockSize(blockId));
        if (isCrcValid(payload, checksum))
        {
            (void)::estd::memory::copy(
                fReadBuffer, payload.offset(getOffset()).trim(fReadBuffer.size()));
            return BLOCKER_OK;
        }
    }

    return BLOCKER_FAILED;
}

BlockerResult BlockerSyn::LoadOperation::loadNoCrc(uint32_t const blockId)
{
    if (getDriver().read(BlockerConfig::getDriverBlockId(blockId), getOffset(), fReadBuffer))
    {
        return BLOCKER_OK;
    }
    else
    {
        return BLOCKER_FAILED;
    }
}

} /* namespace blocker */
