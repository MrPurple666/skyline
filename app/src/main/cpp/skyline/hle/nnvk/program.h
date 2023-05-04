// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "types.h"
#include "versioning.h"
#include "memory_pool.h"

namespace nnvk {
    class Device;

    struct ShaderData {
        BufferAddress data;
        const void *control;
    };

    using SubroutineLinkageMapPtr = void *;
    class Program {
      private:
        const char *debugLabel{};
        Device *device{};

      public:
        Program(ApiVersion version, Device *device);

        ~Program();

        void SetDebugLabel(const char *label);

        bool SetShaders(i32 count, const ShaderData *stageData);

        bool SetSubroutineLinkage(i32 i, const SubroutineLinkageMapPtr *ptr);
    };
    NNVK_VERSIONED_STRUCT(Program, 0x30);
}
