// Copyright 2024 Accenture.

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/LifecycleControlCommand.h>
#ifdef PLATFORM_SUPPORT_CAN
#include "app/CanDemoListener.h"

#include <can/console/CanCommand.h>
#include <console/AsyncCommandWrapper.h>
#include <systems/ICanSystem.h>
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
#include <storage/IStorage.h>
#include <storage/StorageJob.h>
#include <util/buffer/LinkedBuffer.h>
#endif

namespace systems
{

class DemoSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit DemoSystem(
        ::async::ContextType context,
        ::lifecycle::ILifecycleManager& lifecycleManager
#ifdef PLATFORM_SUPPORT_CAN
        ,
        ::can::ICanSystem& canSystem
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
        ,
        ::storage::IStorage& storage
#endif
    );

    DemoSystem(DemoSystem const&)            = delete;
    DemoSystem& operator=(DemoSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

    void cyclic();

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
#ifdef PLATFORM_SUPPORT_CAN
    ::can::ICanSystem& _canSystem;
    ::can::CanDemoListener _canDemoListener;
    ::can::CanCommand _canCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForCanCommand;
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
    void storageJobDone(::storage::StorageJob&);

    ::storage::IStorage& _storage;
    ::storage::StorageJob _storageJob;
    ::util::buffer::LinkedBuffer<uint8_t> _storageReadBuf;
    ::util::buffer::LinkedBuffer<uint8_t const> _storageWriteBuf;
    ::storage::StorageJob::JobDoneCallback const _jobDoneCallback;

    // BEGIN storage data
    struct StorageData
    {
        uint32_t intParam;
        uint8_t charParam0;
        uint8_t charParam1;
        uint16_t reserved;
    };

    // END storage data

    StorageData _storageData;
#endif
};

} // namespace systems
