#pragma once

#include <mutex>
#include <vulkan/vulkan_raii.hpp>
#include <adrenotools/driver.h>
#include "types.h"
#include "trait_manager.h"
#include "memory_manager.h"

namespace nnvk {
    struct VkCore {
        void *adrenotoolsImportHandle{}; //!< Handle used by adrenotools to store mapping import info
        vkcore::TraitManager traitsManager;
        vk::raii::Context context;
        vk::raii::Instance instance;
        vk::raii::DebugReportCallbackEXT debugReportCallback; //!< An RAII Vulkan debug report manager which calls into 'GPU::DebugCallback'
        vk::raii::PhysicalDevice physicalDevice;
        u32 queueFamilyIndex{};
        vk::raii::Device device;
        std::mutex queueMutex; //!< Synchronizes access to the queue as it is externally synchronized
        vk::raii::Queue queue; //!< A Vulkan Queue supporting graphics and compute operations
        vkcore::MemoryManager memoryManager;

        VkCore(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, void *adrenotoolsImportHandle, vk::ApplicationInfo applicationInfo, bool enableValidation);
    };
}