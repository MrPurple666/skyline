// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>
#include "types.h"
#include "vkcore.h"
#include "memory_manager.h"

namespace nnvk::vkcore {
    /**
     * @brief If the result isn't VK_SUCCESS then an exception is thrown
     */
    void ThrowOnFail(VkResult result, const char *function = __builtin_FUNCTION()) {
        if (result != VK_SUCCESS)
            vk::throwResultException(vk::Result(result), function);
    }

    std::span<u8> MemoryAllocation::data() {
        if (!mapping.empty()) [[likely]]
            return mapping;

        VmaAllocationInfo allocationInfo{};
        vmaGetAllocationInfo(vmaAllocator, vmaAllocation, &allocationInfo);

        void *pointer;
        ThrowOnFail(vmaMapMemory(vmaAllocator, vmaAllocation, &pointer));
        mapping = std::span(reinterpret_cast<u8 *>(pointer), allocationInfo.size);
        return mapping;
    }

    MemoryAllocation::~MemoryAllocation() {
        if (!mapping.empty())
            vmaUnmapMemory(vmaAllocator, vmaAllocation);

        if (vmaAllocation)
            vmaFreeMemory(vmaAllocator, vmaAllocation);
    }

    Buffer::~Buffer() {
        if (vkBuffer)
            (*vkDevice).destroy(vkBuffer, nullptr, *vkDevice.getDispatcher());
    }

    Image::~Image() {
        if (vkImage)
            (*vkDevice).destroy(vkImage, nullptr, *vkDevice.getDispatcher());
    }

    std::span<u8> Image::data() {
        return allocation.data();
    }

