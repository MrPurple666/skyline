#include <new>
#include <stdexcept>
#include <fmt/format.h>
#include "logging.h"
#include "memory_manager.h"
#include "nnvk.h"
#include "types.h"
#include "queue.h"

namespace nnvk {
    QueueBuilder::QueueBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(QueueBuilder);
    }

    void QueueBuilder::SetDevice(Device *pDevice) {
        device = pDevice;
    }

    void QueueBuilder::SetFlags(QueueFlags newFlags) {
        flags = newFlags;
    }

    void QueueBuilder::SetCommandMemorySize(u64 size) {
        commandMemorySize = size;
    }

    void QueueBuilder::SetComputeMemorySize(u64 size) {
        computeMemorySize = size;
    }

    void QueueBuilder::SetControlMemorySize(u64 size) {
        controlMemorySize = size;
    }

    u64 QueueBuilder::GetQueueMemorySize() const {
        return PAGE_SIZE;
        //throw std::runtime_error("GetQueueMemorySize is unimplemented");
    }

    void QueueBuilder::SetQueueMemory(void *memory, u64 size) {
        queueMemory = std::span<u8>{static_cast<u8 *>(memory), size};
    }

    void QueueBuilder::SetCommandFlushThreshold(u64 threshold) {
        commandFlushThreshold = threshold;
    }

    const Device *QueueBuilder::GetDevice() const {
        return device;
    }

    QueueFlags QueueBuilder::GetFlags() const {
        return flags;
    }

    bool QueueBuilder::GetCommandMemorySize(i32 *i) const {
        throw std::runtime_error("GetCommandMemorySize is unimplemented");
    }

    bool QueueBuilder::GetComputeMemorySize(i32 *i) const {
        throw std::runtime_error("GetComputeMemorySize is unimplemented");
    }

    bool QueueBuilder::GetControlMemorySize(i32 *i) const {
        throw std::runtime_error("GetControlMemorySize is unimplemented");
    }

    bool QueueBuilder::GetCommandFlushThreshold(i32 *i) const {
        throw std::runtime_error("GetCommandFlushThreshold is unimplemented");
    }

    int QueueBuilder::GetMemorySize() const {
        throw std::runtime_error("GetMemorySize is unimplemented");
    }

    void *QueueBuilder::GetMemory() const {
        return queueMemory.data();
    }

    /* Wrappers */
    void Context::QueueBuilderSetDefaults(QueueBuilder *builder) {
        new (builder) QueueBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, QueueBuilder, SetDevice, Device *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, QueueBuilder, SetFlags, QueueFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, QueueBuilder, SetCommandMemorySize, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, QueueBuilder, SetComputeMemorySize, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, QueueBuilder, SetControlMemorySize, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, QueueBuilder, GetQueueMemorySize)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, QueueBuilder, SetQueueMemory, void *, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, QueueBuilder, SetCommandFlushThreshold, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const Device *, QueueBuilder, GetDevice)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(QueueFlags, QueueBuilder, GetFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, QueueBuilder, GetCommandMemorySize, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, QueueBuilder, GetComputeMemorySize, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, QueueBuilder, GetControlMemorySize, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, QueueBuilder, GetCommandFlushThreshold, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(int, QueueBuilder, GetMemorySize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void *, QueueBuilder, GetMemory)
    /* End wrappers */

    Queue::Queue(ApiVersion version, const QueueBuilder &builder) {
        NNVK_FILL_VERSIONED_STRUCT(Queue);
    }

    Queue::~Queue() {}

    QueueGetErrorResult Queue::GetError(QueueErrorInfo *errorInfo) {
        throw std::runtime_error("GetError is unimplemented");
    }

    u64 Queue::GetTotalCommandMemoryUsed() {
        throw std::runtime_error("GetTotalCommandMemoryUsed is unimplemented");
    }

    u64 Queue::GetTotalControlMemoryUsed() {
        throw std::runtime_error("GetTotalControlMemoryUsed is unimplemented");
    }

    u64 Queue::GetTotalComputeMemoryUsed() {
        throw std::runtime_error("GetTotalComputeMemoryUsed is unimplemented");
    }

    void Queue::ResetMemoryUsageCounts() {
        throw std::runtime_error("ResetMemoryUsageCounts is unimplemented");
    }

    void Queue::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void Queue::SubmitCommands(int numCommands, const CommandHandle *handles) {
        throw std::runtime_error("SubmitCommands is unimplemented");
    }

    void Queue::Flush() {
        throw std::runtime_error("Flush is unimplemented");
    }

    void Queue::Finish() {
        throw std::runtime_error("Finish is unimplemented");
    }

    void Queue::PresentTexture(Window *window, int *textureIndex) {
        throw std::runtime_error("PresentTexture is unimplemented");
    }

    void Queue::FenceSync(Sync *sync, SyncCondition condition, SyncFlags flags) {
        throw std::runtime_error("FenceSync is unimplemented");
    }

    bool Queue::WaitSync(Sync *sync) {
        throw std::runtime_error("WaitSync is unimplemented");
    }

    /* Wrappers */
    bool Context::QueueInitialize(Queue *pool, const QueueBuilder *builder) {
        new (pool) Queue(apiVersion, *builder);
        return true;
    }

    void Context::QueueFinalize(Queue *pool) {
        pool->~Queue();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(QueueGetErrorResult, Queue, GetError, QueueErrorInfo *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Queue, GetTotalCommandMemoryUsed)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Queue, GetTotalControlMemoryUsed)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Queue, GetTotalComputeMemoryUsed)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void, Queue, ResetMemoryUsageCounts)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Queue, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Queue, SubmitCommands, i32, const CommandHandle *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void, Queue, Flush)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void, Queue, Finish)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Queue, PresentTexture, Window *, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, Queue, FenceSync, Sync *, SyncCondition, SyncFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, Queue, WaitSync, Sync *)
    /* End wrappers */
}