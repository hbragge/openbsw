// Copyright 2025 Accenture.

#pragma once

#include "blocker/BlockerTypes.h"
#include "blocker/Handler.h"

namespace blocker
{
struct BlockerJob;

struct BlockerReq
{
    JobDoneCallback jobDone;
    Handler* respHandler;
    BlockerJob* jobObj;
    uint8_t* buf;
    size_t offset;
    size_t length;
    BlockerResult result;
    uint32_t blockId;
};

class BlockerHan : public Handler
{
public:
    BlockerHan(MsgPasser& msgPasser);
    void handle_msg(Message const& msg) override;

private:
    BlockerResult handleRequest(BlockerOpType reqOp, BlockerReq const& req) const;
};

class RespHandler : public Handler
{
public:
    explicit RespHandler(MsgPasser& msgPasser) : Handler(msgPasser) {}

    void handle_msg(Message const& msg) override;
    static void runCallbacks(BlockerOpType reqOp, BlockerReq const& req);
};

} /* namespace blocker */
