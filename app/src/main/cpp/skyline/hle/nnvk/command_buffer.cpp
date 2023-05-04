#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <variant>
#include "types.h"
#include "versioning.h"
#include "vkcore.h"
#include "nnvk.h"
#include "device.h"
#include "command_buffer.h"

namespace nnvk {
    namespace cmdbuf {
        static vk::raii::CommandBuffer AllocateCommandBuffer(vk::raii::Device &device, vk::raii::CommandPool &commandPool) {
            return std::move(device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{ .commandBufferCount = 1, .commandPool = *commandPool, .level = vk::CommandBufferLevel::ePrimary })[0]);
        }

        CommandRecordContext::CommandRecordContext(VkCore &vkCore, vk::raii::CommandPool &commandPool)
            : commandPool{commandPool},
              commandBuffer{AllocateCommandBuffer(vkCore.device, commandPool)},
              auxCommandBuffer{AllocateCommandBuffer(vkCore.device, commandPool)},
              vkRecordContext{commandBuffer, auxCommandBuffer} {}


          CommandList::CommandList(vk::raii::CommandPool &&commandPool, LinkedCommands<CommandRecordContext> &&rawCommands, std::list<CtrlCommand> &&recordedCommands)
              : commandPool{std::move(commandPool)},
                rawCommands{std::move(rawCommands)},
                recordedCommands{std::move(recordedCommands)} {}

    }
    namespace vkcmds {
        struct SetDepthStencilStateImpl {
            void Execute(cmdbuf::VkCommandRecordContext &context) {
                context.commandBuffer.setDepthCompareOpEXT(depthCompareOp);
                if (commonStencil) {
                    context.commandBuffer.setStencilOpEXT(vk::StencilFaceFlagBits::eFrontAndBack, front.failOp, front.passOp, front.depthFailOp, front.compareOp);
                } else {
                    if (frontStencil)
                        context.commandBuffer.setStencilOpEXT(vk::StencilFaceFlagBits::eFront, front.failOp, front.passOp, front.depthFailOp, front.compareOp);

                    if (backStencil)
                        context.commandBuffer.setStencilOpEXT(vk::StencilFaceFlagBits::eBack, back.failOp, back.passOp, back.depthFailOp, back.compareOp);
                }

                context.commandBuffer.setDepthTestEnableEXT(depthTestEnable);
                context.commandBuffer.setDepthWriteEnableEXT(depthWriteEnable);
                context.commandBuffer.setStencilTestEnableEXT(stencilTestEnable);
            }

            vk::CompareOp depthCompareOp;
            struct Face {
                vk::StencilOp failOp;
                vk::StencilOp passOp;
                vk::StencilOp depthFailOp;
                vk::CompareOp compareOp;
            } front, back;
            struct {
                bool depthTestEnable : 1;
                bool depthWriteEnable : 1;
                bool stencilTestEnable : 1;
                bool commonStencil : 1;
                bool frontStencil : 1;
                bool backStencil : 1;
            };

        };
        using SetDepthStencilState = cmdbuf::LinkedCommandHolder<cmdbuf::VkCommandRecordContext, SetDepthStencilStateImpl>;
    }

    namespace cmds {
        struct SetDepthStencilStateImpl {
            void Execute(cmdbuf::CommandRecordContext &context) {
                context.AddPendingCommand<vkcmds::SetDepthStencilState>({
                });
            }

            DepthStencilState state;
        };

        using SetDepthStencilState = cmdbuf::LinkedCommandHolder<cmdbuf::CommandRecordContext, SetDepthStencilStateImpl>;
    }

    CommandBuffer::RecordState::RecordState(VkCore &core)
        : commandPool{core.device.createCommandPool(vk::CommandPoolCreateInfo{ .flags = vk::CommandPoolCreateFlagBits::eTransient })},
          recordContext{core, commandPool} {}

    CommandBuffer::CommandBuffer(ApiVersion version, Device *device) : device{device} {
        NNVK_FILL_VERSIONED_STRUCT(CommandBuffer);
    }

    CommandBuffer::~CommandBuffer() = default;

    void CommandBuffer::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void CommandBuffer::SetMemoryCallback(void *callback) {
        memoryCallback = callback;
    }

    void CommandBuffer::SetMemoryCallbackData(void *callbackData) {
        memoryCallbackData = callbackData;
    }

    void CommandBuffer::AddCommandMemory(const MemoryPool *pool, i64 offset, u64 size) {}

    void CommandBuffer::AddControlMemory(void *memory, u64 size) {}

    u64 CommandBuffer::GetCommandMemorySize() const {
        return PAGE_SIZE;
    }

    u64 CommandBuffer::GetCommandMemoryUsed() const {
        return 0;
    }

    u64 CommandBuffer::GetCommandMemoryFree() const {
        return PAGE_SIZE;
    }

    u64 CommandBuffer::GetControlMemorySize() const {
        return PAGE_SIZE;
    }

    u64 CommandBuffer::GetControlMemoryUsed() const {
        return 0;
    }

    u64 CommandBuffer::GetControlMemoryFree() const {
        return PAGE_SIZE;
    }


    void CommandBuffer::BeginRecording() {
        recordState = std::make_unique<RecordState>(device->vkCore);
    }

    //void CommandBuffer::BindDepthStencilState(const DepthStencilState *state) {
        //recordState->AppendAndExecute<cmds::SetDepthStencilState>({.state = *state});

    //}

    CommandHandle CommandBuffer::EndRecording() {
        cmdbuf::CommandList *list{new cmdbuf::CommandList(std::move(recordState->commandPool), std::move(recordState->commands), std::move(recordState->recordContext.recordedCommands))};
        return CommandHandle{list};
    }

    void *CommandBuffer::GetMemoryCallback() const {
        return memoryCallback;
    }

    void *CommandBuffer::GetMemoryCallbackData() const {
        return memoryCallbackData;
    }

    /* Wrappers */
    bool Context::CommandBufferInitialize(nnvk::CommandBuffer *commandBuffer, nnvk::Device *device) {
        new (commandBuffer) CommandBuffer(apiVersion, device);
        return true;
    }

    void Context::CommandBufferFinalize(nnvk::CommandBuffer *commandBuffer) {
        commandBuffer->~CommandBuffer();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, CommandBuffer, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, CommandBuffer, SetMemoryCallback, void *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, CommandBuffer, SetMemoryCallbackData, void *)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, CommandBuffer, AddCommandMemory, const MemoryPool *, i64, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, CommandBuffer, AddControlMemory, void *, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, CommandBuffer, GetCommandMemorySize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, CommandBuffer, GetCommandMemoryUsed)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, CommandBuffer, GetCommandMemoryFree)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, CommandBuffer, GetControlMemorySize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, CommandBuffer, GetControlMemoryUsed)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, CommandBuffer, GetControlMemoryFree)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void, CommandBuffer, BeginRecording)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(CommandHandle, CommandBuffer, EndRecording)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void *, CommandBuffer, GetMemoryCallback)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void *, CommandBuffer, GetMemoryCallbackData)
    /* End wrappers */
}