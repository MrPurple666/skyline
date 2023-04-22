// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <span>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>
#include "types.h"

namespace nnvk {
    struct VkCore;
}

namespace nnvk::vkcore {

    struct MemoryAllocation {
      private:
        std::span<u8> mapping{};

      public:
        VmaAllocator vmaAllocator;
        VmaAllocation vmaAllocation;

        constexpr MemoryAllocation(VmaAllocator vmaAllocator, VmaAllocation vmaAllocation)
            : vmaAllocator{vmaAllocator},
              vmaAllocation{vmaAllocation} {}

        MemoryAllocation(const MemoryAllocation &) = delete;

        constexpr MemoryAllocation(MemoryAllocation &&other)
            : mapping{std::exchange(other.mapping, {})},
              vmaAllocator{std::exchange(other.vmaAllocator, nullptr)},
              vmaAllocation{std::exchange(other.vmaAllocation, nullptr)} {}

        MemoryAllocation &operator=(const MemoryAllocation &) = delete;

        MemoryAllocation &operator=(MemoryAllocation &&) = default;

        ~MemoryAllocation();

        /**
         * @return A span of the allocation mapped onto the CPU
         * @note If the allocation isn't already mapped on the CPU, this creates a mapping for it
         */
        std::span<u8> data();
    };

    /**
     * @brief A view into a CPU mapping of a Vulkan buffer
     * @note The mapping **should not** be used after the lifetime of the object has ended
     */
    struct Buffer {
        MemoryAllocation memoryAllocation;
        vk::raii::Device &vkDevice;
        vk::Buffer vkBuffer;

        Buffer(MemoryAllocation &&memoryAllocation, vk::raii::Device &vkDevice, vk::Buffer vkBuffer)
            : memoryAllocation{std::move(memoryAllocation)},
              vkDevice{vkDevice},
              vkBuffer{vkBuffer} {}

        Buffer(const Buffer &) = delete;

        constexpr Buffer(Buffer &&other)
            : memoryAllocation{std::move(other.memoryAllocation)},
              vkDevice{other.vkDevice},
              vkBuffer{std::exchange(other.vkBuffer, nullptr)} {}

        Buffer &operator=(const Buffer &) = delete;

        ~Buffer();
    };

    /**
     * @brief A buffer that directly owns it's own memory
     */
    struct ImportedBuffer {
        vk::raii::Buffer vkBuffer;
        vk::raii::DeviceMemory vkMemory;
        std::span<u8> backing;

        ImportedBuffer(vk::raii::Buffer vkBuffer, vk::raii::DeviceMemory vkMemory, std::span<u8> backing)
            : vkBuffer{std::move(vkBuffer)},
              vkMemory{std::move(vkMemory)},
              backing{backing} {}

        ImportedBuffer(const ImportedBuffer &) = delete;

        ImportedBuffer(ImportedBuffer &&other)
            : vkBuffer{std::move(other.vkBuffer)},
              vkMemory{std::move(other.vkMemory)},
              backing{other.backing} {}

        ImportedBuffer &operator=(const ImportedBuffer &) = delete;

        ImportedBuffer &operator=(ImportedBuffer &&) = default;
    };

    /**
     * @brief A Vulkan image which VMA allocates and manages the backing memory for
     * @note Any images created with VMA_ALLOCATION_CREATE_MAPPED_BIT must not be utilized with this since it'll unconditionally unmap when a pointer is present which is illegal when an image was created with that flag as unmapping will be automatically performed on image deletion
     */
    struct Image {
        MemoryAllocation allocation;
        vk::raii::Device &vkDevice;
        vk::Image vkImage;

        constexpr Image(MemoryAllocation &&allocation, vk::raii::Device &vkDevice, vk::Image vkImage)
            : allocation{std::move(allocation)},
              vkDevice{vkDevice},
              vkImage{vkImage} {}

        Image(const Image &) = delete;

        constexpr Image(Image &&other)
            : allocation{std::move(other.allocation)},
              vkDevice{other.vkDevice},
              vkImage{std::exchange(other.vkImage, nullptr)} {}

        Image &operator=(const Image &) = delete;

        ~Image();

        /**
         * @return A span of the allocation mapped onto the CPU
         */
        std::span<u8> data();
    };

    /**
     * @brief An abstraction over memory operations done in Vulkan, it's used for all allocations on the host GPU
     */
    class MemoryManager {
      private:
        VkCore &core;
        VmaAllocator vmaAllocator{VK_NULL_HANDLE};

      public:
        MemoryManager(VkCore &core);

        ~MemoryManager();

        /**
         * @brief Creates a buffer with a CPU mapping and all usage flags
         */
        Buffer AllocateBuffer(vk::DeviceSize size);

        /**
         * @brief Returns a memory allocation suitable for an image of the given create info
         */
        MemoryAllocation AllocateImageMemory(const vk::ImageCreateInfo &createInfo);

        /**
         * @brief Creates an image which is allocated and deallocated using RAII
         */
        Image AllocateImage(const vk::ImageCreateInfo &createInfo);

        /**
         * @brief Creates an image which is allocated and deallocated using RAII and is optimal for being mapped on the CPU
         */
        Image AllocateMappedImage(const vk::ImageCreateInfo &createInfo);

        /**
         * @brief Maps the input CPU mapped region into a new buffer
         */
        ImportedBuffer ImportBuffer(std::span<u8> cpuMapping);

        vk::raii::Buffer CreateAliasingBuffer(vk::DeviceMemory deviceMemory, vk::DeviceSize offset, vk::DeviceSize size);
    };
}
