// Copyright 2025 Accenture.

#include "blocker/Blocker.h"

#include "blocker/BlockerConfig.h"
#include "blocker/Handler.h"
#include "blocker/MsgPasser.h"

#include <async/Types.h>

extern void GetTaskID(uint32_t*);

namespace blocker
{

BlockerJob::BlockerJob(Handler* const hnd) : msg(*hnd){};

Blocker::Blocker(MsgPasser& MsgPasser, RespHandlers& respHandlers, BlockerJobPool& jobPool)
: ::estd::singleton<Blocker>(*this)
, fJobDoneDummy(JobDoneCallback::create<Blocker, &Blocker::jobDoneDummyCb>(*this))
, fJobPool(jobPool)
, fRespHandlers(respHandlers)
, fBlockerHan(MsgPasser)
{}

bool Blocker::sendStoreBlock(
    uint32_t const blockId,
    size_t const offset,
    uint8_t const* const writeData,
    size_t const length,
    JobDoneCallback const jobDone)
{
    return sendBlockerReq(
        blockId, offset, const_cast<uint8_t*>(writeData), length, BLOCKER_STORE, jobDone);
}

bool Blocker::sendLoadBlock(
    uint32_t const blockId,
    size_t const offset,
    uint8_t* const readData,
    size_t const length,
    JobDoneCallback const jobDone)
{
    return sendBlockerReq(blockId, offset, readData, length, BLOCKER_LOAD, jobDone);
}

void Blocker::releaseJob(BlockerJob const* const job)
{
    if (job != nullptr)
    {
        ::async::LockType const lock;
        fJobPool.release(*job);
    }
}

void Blocker::shutdown()
{
    ::async::LockType const lock;
    fJobPool.clear();
}

void Blocker::addRespHandler(uint32_t const taskId, Handler& hnd) const
{
    fRespHandlers[taskId] = &hnd;
}

bool Blocker::sendBlockerReq(
    uint32_t const blockId,
    size_t const offset,
    uint8_t* const buf,
    size_t const length,
    BlockerOpType const reqOp,
    JobDoneCallback const jobDone)
{
    if (!BlockerConfig::isInputValid(blockId, offset, buf, length))
    {
        return false;
    }

    BlockerJob* jobPtr = nullptr;

    {
        ::async::LockType const lock;
        if (fJobPool.empty())
        {
            return false;
        }
        else
        {
            auto c = fJobPool.allocate();
            jobPtr = &(c.construct(&fBlockerHan));
        }
    }

    auto& req = jobPtr->getReq();
    if ((jobPtr->getMsg().prepare(static_cast<uint16_t>(reqOp), &req)) == nullptr)
    {
        releaseJob(jobPtr);
        return false;
    }
    req.blockId     = blockId;
    req.offset      = offset;
    req.buf         = buf;
    req.length      = length;
    req.jobDone     = jobDone;
    req.respHandler = getRespHandler();
    req.jobObj      = jobPtr;
    (void)jobPtr->getMsg().fire();
    return true;
}

Handler* Blocker::getRespHandler() const
{
    uint32_t curTaskId;
    (void)GetTaskID(&curTaskId);
    curTaskId &= 0xFFU;
    if (fRespHandlers.count(curTaskId) == 0U)
    {
        return nullptr;
    }
    else
    {
        return fRespHandlers[curTaskId];
    }
}

} /* namespace blocker */
