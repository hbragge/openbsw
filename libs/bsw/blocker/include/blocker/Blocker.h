// Copyright 2025 Accenture.

#pragma once

#include "blocker/BlockerConfig.h"
#include "blocker/BlockerHan.h"
#include "blocker/Handler.h"
#include "blocker/Message.h"

#include <estd/object_pool.h>
#include <estd/ordered_map.h>
#include <estd/singleton.h>

namespace blocker
{
class MsgPasser;

class BlockerJob
{
public:
    explicit BlockerJob(Handler* hnd);

    BlockerReq& getReq() { return req; }

    Message& getMsg() { return msg; }

private:
    BlockerReq req;
    Message msg;
};

class Blocker : public ::estd::singleton<Blocker>
{
public:
    bool sendStoreBlock(
        uint32_t blockId,
        size_t offset,
        uint8_t const* writeData,
        size_t length,
        JobDoneCallback jobDone);
    bool sendLoadBlock(
        uint32_t blockId, size_t offset, uint8_t* readData, size_t length, JobDoneCallback jobDone);

    void shutdown();
    void addRespHandler(uint32_t taskId, Handler& hnd) const;
    void releaseJob(BlockerJob const* job);

protected:
    typedef ::estd::ordered_map<uint32_t, Handler*> RespHandlers;
    typedef ::estd::object_pool<BlockerJob> BlockerJobPool;
    Blocker(MsgPasser& MsgPasser, RespHandlers& respHandlers, BlockerJobPool& jobPool);

private:
    Handler* getRespHandler() const;
    bool sendBlockerReq(
        uint32_t blockId,
        size_t offset,
        uint8_t* buf,
        size_t length,
        BlockerOpType reqOp,
        JobDoneCallback jobDone);

    void jobDoneDummyCb(BlockerOpType const, BlockerResult const, uint32_t const) {}

    JobDoneCallback fJobDoneDummy;

    BlockerJobPool& fJobPool;
    RespHandlers& fRespHandlers;
    BlockerHan fBlockerHan;
};

namespace declare
{
template<std::size_t MAX_REQS, std::size_t MAX_RESP_HANDLERS>
class Blocker : public blocker::Blocker
{
public:
    explicit Blocker(MsgPasser& MsgPasser) : blocker::Blocker(MsgPasser, fRespHandlers, fJobPool) {}

private:
    ::estd::declare::object_pool<BlockerJob, MAX_REQS> fJobPool;
    ::estd::declare::ordered_map<uint32_t, Handler*, MAX_RESP_HANDLERS> fRespHandlers;
};
} /* namespace declare */

} /* namespace blocker */
