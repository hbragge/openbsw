// Copyright 2025 Accenture.

#pragma once

#include "blocker/BlockerTypes.h"
#include "blocker/IBlockerDriver.h"

#include <estd/singleton.h>
#include <estd/slice.h>
#include <estd/vector.h>

namespace blocker
{

class BlockerSyn : public ::estd::singleton<BlockerSyn>
{
public:
    BlockerResult
    storeBlock(uint32_t blockId, size_t offset, uint8_t const* writeData, size_t length);
    BlockerResult loadBlock(uint32_t blockId, size_t offset, uint8_t* readData, size_t length);

    void addBlockerDriver(IBlockerDriver& driver);

protected:
    typedef ::estd::vector<IBlockerDriver*> BlockerDrivers;
    explicit BlockerSyn(BlockerDrivers& blockerDrivers);

private:
    class BlockerOp
    {
    public:
        explicit BlockerOp(size_t const offset, IBlockerDriver& driver)
        : fDriver(driver), fOffset(offset)
        {}

    protected:
        virtual uint32_t getCurrOrPrev(uint32_t curr, uint32_t prev) const = 0;

        IBlockerDriver& getDriver() { return fDriver; }

        size_t getOffset() const { return fOffset; }

    private:
        IBlockerDriver& fDriver;
        size_t const fOffset;
    };

    class LoadOperation : public BlockerOp
    {
    public:
        explicit LoadOperation(
            size_t const offset, ::estd::slice<uint8_t>& readBuffer, IBlockerDriver& driver)
        : BlockerOp(offset, driver), fReadBuffer(readBuffer)
        {}

        BlockerResult execute(uint32_t blockId);

    private:
        BlockerResult loadWithCrc(uint32_t blockId);
        BlockerResult loadNoCrc(uint32_t blockId);

        uint32_t getCurrOrPrev(uint32_t const /*curr*/, uint32_t const prev) const override
        {
            return prev;
        }

        ::estd::slice<uint8_t>& fReadBuffer;
    };

    class StoreOperation : public BlockerOp
    {
    public:
        explicit StoreOperation(
            size_t const offset,
            ::estd::slice<uint8_t const> const& writeBuffer,
            IBlockerDriver& driver)
        : BlockerOp(offset, driver), fWriteBuffer(writeBuffer)
        {}

        BlockerResult execute(uint32_t blockId);

    private:
        BlockerResult storeData(uint32_t blockId, ::estd::slice<uint8_t const> const& data);
        BlockerResult storeWithCrc(uint32_t blockId);
        BlockerResult readExistingData(uint32_t blockId);

        uint32_t getCurrOrPrev(uint32_t const curr, uint32_t const /*prev*/) const override
        {
            return curr;
        }

        ::estd::slice<uint8_t const> const& fWriteBuffer;
    };

    IBlockerDriver* getBlockerDriver(uint32_t blockId) const;
    static bool isCrcValid(
        ::estd::slice<uint8_t const> const& payload, ::estd::slice<uint8_t const> const& checksum);
    static void
    calcCrc(::estd::slice<uint8_t const> const& payload, ::estd::slice<uint8_t> const& result);
    static BlockerResult
    recoverDefaultData(uint32_t blockId, size_t offset, ::estd::slice<uint8_t> const& readBuffer);

    BlockerDrivers& fBlockerDrivers;
    static uint8_t const INVALID_DRIVER_ID = 255U;
};

namespace declare
{
template<std::size_t MAX_DRIVERS>
class BlockerSyn : public blocker::BlockerSyn
{
public:
    BlockerSyn() : blocker::BlockerSyn(fBlockerDrivers){};

private:
    ::estd::declare::vector<IBlockerDriver*, MAX_DRIVERS> fBlockerDrivers;
};
} /* namespace declare */

} /* namespace blocker */
