// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "types.h"
#include "versioning.h"
#include "vkcore.h"

namespace nnvk {
    class Device;

    struct SyncFlags {
        bool flushForCpu : 1;
        u32 _pad_ : 31;
    };


    enum class SyncWaitResult : i32 {
        AlreadySignalled,
        ConditionSatisfied,
        TimeoutExpired,
        Failed
    };

    enum class SyncCondition : i32 {
        AllGpuCommandsComplete,
        GraphicsWorldSpaceComplete,
    };

    /**
     * Reqs:
     * Signal recorded -> present submit -> signal cmdbuf submitted:
     * Need to have a way to wait for submission on CPU due to present not supporting timeline semaphores.
     * Hence create one binary semaphore per signal operation, together with a CV that is signalled on submission. Then wait on CV before queue present. Semaphore ownership needs to be transferred
     * Wait recorded -> wait submitted -> signal recorded -> signal submitted
     * Handled by timeline semaphores by default
     *
     * Misc:
     * Cmdbufs will need to be split up based on sync boundaries - a single guest cmdbuf may need to spluit into multiple host cmdbufs and submissions
     */
    class Sync {
      private:
        friend class Queue;
        friend class Window;

        const char *debugLabel{};
        Device *device;

        u64 timelineValue{};

        bool Waiting() const;

        void Fence(u64 pTimelineValue);

      public:
        Sync(ApiVersion version, Device *device);

        ~Sync();

        void SetDebugLabel(const char *label);

        SyncWaitResult Wait(u64 timeout) const;
    };
    NNVK_VERSIONED_STRUCT(Sync, 0x40);
}
