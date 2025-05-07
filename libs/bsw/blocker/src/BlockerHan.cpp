// Copyright 2025 Accenture.

#include "blocker/BlockerHan.h"

#include "blocker/Blocker.h"
#include "blocker/BlockerConfig.h"
#include "blocker/BlockerSyn.h"

namespace blocker
{

BlockerHan::BlockerHan(MsgPasser& msgPasser) : Handler(msgPasser) {}

void BlockerHan::handle_msg(Message const& msg)
{
    auto* const reqPtr = static_cast<BlockerReq*>(msg.obj());
    auto& req          = *reqPtr;

    auto const reqOp = static_cast<BlockerOpType>(msg.what());
    req.result       = handleRequest(reqOp, req);
    if (req.respHandler == nullptr)
    {
        RespHandler::runCallbacks(reqOp, req);
    }
    else
    {
        auto& acquiredMsg = req.jobObj->getMsg();
        if (((acquiredMsg.repurpose(*(req.respHandler))) == nullptr)
            || ((acquiredMsg.prepare(msg.what(), reqPtr)) == nullptr))
        {
            Blocker::instance().releaseJob(req.jobObj);
            return;
        }
        (void)req.jobObj->getMsg().fire();
    }
}

BlockerResult BlockerHan::handleRequest(BlockerOpType const reqOp, BlockerReq const& req) const
{
    if (reqOp == BLOCKER_LOAD)
    {
        return BlockerSyn::instance().loadBlock(req.blockId, req.offset, req.buf, req.length);
    }
    else
    {
        return BlockerSyn::instance().storeBlock(req.blockId, req.offset, req.buf, req.length);
    }
}

void RespHandler::handle_msg(Message const& msg)
{
    auto const* const reqPtr = static_cast<BlockerReq*>(msg.obj());
    runCallbacks(static_cast<BlockerOpType>(msg.what()), *reqPtr);
}

void RespHandler::runCallbacks(BlockerOpType const reqOp, BlockerReq const& req)
{
    auto const blockId = req.blockId;
    auto const result  = req.result;
    auto const jobDone = req.jobDone;
    Blocker::instance().releaseJob(req.jobObj);
    jobDone(reqOp, result, blockId);
}

} /* namespace blocker */
