#include <mutex>
#include <queue>
#include <functional>
#include <atomic>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>
#include <boost/container/static_vector.hpp>
#include "types.h"
#include "vkcore.h"
#include "command_scheduler.h"

namespace nnvk::vkcore {
    BinarySemaphore::BinarySemaphore(vk::raii::Semaphore semaphore)
        : semaphore{std::move(semaphore)} {}

    BinarySemaphore::BinarySemaphore(BinarySemaphore &&other)
        : semaphore{std::move(other.semaphore)} {}

    void BinarySemaphore::MarkSubmitted() {
        {
            std::scoped_lock lock{mutex};
            submitted = true;
        }
        cv.notify_all();
    }

    void BinarySemaphore::WaitSubmitted() {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] { return submitted; });
    }

    static vk::raii::Semaphore CreateTimeline(const vk::raii::Device &device) {
        vk::StructureChain<vk::SemaphoreCreateInfo, vk::SemaphoreTypeCreateInfo> createInfo{
            vk::SemaphoreCreateInfo{},
            vk::SemaphoreTypeCreateInfo{
                .semaphoreType = vk::SemaphoreType::eTimeline,
                .initialValue = 1,
            },
        };

        return vk::raii::Semaphore(device, createInfo.get<vk::SemaphoreCreateInfo>());
    }

    void CommandScheduler::OperationThread() {
        while (true) {
            std::optional<Operation> operation;
            {
                std::unique_lock lock{operationQueueMutex};
                operationQueueCv.wait(lock, [this] { return !operationQueue.empty(); });
                operation.emplace(std::move(operationQueue.front()));
                operationQueue.pop();
            }

            if (auto *syncOperation{std::get_if<SyncOperation>(&*operation)}; syncOperation) {
                boost::container::static_vector<vk::SemaphoreSubmitInfoKHR, 2> waitSemaphoreInfo;

                waitSemaphoreInfo.emplace_back(
                    vk::SemaphoreSubmitInfoKHR{
                        .semaphore = *timeline,
                        .value = syncOperation->waitTimelineValue,
                        .stageMask = syncOperation->waitStageMask,
                    });

                if (syncOperation->waitBinarySemaphore) {
                    // TODO NOT DO THIS!!
                    if (syncOperation->waitBinarySemaphore->submitted) {
                        syncOperation->waitBinarySemaphore->WaitSubmitted();
                        waitSemaphoreInfo.emplace_back(vk::SemaphoreSubmitInfoKHR{
                            .semaphore = *syncOperation->waitBinarySemaphore->semaphore,
                            .stageMask = syncOperation->waitStageMask,
                        });
                    }
                }

                boost::container::static_vector<vk::SemaphoreSubmitInfoKHR, 2> signalSemaphoreInfo;
                signalSemaphoreInfo.emplace_back(
                    vk::SemaphoreSubmitInfoKHR{
                        .semaphore = *timeline,
                        .value = syncOperation->signalTimelineValue,
                        .stageMask = syncOperation->signalStageMask,
                    });

                if (syncOperation->signalBinarySemaphore)
                    signalSemaphoreInfo.emplace_back(vk::SemaphoreSubmitInfoKHR{
                        .semaphore = *syncOperation->signalBinarySemaphore->semaphore,
                        .stageMask = syncOperation->signalStageMask,
                    });

                vk::SubmitInfo2KHR info{
                    .flags = {},
                    .waitSemaphoreInfoCount = static_cast<u32>(waitSemaphoreInfo.size()),
                    .pWaitSemaphoreInfos = waitSemaphoreInfo.data(),
                    .commandBufferInfoCount = 0,
                    .pCommandBufferInfos = nullptr,
                    .signalSemaphoreInfoCount = static_cast<u32>(signalSemaphoreInfo.size()),
                    .pSignalSemaphoreInfos = signalSemaphoreInfo.data(),
                };

                std::scoped_lock lock{vkCore.queueMutex};
                vkCore.queue.submit2KHR(info);
                if (syncOperation->waitBinarySemaphore)
                    syncOperation->waitBinarySemaphore->MarkSubmitted();
            }
        }

    }

    CommandScheduler::CommandScheduler(VkCore &vkCore)
        : vkCore{vkCore},
          operationThread{&CommandScheduler::OperationThread, this},
          timeline{CreateTimeline(vkCore.device)} {}

    void CommandScheduler::GenAndQueueOperation(std::function<Operation(void)> &&func) {
        {
            std::scoped_lock lock{operationQueueMutex};
            operationQueue.push(func());
        }

        operationQueueCv.notify_one();
    }

    CommandScheduler::WaitResult CommandScheduler::WaitTimeline(u64 value, u64 timeout) {
        if (timelineMin.load() >= value)
            return WaitResult::AlreadySignaled;

        auto ret{vkCore.device.waitSemaphores(vk::SemaphoreWaitInfo{
            .semaphoreCount = 1,
            .pSemaphores = &*timeline,
            .pValues = &value,
        }, timeout)};


        // Update the timeline minimum value to max (value, timelineMin)
        u64 prev{timelineMin.load()};
        while (prev < value && !timelineMin.compare_exchange_weak(prev, value));

        if (ret == vk::Result::eSuccess)
            return WaitResult::WaitSignalled;
        else if (ret == vk::Result::eTimeout)
            return WaitResult::WaitTimeout;
        else
            throw std::runtime_error("Unexpected result from vk::Device::waitSemaphores");
    }

    u64 CommandScheduler::IncrTimeline() {
        return timelineMax.fetch_add(1) + 1;
    }

    void CommandScheduler::Flush() {}
}