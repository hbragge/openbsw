// Copyright 2025 Accenture.

#pragma once

#include <estd/functional.h>
#include <platform/estdint.h>

namespace blocker
{

enum BlockerOpType
{
    BLOCKER_LOAD,
    BLOCKER_STORE,
};

enum BlockerResult
{
    BLOCKER_OK,
    BLOCKER_FAILED,
};

typedef ::estd::function<void(BlockerOpType const, BlockerResult const, uint32_t const)>
    JobDoneCallback;

} /* namespace blocker */
