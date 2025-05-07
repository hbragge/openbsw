// Copyright 2025 Accenture.

#include "blocker/EepBlockerDriver.h"

#include <bsp/eeprom/IEepromDriver.h>

namespace blocker
{
EepBlockerDriver::EepBlockerDriver(eeprom::IEepromDriver& driver, EepBlockConfBase const& config)
: fDriver(driver), fConfig(config)
{}

bool EepBlockerDriver::init(bool const /*wakeUp*/) { return (fDriver.init() == bsp::BSP_OK); }

bool EepBlockerDriver::write(
    uint16_t const blockId, size_t const offset, estd::slice<uint8_t const> const& src)
{
    auto const length = src.size();
    if (!isInputValid(blockId, offset, length))
    {
        return false;
    }

    auto const address = fConfig.getAddress(blockId);
    if (fDriver.write(
            address + static_cast<uint32_t>(offset), src.data(), static_cast<uint32_t>(length))
        == bsp::BSP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool EepBlockerDriver::read(
    uint16_t const blockId, size_t const offset, estd::slice<uint8_t>& dst) const
{
    auto const length = dst.size();
    if (!isInputValid(blockId, offset, length))
    {
        return false;
    }

    auto const address = fConfig.getAddress(blockId);
    if (fDriver.read(
            address + static_cast<uint32_t>(offset), dst.data(), static_cast<uint32_t>(length))
        == bsp::BSP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool EepBlockerDriver::isInputValid(
    uint16_t const blockId, size_t const offset, size_t const length) const
{
    auto const total = static_cast<size_t>(offset + length);
    if (total < offset)
    {
        return false;
    }
    if ((length == 0U) || (!fConfig.isValidId(blockId))
        || ((offset + length) > fConfig.getBlockSize(blockId)))
    {
        return false;
    }
    else
    {
        return true;
    }
}

} /* namespace blocker */
