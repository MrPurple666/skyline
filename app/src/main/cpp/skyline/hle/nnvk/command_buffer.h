// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <list>
#include <memory>
#include <variant>
#include "types.h"
#include "versioning.h"
#include "linear_allocator.h"
#include "vkcore.h"
#include "sync.h"



namespace nnvk {
    namespace cmdbuf {
        template<typename Context>
        struct LinkedCommandHeader {
            LinkedCommandHeader *next;
            using ExecuteFunc = void (*)(Context &, LinkedCommandHeader *header);
            ExecuteFunc execute;
        };

        template<typename Context, typename Cmd>
        struct LinkedCommandHolder {
            using CmdType = Cmd;

            LinkedCommandHeader<Context> header{nullptr, Execute};
            Cmd cmd;

            LinkedCommandHolder(Cmd &&cmd) : cmd{cmd} {}

            LinkedCommandHolder() = default;

            static void Execute(Context &context, LinkedCommandHeader<Context> *header) {
                reinterpret_cast<LinkedCommandHolder<Context, Cmd> *>(header)->cmd.Execute(context);
            }
        };

        template<typename Context>
        class LinkedCommands {
          private:
            void Append(LinkedCommandHeader<Context> *cmd) {
                if (last) {
                    last->next = cmd;
                    last = last->next;
                } else {
                    first = cmd;
                    last = first;
                }
            }

          public:
            LinearAllocatorState<> allocator{};
            LinkedCommandHeader<Context> *first{};
            LinkedCommandHeader<Context> *last{};

            void Execute(Context &context) const {
                for (LinkedCommandHeader<Context> *cmd{first}; cmd; cmd = cmd->next)
                    cmd->execute(context, cmd);
            }

            template<typename Cmd>
            void Append(typename Cmd::CmdType &&contents) {
                Cmd *cmd{allocator.template Emplace<Cmd>(std::forward<typename Cmd::CmdType>(contents))};
                Append(reinterpret_cast<LinkedCommandHeader<Context> *>(cmd));
            }
        };

        struct QueueState {

        };

        struct CtrlWaitSyncCommand {
            Sync *sync;
        };

        struct CtrlFenceSyncCommand {
            Sync *sync;
            SyncCondition condition;
            SyncFlags flags;
        };

        struct CtrlVkCommandBufferCommand {
            vk::raii::CommandBuffer commandBuffer;
        };

        using CtrlCommand = std::variant<CtrlWaitSyncCommand, CtrlFenceSyncCommand, CtrlVkCommandBufferCommand>;

        struct VkCommandRecordContext {
            vk::raii::CommandBuffer &commandBuffer;
            vk::raii::CommandBuffer &auxCommandBuffer;

            // FIX
            VkCommandRecordContext(vk::raii::CommandBuffer &commandBuffer, vk::raii::CommandBuffer &auxCommandBuffer) : commandBuffer{commandBuffer}, auxCommandBuffer{auxCommandBuffer} {}
        };

        struct CommandRecordContext {
            vk::raii::CommandPool &commandPool;
            vk::raii::CommandBuffer commandBuffer;
            vk::raii::CommandBuffer auxCommandBuffer;

            VkCommandRecordContext vkRecordContext;
            LinkedCommands<VkCommandRecordContext> pendingVkCommands;
            QueueState stateMirror;

            std::list<CtrlCommand> recordedCommands;

            CommandRecordContext(VkCore &core, vk::raii::CommandPool &commandPool);

            template<typename Cmd>
            void AddPendingCommand(typename Cmd::CmdType &&contents) {
                pendingVkCommands.Append<Cmd>(std::forward<typename Cmd::CmdType>(contents));
            }

            // FIX
        };

        struct CommandList {
            vk::raii::CommandPool commandPool;
            LinkedCommands<CommandRecordContext> rawCommands;
            std::list<CtrlCommand> recordedCommands;

            CommandList(vk::raii::CommandPool &&commandPool, LinkedCommands<CommandRecordContext> &&rawCommands, std::list<CtrlCommand> &&recordedCommands);
        };
    }

    class Sync;
    class Device;

    class DepthStencilState {};

    class CommandBuffer {
      private:
        friend class Queue;

        const char *debugLabel{};
        Device *device;
        void *memoryCallback{};
        void *memoryCallbackData{};
        cmdbuf::QueueState queueState;

        struct RecordState {
            vk::raii::CommandPool commandPool;
            cmdbuf::CommandRecordContext recordContext;
            cmdbuf::LinkedCommands<cmdbuf::CommandRecordContext> commands{};

            RecordState(VkCore &core);

            template<typename Cmd>
            void AppendAndExecute(typename Cmd::CmdType &&contents) {
                commands.Append<Cmd>(std::forward<typename Cmd::CmdType>(contents));
                Cmd::Execute(recordContext, commands.last);
            }
        };

        std::unique_ptr<RecordState> recordState{};

      public:
        CommandBuffer(ApiVersion version, Device *device);

        ~CommandBuffer();

        void SetDebugLabel(const char *label);

        void SetMemoryCallback(void *callback);

        void SetMemoryCallbackData(void *callbackData);

        void AddCommandMemory(const MemoryPool *pool, i64 offset, u64 size);

        void AddControlMemory(void *memory, u64 size);

        u64 GetCommandMemorySize() const;

        u64 GetCommandMemoryUsed() const;

        u64 GetCommandMemoryFree() const;

        u64 GetControlMemorySize() const;

        u64 GetControlMemoryUsed() const;

        u64 GetControlMemoryFree() const;

        void BeginRecording();

        CommandHandle EndRecording();

        void *GetMemoryCallback() const;

        void *GetMemoryCallbackData() const;
    };
    NNVK_VERSIONED_STRUCT(CommandBuffer, 0xA0);

}
