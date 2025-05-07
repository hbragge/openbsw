// Copyright 2025 Accenture.

#include "blocker/Blocker.h"

#include "blocker/BlockerConfig.h"
#include "blocker/BlockerSyn.h"
#include "blocker/EepBlockConf.h"
#include "blocker/EepBlockerDriver.h"
#include "blocker/EepromDriverMock.h"
#include "blocker/MsgPasser.h"

#include <estd/assert.h>
#include <estd/big_endian.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace test
{
using ::util::string::ConstString;

using namespace ::blocker;
using namespace ::testing;

struct BlockProps
{
    uint32_t blockId;
    uint32_t baseAddress;
};

static uint64_t systemTimeNs = 1000000U;

extern "C" uint64_t getSystemTimeNs() { return systemTimeNs; }

class BlockerTest : public ::testing::Test
{
public:
    BlockerTest()
    : fRespHandler(fResponseMsgPasser)
    , fEepBlockerDriver(fEepMock, fEepBlockConf)
    , fBlocker(fMsgPasser)
    {}

    virtual void SetUp()
    {
        fBlocker.addRespHandler(getTaskId(), fRespHandler);
        fResponseMsgPasser.init();
        fMsgPasser.init();

        fBlockerSyn.addBlockerDriver(fEepBlockerDriver);

        for (uint32_t blockId = 0; BlockerConfig::isValidIntId(blockId); ++blockId)
        {
            memset(fBlockerData[blockId], DATA_INIT, LAYOUTSIZE);
        }

        memset(fReadData, 0, BLOCKSIZE);
        memset(fUserData, USER_DATA_INIT, BLOCKSIZE);

        jobdone_reset();
    }

    virtual void TearDown() {}

    static uint32_t getTaskId() { return fTaskId; }

    BlockProps eepAddr2Props(uint32_t const address)
    {
        BlockProps properties;
        for (uint16_t eepId = 0U; eepId < fEepBlockConf.getBlockCount(); ++eepId)
        {
            if (address >= fEepBlockConf.getAddress(eepId)
                && (address
                    < (fEepBlockConf.getAddress(eepId) + fEepBlockConf.getBlockSize(eepId))))
            {
                properties.baseAddress = fEepBlockConf.getAddress(eepId);
                for (uint32_t blockId = 0; BlockerConfig::isValidIntId(blockId); ++blockId)
                {
                    if ((BlockerConfig::getDriverBlockId(blockId) == eepId)
                        && (BlockerConfig::getStorageType(blockId).compare(ConstString("eep"))
                            == 0))
                    {
                        properties.blockId = blockId;
                        return properties;
                    }
                }
            }
        }
        estd_assert(false);
        return properties;
    }

    void EepRead(uint32_t address, uint8_t* dst, uint32_t length)
    {
        auto const properties = eepAddr2Props(address);
        auto const blockId    = properties.blockId;
        auto const offset     = static_cast<size_t>(address - properties.baseAddress);
        (void)memcpy(dst, fBlockerData[blockId] + offset, length);
    }

    void EepWrite(uint32_t address, uint8_t const* src, uint32_t length)
    {
        auto const blockId = eepAddr2Props(address).blockId;
        auto const offset  = static_cast<size_t>(address - eepAddr2Props(address).baseAddress);
        (void)memcpy(fBlockerData[blockId] + offset, src, length);
    }

    void msgpasser_wait()
    {
        fMsgPasser.loop();
        fResponseMsgPasser.loop();
    }

    void jobdone_verify(uint16_t jobs = 1)
    {
        msgpasser_wait();
        estd_assert(fJobCount == jobs);
    }

    void jobdone_reset()
    {
        for (uint32_t blockId = 0U; BlockerConfig::isValidId(blockId); ++blockId)
        {
            fJobResult[blockId] = BLOCKER_FAILED;
        }
        fJobCount = 0U;
    }

    static void jobdone_cb(BlockerResult const result, uint32_t const blockId)
    {
        estd_assert(BlockerConfig::isValidId(blockId));
        fJobResult[blockId] = result;
        ++fJobCount;
    }

    static void readjobdone_cb(
        BlockerOpType const serviceId, BlockerResult const result, uint32_t const blockId)
    {
        estd_assert(serviceId == BLOCKER_LOAD);
        jobdone_cb(result, blockId);
    }

    static void writejobdone_cb(
        BlockerOpType const serviceId, BlockerResult const result, uint32_t const blockId)
    {
        estd_assert(serviceId == BLOCKER_STORE);
        jobdone_cb(result, blockId);
    }

    static size_t const BLOCKSIZE       = 10U;
    static uint8_t const USER_DATA_INIT = 42U;
    static uint8_t const DATA_INIT      = 33U;

protected:
    MsgPasser fResponseMsgPasser;
    MsgPasser fMsgPasser;
    RespHandler fRespHandler;

    static size_t const LAYOUTSIZE = 36U;

    EepBlockConf fEepBlockConf;

    ::eeprom::EepromDriverMock fEepMock;
    EepBlockerDriver fEepBlockerDriver;
    ::blocker::declare::BlockerSyn<1U> fBlockerSyn;
    ::blocker::declare::Blocker<6, 3> fBlocker;
    uint8_t fBlockerData[BlockerConfig::INTBLOCKCOUNT][LAYOUTSIZE];
    uint8_t fReadData[BLOCKSIZE];
    uint8_t fUserData[BLOCKSIZE];

    static BlockerResult fJobResult[];
    static uint16_t fJobCount;
    static uint32_t fTaskId;
};

uint8_t const BlockerTest::USER_DATA_INIT;
uint8_t const BlockerTest::DATA_INIT;
uint32_t BlockerTest::fTaskId = 1U;

BlockerResult BlockerTest::fJobResult[BlockerConfig::BLOCKCOUNT];
uint16_t BlockerTest::fJobCount;

TEST_F(BlockerTest, EepCrcOk)
{
    EXPECT_CALL(fEepMock, read(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepRead), Return(bsp::BSP_OK)));
    EXPECT_CALL(fEepMock, write(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepWrite), Return(bsp::BSP_OK)));
    auto const blockId      = BlockerConfig::EepBlock2;
    auto const writejobdone = JobDoneCallback::create<&writejobdone_cb>();
    bool result             = fBlocker.sendStoreBlock(blockId, 0, fUserData, 10, writejobdone);
    ASSERT_TRUE(result);
    jobdone_verify();
    ASSERT_EQ(BLOCKER_OK, fJobResult[blockId]);
    jobdone_reset();
    ASSERT_EQ(fBlockerData[blockId][0], USER_DATA_INIT);
    ASSERT_EQ(fBlockerData[blockId][9], USER_DATA_INIT);
    auto const readjobdone = JobDoneCallback::create<&readjobdone_cb>();
    result                 = fBlocker.sendLoadBlock(blockId, 0, fReadData, 10, readjobdone);
    ASSERT_TRUE(result);
    msgpasser_wait();
    jobdone_verify();
    ASSERT_EQ(BLOCKER_OK, fJobResult[blockId]);
    ASSERT_EQ(fReadData[0], USER_DATA_INIT);
    ASSERT_EQ(fReadData[9], USER_DATA_INIT);
}

