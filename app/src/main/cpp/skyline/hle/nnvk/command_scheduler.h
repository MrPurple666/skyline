#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <variant>
#include <functional>
#include <optional>
#include <atomic>
#include <thread>
#include <vulkan/vulkan_raii.hpp>
#include "types.h"

namespace nnvk {
    struct VkCore;
}
namespace nnvk::vkcore {
    struct BinarySemaphore {
        std::mutex mutex;
        std::condition_variable cv;
        bool submitted{};
        vk::raii::Semaphore semaphore;

        BinarySemaphore(vk::raii::Semaphore semaphore);

        BinarySemaphore(const BinarySemaphore &) = delete;
        BinarySemaphore &operator=(const BinarySemaphore &) = delete;

        BinarySemaphore(BinarySemaphore &&);

        void MarkSubmitted();

        void WaitSubmitted();
    };

    class CommandScheduler {
      public:
        struct SyncOperation {
            u64 waitTimelineValue{};
            u64 signalTimelineValue{};
            vk::PipelineStageFlags2KHR waitStageMask{};
            vk::PipelineStageFlags2KHR signalStageMask{};
            BinarySemaphore *signalBinarySemaphore{};
            std::optional<BinarySemaphore> waitBinarySemaphore;
        };

        struct CommandOperation {};

        using Operation = std::variant<SyncOperation, CommandOperation>;

      private:
        VkCore &vkCore;

        std::queue<Operation> operationQueue;
        std::condition_variable operationQueueCv;
        std::mutex operationQueueMutex;

        vk::raii::Semaphore timeline;
        std::atomic<u64> timelineMax{1};
        std::atomic<u64> timelineMin{1};

        std::thread operationThread;

        void OperationThread();

      public:
        enum class WaitResult {
            AlreadySignaled,
            WaitSignalled,
            WaitTimeout,
        };

        CommandScheduler(VkCore &vkCore);

        void GenAndQueueOperation(std::function<Operation(void)> &&func);

        WaitResult WaitTimeline(u64 value, u64 timeout);

        /**
         * @brief Increments the timeline and returns the new value
         */
        u64 IncrTimeline();

        void Flush();
    };
}