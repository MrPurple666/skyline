// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <range/v3/view.hpp>
#include <loader/loader.h>
#include <gpu.h>
#include "command_executor.h"

namespace skyline::gpu::interconnect {
    CommandRecordThread::CommandRecordThread(const DeviceState &state) : state{state}, thread{&CommandRecordThread::Run, this} {}

    static vk::raii::CommandBuffer AllocateRaiiCommandBuffer(GPU &gpu, vk::raii::CommandPool &pool) {
        return {gpu.vkDevice, (*gpu.vkDevice).allocateCommandBuffers(
                    {
                        .commandPool = *pool,
                        .level = vk::CommandBufferLevel::ePrimary,
                        .commandBufferCount = 1
                    }, *gpu.vkDevice.getDispatcher()).front(),
                *pool};
    }

    CommandRecordThread::Slot::Slot(GPU &gpu)
        : commandPool{gpu.vkDevice,
                      vk::CommandPoolCreateInfo{
                          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient,
                          .queueFamilyIndex = gpu.vkQueueFamilyIndex
                      }
          },
          commandBuffer{AllocateRaiiCommandBuffer(gpu, commandPool)},
          fence{gpu.vkDevice, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }},
          cycle{std::make_shared<FenceCycle>(gpu.vkDevice, *fence, true)} {}

    std::shared_ptr<FenceCycle> CommandRecordThread::Slot::Reset(GPU &gpu) {
        cycle->Wait();
        cycle = std::make_shared<FenceCycle>(gpu.vkDevice, *fence);
        // Command buffer doesn't need to be reset since that's done implicitly by begin
        return cycle;
    }

    void CommandRecordThread::ProcessSlot(Slot *slot) {
        TRACE_EVENT_FMT("gpu", "ProcessSlot: 0x{:X}, execution: {}", slot, slot->executionNumber);
        auto &gpu{*state.gpu};

        vk::RenderPass lRenderPass;
        u32 subpassIndex;

        std::scoped_lock bufferLock{gpu.buffer.recreationMutex};
        using namespace node;
        for (NodeVariant &node : slot->nodes) {
            #define NODE(name) [&](name& node) { node(slot->commandBuffer, slot->cycle, gpu); }
            std::visit(VariantVisitor{
                NODE(FunctionNode),

                [&](RenderPassNode &node) {
                    lRenderPass = node(slot->commandBuffer, slot->cycle, gpu);
                    subpassIndex = 0;
                },

                [&](NextSubpassNode &node) {
                    node(slot->commandBuffer, slot->cycle, gpu);
                    ++subpassIndex;
                },
                [&](SubpassFunctionNode &node) { node(slot->commandBuffer, slot->cycle, gpu, lRenderPass, subpassIndex); },
                [&](NextSubpassFunctionNode &node) { node(slot->commandBuffer, slot->cycle, gpu, lRenderPass, ++subpassIndex); },

                NODE(RenderPassEndNode),
            }, node);
            #undef NODE
        }

        slot->commandBuffer.end();

        gpu.scheduler.SubmitCommandBuffer(slot->commandBuffer, slot->cycle);

        slot->nodes.clear();
        slot->allocator.Reset();
    }

    void CommandRecordThread::Run() {
        auto &gpu{*state.gpu};
        std::array<Slot, ActiveRecordSlots> slots{{gpu, gpu, gpu, gpu, gpu, gpu}};
        outgoing.AppendTranform(span<Slot>(slots), [](auto &slot) { return &slot; });

        if (int result{pthread_setname_np(pthread_self(), "Sky-CmdRecord")})
            Logger::Warn("Failed to set the thread name: {}", strerror(result));

        try {
            signal::SetSignalHandler({SIGINT, SIGILL, SIGTRAP, SIGBUS, SIGFPE, SIGSEGV}, signal::ExceptionalSignalHandler);

            incoming.Process([this](Slot *slot) {
                ProcessSlot(slot);
                outgoing.Push(slot);
            }, [] {});
        } catch (const signal::SignalException &e) {
            Logger::Error("{}\nStack Trace:{}", e.what(), state.loader->GetStackTrace(e.frames));
            if (state.process)
                state.process->Kill(false);
            else
                std::rethrow_exception(std::current_exception());
        } catch (const std::exception &e) {
            Logger::Error(e.what());
            if (state.process)
                state.process->Kill(false);
            else
                std::rethrow_exception(std::current_exception());
        }
    }

    CommandRecordThread::Slot *CommandRecordThread::AcquireSlot() {
        return outgoing.Pop();
    }

    void CommandRecordThread::ReleaseSlot(Slot *slot) {
        incoming.Push(slot);
    }

    CommandExecutor::CommandExecutor(const DeviceState &state)
        : gpu{*state.gpu},
          recordThread{state},
          tag{AllocateTag()} {
        RotateRecordSlot();
    }

    CommandExecutor::~CommandExecutor() {
        cycle->Cancel();
    }

    void CommandExecutor::RotateRecordSlot() {
        if (slot)
            recordThread.ReleaseSlot(slot);

        slot = recordThread.AcquireSlot();
        cycle = slot->Reset(gpu);
        slot->executionNumber = executionNumber;
        allocator = &slot->allocator;
    }

    TextureManager &CommandExecutor::AcquireTextureManager() {
        if (!textureManagerLock)
            textureManagerLock.emplace(gpu.texture);
        return gpu.texture;
    }

    BufferManager &CommandExecutor::AcquireBufferManager() {
        if (!bufferManagerLock)
            bufferManagerLock.emplace(gpu.buffer);
        return gpu.buffer;
    }

    MegaBufferAllocator &CommandExecutor::AcquireMegaBufferAllocator() {
        if (!megaBufferAllocatorLock)
            megaBufferAllocatorLock.emplace(gpu.megaBufferAllocator);
        return gpu.megaBufferAllocator;
    }

    bool CommandExecutor::CreateRenderPassWithSubpass(vk::Rect2D renderArea, span<TextureView *> inputAttachments, span<TextureView *> colorAttachments, TextureView *depthStencilAttachment, bool noSubpassCreation) {
        auto addSubpass{[&] {
            renderPass->AddSubpass(inputAttachments, colorAttachments, depthStencilAttachment, gpu);

            lastSubpassAttachments.clear();
            auto insertAttachmentRange{[this](auto &attachments) -> std::pair<size_t, size_t> {
                size_t beginIndex{lastSubpassAttachments.size()};
                lastSubpassAttachments.insert(lastSubpassAttachments.end(), attachments.begin(), attachments.end());
                return {beginIndex, attachments.size()};
            }};

            auto rangeToSpan{[this](auto &range) -> span<TextureView *> {
                return {lastSubpassAttachments.data() + range.first, range.second};
            }};

            auto inputAttachmentRange{insertAttachmentRange(inputAttachments)};
            auto colorAttachmentRange{insertAttachmentRange(colorAttachments)};

            lastSubpassInputAttachments = rangeToSpan(inputAttachmentRange);
            lastSubpassColorAttachments = rangeToSpan(colorAttachmentRange);
            lastSubpassDepthStencilAttachment = depthStencilAttachment;
        }};

        bool attachmentsMatch{ranges::equal(lastSubpassInputAttachments, inputAttachments) &&
                              ranges::equal(lastSubpassColorAttachments, colorAttachments) &&
                              lastSubpassDepthStencilAttachment == depthStencilAttachment};

        if (renderPass == nullptr || renderPass->renderArea != renderArea ||
            ((noSubpassCreation || subpassCount >= gpu.traits.quirks.maxSubpassCount) && !attachmentsMatch)) {
            // We need to create a render pass if one doesn't already exist or the current one isn't compatible
            if (renderPass != nullptr)
                slot->nodes.emplace_back(std::in_place_type_t<node::RenderPassEndNode>());
            renderPass = &std::get<node::RenderPassNode>(slot->nodes.emplace_back(std::in_place_type_t<node::RenderPassNode>(), renderArea));
            addSubpass();
            subpassCount = 1;
            return false;
        } else {
            if (attachmentsMatch) {
                // The last subpass had the same attachments, so we can reuse them
                return false;
            } else {
                // The last subpass had different attachments, so we need to create a new one
                addSubpass();
                subpassCount++;
                return true;
            }
        }
    }

    void CommandExecutor::FinishRenderPass() {
        if (renderPass) {
            slot->nodes.emplace_back(std::in_place_type_t<node::RenderPassEndNode>());

            renderPass = nullptr;
            subpassCount = 0;

            lastSubpassAttachments.clear();
            lastSubpassInputAttachments = nullptr;
            lastSubpassColorAttachments = nullptr;
            lastSubpassDepthStencilAttachment = nullptr;
        }
    }

    CommandExecutor::LockedTexture::LockedTexture(std::shared_ptr<Texture> texture) : texture{std::move(texture)} {}

    constexpr CommandExecutor::LockedTexture::LockedTexture(CommandExecutor::LockedTexture &&other) : texture{std::exchange(other.texture, nullptr)} {}

    constexpr Texture *CommandExecutor::LockedTexture::operator->() const {
        return texture.get();
    }

    CommandExecutor::LockedTexture::~LockedTexture() {
        if (texture)
            texture->unlock();
    }

    bool CommandExecutor::AttachTexture(TextureView *view) {
        if (!textureManagerLock)
            // Avoids a potential deadlock with this resource being locked while acquiring the TextureManager lock while the thread owning it tries to acquire a lock on this texture
            textureManagerLock.emplace(gpu.texture);

        bool didLock{view->LockWithTag(tag)};
        if (didLock) {
            if (view->texture->FrequentlyLocked())
                attachedTextures.emplace_back(view->texture);
            else
                preserveAttachedTextures.emplace_back(view->texture);
        }

        return didLock;
    }

    CommandExecutor::LockedBuffer::LockedBuffer(std::shared_ptr<Buffer> buffer) : buffer{std::move(buffer)} {}

    constexpr CommandExecutor::LockedBuffer::LockedBuffer(CommandExecutor::LockedBuffer &&other) : buffer{std::exchange(other.buffer, nullptr)} {}

    constexpr Buffer *CommandExecutor::LockedBuffer::operator->() const {
        return buffer.get();
    }

    CommandExecutor::LockedBuffer::~LockedBuffer() {
        if (buffer)
            buffer->unlock();
    }

    bool CommandExecutor::AttachBuffer(BufferView &view) {
        if (!bufferManagerLock)
            // See AttachTexture(...)
            bufferManagerLock.emplace(gpu.buffer);

        bool didLock{view.LockWithTag(tag)};
        if (didLock) {
            if (view.GetBuffer()->FrequentlyLocked())
                attachedBuffers.emplace_back(view.GetBuffer()->shared_from_this());
            else
                preserveAttachedBuffers.emplace_back(view.GetBuffer()->shared_from_this());
        }
        return didLock;
    }

    void CommandExecutor::AttachLockedBufferView(BufferView &view, ContextLock<BufferView> &&lock) {
        if (!bufferManagerLock)
            // See AttachTexture(...)
            bufferManagerLock.emplace(gpu.buffer);

        if (lock.OwnsLock()) {
            // Transfer ownership to executor so that the resource will stay locked for the period it is used on the GPU
            if (view.GetBuffer()->FrequentlyLocked())
                attachedBuffers.emplace_back(view.GetBuffer()->shared_from_this());
            else
                preserveAttachedBuffers.emplace_back(view.GetBuffer()->shared_from_this());
            lock.Release(); // The executor will handle unlocking the lock so it doesn't need to be handled here
        }
    }

    void CommandExecutor::AttachLockedBuffer(std::shared_ptr<Buffer> buffer, ContextLock<Buffer> &&lock) {
        if (lock.OwnsLock()) {
            if (buffer->FrequentlyLocked())
                attachedBuffers.emplace_back(std::move(buffer));
            else
                preserveAttachedBuffers.emplace_back(std::move(buffer));
            lock.Release(); // See AttachLockedBufferView(...)
        }
    }

    void CommandExecutor::AttachDependency(const std::shared_ptr<void> &dependency) {
        cycle->AttachObject(dependency);
    }

    void CommandExecutor::AddSubpass(std::function<void(vk::raii::CommandBuffer &, const std::shared_ptr<FenceCycle> &, GPU &, vk::RenderPass, u32)> &&function, vk::Rect2D renderArea, span<TextureView *> inputAttachments, span<TextureView *> colorAttachments, TextureView *depthStencilAttachment, bool noSubpassCreation) {
        bool gotoNext{CreateRenderPassWithSubpass(renderArea, inputAttachments, colorAttachments, depthStencilAttachment ? &*depthStencilAttachment : nullptr, noSubpassCreation)};
        if (gotoNext)
            slot->nodes.emplace_back(std::in_place_type_t<node::NextSubpassFunctionNode>(), std::forward<decltype(function)>(function));
        else
            slot->nodes.emplace_back(std::in_place_type_t<node::SubpassFunctionNode>(), std::forward<decltype(function)>(function));
    }

    void CommandExecutor::AddOutsideRpCommand(std::function<void(vk::raii::CommandBuffer &, const std::shared_ptr<FenceCycle> &, GPU &)> &&function) {
        if (renderPass)
            FinishRenderPass();

        slot->nodes.emplace_back(std::in_place_type_t<node::FunctionNode>(), std::forward<decltype(function)>(function));
    }

    void CommandExecutor::AddClearColorSubpass(TextureView *attachment, const vk::ClearColorValue &value) {
        bool gotoNext{CreateRenderPassWithSubpass(vk::Rect2D{.extent = attachment->texture->dimensions}, {}, attachment, nullptr)};
        if (renderPass->ClearColorAttachment(0, value, gpu)) {
            if (gotoNext)
                slot->nodes.emplace_back(std::in_place_type_t<node::NextSubpassNode>());
        } else {
            auto function{[scissor = attachment->texture->dimensions, value](vk::raii::CommandBuffer &commandBuffer, const std::shared_ptr<FenceCycle> &, GPU &, vk::RenderPass, u32) {
                commandBuffer.clearAttachments(vk::ClearAttachment{
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .colorAttachment = 0,
                    .clearValue = value,
                }, vk::ClearRect{
                    .rect = vk::Rect2D{.extent = scissor},
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                });
            }};

            if (gotoNext)
                slot->nodes.emplace_back(std::in_place_type_t<node::NextSubpassFunctionNode>(), function);
            else
                slot->nodes.emplace_back(std::in_place_type_t<node::SubpassFunctionNode>(), function);
        }
    }

    void CommandExecutor::AddClearDepthStencilSubpass(TextureView *attachment, const vk::ClearDepthStencilValue &value) {
        bool gotoNext{CreateRenderPassWithSubpass(vk::Rect2D{.extent = attachment->texture->dimensions}, {}, {}, attachment)};
        if (renderPass->ClearDepthStencilAttachment(value, gpu)) {
            if (gotoNext)
                slot->nodes.emplace_back(std::in_place_type_t<node::NextSubpassNode>());
        } else {
            auto function{[aspect = attachment->format->vkAspect, extent = attachment->texture->dimensions, value](vk::raii::CommandBuffer &commandBuffer, const std::shared_ptr<FenceCycle> &, GPU &, vk::RenderPass, u32) {
                commandBuffer.clearAttachments(vk::ClearAttachment{
                    .aspectMask = aspect,
                    .clearValue = value,
                }, vk::ClearRect{
                    .rect.extent = extent,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                });
            }};

            if (gotoNext)
                slot->nodes.emplace_back(std::in_place_type_t<node::NextSubpassFunctionNode>(), function);
            else
                slot->nodes.emplace_back(std::in_place_type_t<node::SubpassFunctionNode>(), function);
        }
    }

    void CommandExecutor::AddFlushCallback(std::function<void()> &&callback) {
        flushCallbacks.emplace_back(std::forward<decltype(callback)>(callback));
    }

    void CommandExecutor::AddPipelineChangeCallback(std::function<void()> &&callback) {
        pipelineChangeCallbacks.emplace_back(std::forward<decltype(callback)>(callback));
    }

    void CommandExecutor::NotifyPipelineChange() {
        for (auto &callback : pipelineChangeCallbacks)
            callback();
    }

    void CommandExecutor::SubmitInternal() {
        if (renderPass)
            FinishRenderPass();

        {
            slot->commandBuffer.begin(vk::CommandBufferBeginInfo{
                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
            });

            // We need this barrier here to ensure that resources are in the state we expect them to be in, we shouldn't overwrite resources while prior commands might still be using them or read from them while they might be modified by prior commands
            slot->commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, vk::MemoryBarrier{
                    .srcAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
                    .dstAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
                }, {}, {}
            );

            boost::container::small_vector<FenceCycle *, 8> chainedCycles;
            for (const auto &texture : ranges::views::concat(attachedTextures, preserveAttachedTextures)) {
                texture->SynchronizeHostInline(slot->commandBuffer, cycle, true);
                // We don't need to attach the Texture to the cycle as a TextureView will already be attached
                if (ranges::find(chainedCycles, texture->cycle.get()) == chainedCycles.end()) {
                    cycle->ChainCycle(texture->cycle);
                    chainedCycles.emplace_back(texture->cycle.get());
                }

                texture->cycle = cycle;
            }
        }

        for (const auto &attachedBuffer : ranges::views::concat(attachedBuffers, preserveAttachedBuffers)) {
            if (attachedBuffer->RequiresCycleAttach()) {
                attachedBuffer->SynchronizeHost(); // Synchronize attached buffers from the CPU without using a staging buffer
                cycle->AttachObject(attachedBuffer.buffer);
                attachedBuffer->UpdateCycle(cycle);
                attachedBuffer->AllowAllBackingWrites();
            }
        }

        RotateRecordSlot();
    }

    void CommandExecutor::ResetInternal() {
        attachedTextures.clear();
        textureManagerLock.reset();
        attachedBuffers.clear();
        bufferManagerLock.reset();
        megaBufferAllocatorLock.reset();
        allocator->Reset();

        // Periodically clear preserve attachments just in case there are new waiters which would otherwise end up waiting forever
        if ((submissionNumber % CommandRecordThread::ActiveRecordSlots * 2) == 0) {
            preserveAttachedBuffers.clear();
            preserveAttachedTextures.clear();
        }
    }

    void CommandExecutor::Submit() {
        for (const auto &callback : flushCallbacks)
            callback();

        executionNumber++;

        if (!slot->nodes.empty()) {
            TRACE_EVENT("gpu", "CommandExecutor::Submit");
            SubmitInternal();
            submissionNumber++;
        }

        ResetInternal();
    }

    void CommandExecutor::LockPreserve() {
        if (!preserveLocked) {
            preserveLocked = true;

            for (auto &buffer : preserveAttachedBuffers)
                buffer->LockWithTag(tag);

            for (auto &texture : preserveAttachedTextures)
                texture->LockWithTag(tag);
        }
    }

    void CommandExecutor::UnlockPreserve() {
        if (preserveLocked) {
            for (auto &buffer : preserveAttachedBuffers)
                buffer->unlock();

            for (auto &texture : preserveAttachedTextures)
                texture->unlock();

            preserveLocked = false;
        }
    }
}
