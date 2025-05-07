// Copyright 2025 Accenture.

#pragma once

#include "blocker/EepBlockConfBase.h"
#include "blocker/IBlockerDriver.h"

namespace eeprom
{
class IEepromDriver;
}

namespace blocker
{
class EepBlockerDriver : public IBlockerDriver
{
public:
    explicit EepBlockerDriver(eeprom::IEepromDriver& driver, EepBlockConfBase const& config);

    char const* getName() const override { return fConfig.getName(); }

    uint8_t getId() const override { return fConfig.getId(); }

    bool init(bool wakeUp = false) override;
    bool write(uint16_t blockId, size_t offset, estd::slice<uint8_t const> const& src) override;
    bool read(uint16_t blockId, size_t offset, estd::slice<uint8_t>& dst) const override;

private:
    bool isInputValid(uint16_t blockId, size_t offset, size_t length) const;

    eeprom::IEepromDriver& fDriver;
    EepBlockConfBase const& fConfig;
};

} /* namespace blocker */
