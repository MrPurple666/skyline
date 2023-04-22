// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <span>
#include "types.h"
#include "versioning.h"
#include "device.h"

namespace nnvk {
    struct QueueFlags {
        bool noFragmentInterlock : 1;
        bool noZCull : 1;
        u32 _pad_ : 30;
    };
    static_assert(sizeof(QueueFlags) == 4);

    class Queue;

    class QueueBuilder {
      private:
        friend Queue;

        Device *device{};
        QueueFlags flags{};
        u64 commandMemorySize{};
        u64 computeMemorySize{};
        u64 controlMemorySize{};
        std::span<u8> queueMemory;
        u64 commandFlushThreshold{256};

      public:
        QueueBuilder(ApiVersion version);

        void SetDevice(Device *device);

        void SetFlags(QueueFlags newFlags);

        void SetCommandMemorySize(u64 size);

        void SetComputeMemorySize(u64 size);

        void SetControlMemorySize(u64 size);

        u64 GetQueueMemorySize() const;

        void SetQueueMemory(void *memory, u64 size);

        void SetCommandFlushThreshold(u64 threshold);

        const Device *GetDevice() const;

        QueueFlags GetFlags() const;

        bool GetCommandMemorySize(i32 *i) const;

        bool GetComputeMemorySize(i32 *i) const;

        bool GetControlMemorySize(i32 *i) const;

        bool GetCommandFlushThreshold(i32 *i) const;

        int GetMemorySize() const;

        void *GetMemory() const;
    };
    NNVK_VERSIONED_STRUCT(QueueBuilder, 0x40);

    enum class QueueGetErrorResult : i32 {
        GpuNoError,
        GpuErrorUnknown,
        GpuErrorMmuFault,
        GpuErrorPbDmaException,
        GpuErrorEngineException,
        GpuErrorTimeout,
    };

    union QueueErrorInfo;

    class Window;

    class Sync;

    enum class SyncCondition : i32 {
        AllGpuCommandsComplete,
        GraphicsWorldSpaceComplete,
    };

    enum class SyncWaitResult : i32 {
        AlreadySignalled,
        ConditionSatisfied,
        TimeoutExpired,
        Failed
    };

    enum class QueueAcquireTextureResult : i32 {
        Success,
        NativeError
    };

    struct SyncFlags {
        bool flushForCpu : 1;
        u32 _pad_ : 31;
    };

    class Queue {
      private:
        const char *debugLabel{};
        Device *device;

      public:
        Queue(ApiVersion version, const QueueBuilder &builder);

        ~Queue();

        QueueGetErrorResult GetError(QueueErrorInfo *errorInfo);

        u64 GetTotalCommandMemoryUsed();

        u64 GetTotalControlMemoryUsed();

        u64 GetTotalComputeMemoryUsed();

        void ResetMemoryUsageCounts();

        void SetDebugLabel(const char *label);

        void SubmitCommands(i32 numCommands, const CommandHandle *handles);

        void Flush();

        void Finish();

        void PresentTexture(Window *window, i32 *textureIndex);

        void FenceSync(Sync *sync, SyncCondition condition, SyncFlags flags);

        bool WaitSync(Sync *sync);
    };
    NNVK_VERSIONED_STRUCT(Queue, 0x2000);
}