    MemoryManager::MemoryManager(VkCore &core) : core{core} {
        auto instanceDispatcher{core.instance.getDispatcher()};
        auto deviceDispatcher{core.device.getDispatcher()};
        VmaVulkanFunctions vulkanFunctions{
            .vkGetPhysicalDeviceProperties = instanceDispatcher->vkGetPhysicalDeviceProperties,
            .vkGetPhysicalDeviceMemoryProperties = instanceDispatcher->vkGetPhysicalDeviceMemoryProperties,
            .vkAllocateMemory = deviceDispatcher->vkAllocateMemory,
            .vkFreeMemory = deviceDispatcher->vkFreeMemory,
            .vkMapMemory = deviceDispatcher->vkMapMemory,
            .vkUnmapMemory = deviceDispatcher->vkUnmapMemory,
            .vkFlushMappedMemoryRanges = deviceDispatcher->vkFlushMappedMemoryRanges,
            .vkInvalidateMappedMemoryRanges = deviceDispatcher->vkInvalidateMappedMemoryRanges,
            .vkBindBufferMemory = deviceDispatcher->vkBindBufferMemory,
            .vkBindImageMemory = deviceDispatcher->vkBindImageMemory,
            .vkGetBufferMemoryRequirements = deviceDispatcher->vkGetBufferMemoryRequirements,
            .vkGetImageMemoryRequirements = deviceDispatcher->vkGetImageMemoryRequirements,
            .vkCreateBuffer = deviceDispatcher->vkCreateBuffer,
            .vkDestroyBuffer = deviceDispatcher->vkDestroyBuffer,
            .vkCreateImage = deviceDispatcher->vkCreateImage,
            .vkDestroyImage = deviceDispatcher->vkDestroyImage,
            .vkCmdCopyBuffer = deviceDispatcher->vkCmdCopyBuffer,
            .vkGetBufferMemoryRequirements2KHR = deviceDispatcher->vkGetBufferMemoryRequirements2,
            .vkGetImageMemoryRequirements2KHR = deviceDispatcher->vkGetImageMemoryRequirements2,
            .vkBindBufferMemory2KHR = deviceDispatcher->vkBindBufferMemory2,
            .vkBindImageMemory2KHR = deviceDispatcher->vkBindImageMemory2,
            .vkGetPhysicalDeviceMemoryProperties2KHR = instanceDispatcher->vkGetPhysicalDeviceMemoryProperties2,
        };
        VmaAllocatorCreateInfo allocatorCreateInfo{
            .physicalDevice = *core.physicalDevice,
            .device = *core.device,
            .instance = *core.instance,
            .pVulkanFunctions = &vulkanFunctions,
            .vulkanApiVersion = VK_API_VERSION_1_1,
        };
        ThrowOnFail(vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator));
    }

    MemoryManager::~MemoryManager() {
        vmaDestroyAllocator(vmaAllocator);
    }

    Buffer MemoryManager::AllocateBuffer(vk::DeviceSize size) {
        vk::BufferCreateInfo bufferCreateInfo{
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformTexelBuffer | vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &core.queueFamilyIndex,
        };
        VmaAllocationCreateInfo allocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_UNKNOWN,
            .requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal),
        };

        VkBuffer buffer;
        VmaAllocation allocation;
        ThrowOnFail(vmaCreateBuffer(vmaAllocator, &static_cast<const VkBufferCreateInfo &>(bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, nullptr));

        return Buffer{MemoryAllocation{vmaAllocator, allocation}, core.device, buffer};
    }

    MemoryAllocation MemoryManager::AllocateImageMemory(const vk::ImageCreateInfo &createInfo) {
        VmaAllocationCreateInfo allocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };


        VkImage image;
        VmaAllocation allocation;
        ThrowOnFail(vmaCreateImage(vmaAllocator, &static_cast<const VkImageCreateInfo &>(createInfo), &allocationCreateInfo, &image, &allocation, nullptr));

        (*core.device).destroy(image, nullptr, *core.device.getDispatcher());

        return MemoryAllocation{vmaAllocator, allocation};
    }

    Image MemoryManager::AllocateImage(const vk::ImageCreateInfo &createInfo) {
        VmaAllocationCreateInfo allocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };

        VkImage image;
        VmaAllocation allocation;
        ThrowOnFail(vmaCreateImage(vmaAllocator, &static_cast<const VkImageCreateInfo &>(createInfo), &allocationCreateInfo, &image, &allocation, nullptr));

        return Image{MemoryAllocation{vmaAllocator,  allocation}, core.device, image};
    }

    Image MemoryManager::AllocateMappedImage(const vk::ImageCreateInfo &createInfo) {
        VmaAllocationCreateInfo allocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_UNKNOWN,
            .requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal),
        };

        VkImage image;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
        ThrowOnFail(vmaCreateImage(vmaAllocator, &static_cast<const VkImageCreateInfo &>(createInfo), &allocationCreateInfo, &image, &allocation, &allocationInfo));

        return Image{MemoryAllocation{vmaAllocator,  allocation}, core.device, image};
    }

    ImportedBuffer MemoryManager::ImportBuffer(std::span<u8> cpuMapping) {
        if (!core.traitsManager.supportsAdrenoDirectMemoryImport)
            throw std::runtime_error("Cannot import host buffers without adrenotools import support!");

        if (!adrenotools_import_user_mem(core.adrenotoolsImportHandle, cpuMapping.data(), cpuMapping.size()))
            throw std::runtime_error("Failed to import user memory");

        auto buffer{core.device.createBuffer(vk::BufferCreateInfo{
            .size = cpuMapping.size(),
            .usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformTexelBuffer | vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT,
            .sharingMode = vk::SharingMode::eExclusive
        })};

        auto memory{core.device.allocateMemory(vk::MemoryAllocateInfo{
            .allocationSize = cpuMapping.size(),
            .memoryTypeIndex = core.traitsManager.hostVisibleCoherentCachedMemoryType,
        })};

        if (!adrenotools_validate_gpu_mapping(core.adrenotoolsImportHandle))
            throw std::runtime_error("Failed to validate GPU mapping");

        core.device.bindBufferMemory2({vk::BindBufferMemoryInfo{
            .buffer = *buffer,
            .memory = *memory,
            .memoryOffset = 0
        }});

        return ImportedBuffer{std::move(buffer), std::move(memory), cpuMapping};
    }

    vk::raii::Buffer MemoryManager::CreateAliasingBuffer(vk::DeviceMemory deviceMemory, vk::DeviceSize offset, vk::DeviceSize size) {
        auto buffer{core.device.createBuffer(vk::BufferCreateInfo{
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformTexelBuffer | vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 0,
        })};

        core.device.bindBufferMemory2({vk::BindBufferMemoryInfo{
            .buffer = *buffer,
            .memory = deviceMemory,
            .memoryOffset = offset
        }});

        return buffer;
    }

}