TEST_F(BlockerTest, EepPartial)
{
    EXPECT_CALL(fEepMock, read(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepRead), Return(bsp::BSP_OK)));
    EXPECT_CALL(fEepMock, write(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepWrite), Return(bsp::BSP_OK)));
    auto const blockId      = BlockerConfig::EepBlock1;
    auto const writejobdone = JobDoneCallback::create<&writejobdone_cb>();
    bool result             = fBlocker.sendStoreBlock(blockId, 0, fUserData, 8, writejobdone);
    ASSERT_TRUE(result);
    jobdone_verify();
    ASSERT_EQ(BLOCKER_OK, fJobResult[blockId]);
    jobdone_reset();
    ASSERT_EQ(fBlockerData[blockId][0], USER_DATA_INIT);
    ASSERT_EQ(fBlockerData[blockId][7], USER_DATA_INIT);
    ASSERT_EQ(fBlockerData[blockId][8], DATA_INIT);
    auto const readjobdone = JobDoneCallback::create<&readjobdone_cb>();
    result                 = fBlocker.sendLoadBlock(blockId, 0, fReadData, 4, readjobdone);
    ASSERT_TRUE(result);
    msgpasser_wait();
    jobdone_verify();
    ASSERT_EQ(BLOCKER_OK, fJobResult[blockId]);
    ASSERT_EQ(fReadData[0], USER_DATA_INIT);
    ASSERT_EQ(fReadData[3], USER_DATA_INIT);
    ASSERT_EQ(fReadData[4], 0);
}

TEST_F(BlockerTest, EepFail)
{
    EXPECT_CALL(fEepMock, read(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepRead), Return(bsp::BSP_ERROR)));
    EXPECT_CALL(fEepMock, write(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepWrite), Return(bsp::BSP_ERROR)));
    auto const blockId      = BlockerConfig::EepBlock1;
    auto const writejobdone = JobDoneCallback::create<&writejobdone_cb>();
    bool result             = fBlocker.sendStoreBlock(blockId, 0, fUserData, 1, writejobdone);
    ASSERT_TRUE(result);
    jobdone_verify();
    ASSERT_EQ(BLOCKER_FAILED, fJobResult[blockId]);
    jobdone_reset();
    auto const readjobdone = JobDoneCallback::create<&readjobdone_cb>();
    result                 = fBlocker.sendLoadBlock(blockId, 0, fReadData, 1, readjobdone);
    ASSERT_TRUE(result);
    jobdone_verify();
    ASSERT_EQ(BLOCKER_FAILED, fJobResult[blockId]);
}

TEST_F(BlockerTest, EepCrcFail)
{
    EXPECT_CALL(fEepMock, read(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepRead), Return(bsp::BSP_ERROR)));
    EXPECT_CALL(fEepMock, write(_, _, _))
        .WillRepeatedly(DoAll(Invoke(this, &BlockerTest::EepWrite), Return(bsp::BSP_ERROR)));
    auto const blockId      = BlockerConfig::EepBlock2;
    auto const writejobdone = JobDoneCallback::create<&writejobdone_cb>();
    bool result             = fBlocker.sendStoreBlock(blockId, 0, fUserData, 1, writejobdone);
    ASSERT_TRUE(result);
    jobdone_verify();
    ASSERT_EQ(BLOCKER_FAILED, fJobResult[blockId]);
    jobdone_reset();
    auto const readjobdone = JobDoneCallback::create<&readjobdone_cb>();
    result                 = fBlocker.sendLoadBlock(blockId, 0, fReadData, 1, readjobdone);
    ASSERT_TRUE(result);
    jobdone_verify();
    ASSERT_EQ(BLOCKER_FAILED, fJobResult[blockId]);
}

} /* namespace test */

void GetTaskID(uint32_t* taskId) { *taskId = test::BlockerTest::getTaskId(); }
