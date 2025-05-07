// Copyright 2025 Accenture.

#pragma once

#include "blocker/EepBlockConfBase.h"
#include "blocker/IBlockerDriver.h"

#include <platform/estdint.h>

namespace blocker
{

class EepBlockConf : public EepBlockConfBase
{
public:
    static uint16_t const EepBlock1  = 0U;
    static uint16_t const EepBlock2  = 1U;
    static uint16_t const BLOCKCOUNT = 2U;

    explicit EepBlockConf() : EepBlockConfBase(BLOCKCOUNT) {}

    char const* getName() const override { return "eep"; }

    uint8_t getId() const override { return 10U; }

protected:
    EepBlockPrefs const& getBlockPrefs(uint16_t const blockId) const override
    {
        return fBlockPrefs[blockId];
    }

private:
    static EepBlockPrefs const fBlockPrefs[BLOCKCOUNT];
};

} /* namespace blocker */
