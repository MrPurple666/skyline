#include <new>
#include "nnvk.h"
#include "types.h"
#include "versioning.h"
#include "logging.h"
#include "sync.h"

namespace nnvk {
    bool Sync::Waiting() const {
        return timelineValue != 0;
    }

    void Sync::Fence(u64 pTimelineValue) {
        timelineValue = pTimelineValue;
    }

    Sync::Sync(ApiVersion version, Device *device)
        : device{device} {
        NNVK_FILL_VERSIONED_STRUCT(Sync);
    }

    Sync::~Sync() {}

    void Sync::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    SyncWaitResult Sync::Wait(u64 timeout) const {
        switch (device->vkCore.scheduler.WaitTimeline(timelineValue, timeout)) {
            case vkcore::CommandScheduler::WaitResult::AlreadySignaled:
                return SyncWaitResult::AlreadySignalled;
            case vkcore::CommandScheduler::WaitResult::WaitSignalled:
                return SyncWaitResult::ConditionSatisfied;
            case vkcore::CommandScheduler::WaitResult::WaitTimeout:
                return SyncWaitResult::TimeoutExpired;
        }


    /*
        */
    }


    /* Wrappers */
    bool Context::SyncInitialize(Sync *sync, Device *device) {
        new (sync) Sync(apiVersion, device);
        return true;
    }

    void Context::SyncFinalize(Sync *pool) {
        pool->~Sync();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Sync, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(SyncWaitResult, Sync, Wait, u64)
    /* End wrappers */
}