// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <adrenotools/driver.h>
#include "types.h"

namespace nnvk::vkcore {
    /**
     * @brief Checks and stores all the traits of the host GPU discovered at runtime
     */
    class TraitManager {
      public:
        bool supportsUint8Indices{};
        bool supportsSamplerMirrorClampToEdge{};
        bool supportsSamplerReductionMode{};
        bool supportsCustomBorderColor{};
        bool supportsAnisotropicFiltering{};
        bool supportsLastProvokingVertex{};
        bool supportsLogicOp{};
        bool supportsVertexAttributeDivisor{};
        bool supportsVertexAttributeZeroDivisor{};
        bool supportsPushDescriptors{};
        bool supportsImageFormatList{};
        bool supportsImagelessFramebuffers{};
        bool supportsMultipleViewports{};
        bool supportsShaderViewportIndexLayer{};
        bool supportsSpirv14{};
        bool supportsShaderDemoteToHelper{};
        bool supportsFloat16{};
        bool supportsInt8{};
        bool supportsInt16{};
        bool supportsInt64{};
        bool supportsAtomicInt64{};
        bool supportsTransformFeedback{};
        bool supportsImageReadWithoutFormat{};
        bool supportsTopologyListRestart{};
        bool supportsTopologyPatchListRestart{};
        bool supportsGeometryShaders;
        bool supportsVertexPipelineStoresAndAtomics{};
        bool supportsFragmentStoresAndAtomics{};
        bool supportsShaderStorageImageWriteWithoutFormat{};
        bool supportsSubgroupVote{};
        bool supportsWideLines{};
        bool supportsDepthClamp{};
        bool supportsExtendedDynamicState{};
        bool supportsNullDescriptor{};
        u32 subgroupSize{};
        u32 hostVisibleCoherentCachedMemoryType{std::numeric_limits<u32>::max()};
        u32 minimumStorageBufferAlignment{};

        u32 vendorId{};
        u32 deviceId{};
        u32 driverVersion{};
        std::array<u8, VK_UUID_SIZE> pipelineCacheUuid{};

        std::bitset<7> bcnSupport{}; //!< Bitmask of BCn texture formats supported, it is ordered as BC1, BC2, BC3, BC4, BC5, BC6H and BC7
        bool supportsAdrenoDirectMemoryImport{};

        /**
         * @brief Manages a list of any vendor/device-specific errata in the host GPU
         */
        struct QuirkManager {
            bool adrenoRelaxedFormatAliasing{}; //!< [Adreno Proprietary/Freedreno] An indication that the GPU supports a relaxed version of view format aliasing without needing VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, this is designed to work in tandem with 'vkImageMutableFormatCostly'
            bool adrenoBrokenFormatReport{}; //!< [Adreno Proprietary] If the drivers report format support incorrectly and include cases that are supported by the hardware
            bool brokenPushDescriptors{}; //!< [Adreno Proprietary] A bug that causes push descriptor updates to ignored by the driver in certain situations
            bool brokenSpirvPositionInput{}; //!< [Adreno Proprietary] A bug that causes the shader compiler to fail on shaders with vertex position inputs not contained within a struct
            bool brokenSpirvAccessChainOpt{}; //!< [Adreno Proprietary] A broken optimisation pass causes dynamic access chain offsets to break
            bool brokenMultithreadedPipelineCompilation{}; //!< [Qualcomm Proprietary] A bug that causes the shader compiler to crash when compiling pipelines on multiple threads simultaneously
            bool brokenSubgroupMaskExtractDynamic{};  //!< [Qualcomm Proprietary] A bug that causes shaders using OpVectorExtractDynamic on the subgroup mask builtins to fail to compile
            bool brokenSubgroupShuffle{}; //!< [Qualcomm Proprietary] A bug that causes shaders using OpSubgroupShuffle to do all sorts of weird things
            bool brokenSpirvVectorAccessChain{}; //!< [Qualcomm Proprietary] A bug that causes SPIR-V OpAccessChains to work incorrectly when used to index vector arrays
            bool brokenDynamicStateVertexBindings{};  //!< [ARM Proprietary] A bug that causes VK_EXT_dynamic_state vertex bindings not to work correctly

            QuirkManager() = default;

            QuirkManager(const vk::PhysicalDeviceProperties &deviceProperties, const vk::PhysicalDeviceDriverProperties &driverProperties);
        } quirks;

        TraitManager() = default;

        using DeviceProperties2 = vk::StructureChain<
            vk::PhysicalDeviceProperties2,
            vk::PhysicalDeviceDriverProperties,
            vk::PhysicalDeviceFloatControlsProperties,
            vk::PhysicalDeviceTransformFeedbackPropertiesEXT,
            vk::PhysicalDeviceSubgroupProperties>;

        using DeviceFeatures2 = vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceCustomBorderColorFeaturesEXT,
            vk::PhysicalDeviceVertexAttributeDivisorFeaturesEXT,
            vk::PhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT,
            vk::PhysicalDeviceShaderFloat16Int8Features,
            vk::PhysicalDeviceShaderAtomicInt64Features,
            vk::PhysicalDeviceUniformBufferStandardLayoutFeatures,
            vk::PhysicalDeviceShaderDrawParametersFeatures,
            vk::PhysicalDeviceProvokingVertexFeaturesEXT,
            vk::PhysicalDevicePrimitiveTopologyListRestartFeaturesEXT,
            vk::PhysicalDeviceImagelessFramebufferFeatures,
            vk::PhysicalDeviceTransformFeedbackFeaturesEXT,
            vk::PhysicalDeviceIndexTypeUint8FeaturesEXT,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
            vk::PhysicalDeviceRobustness2FeaturesEXT,
            vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>;

        TraitManager(const DeviceFeatures2 &deviceFeatures2, DeviceFeatures2 &enabledFeatures2, const std::vector<vk::ExtensionProperties> &deviceExtensions, std::vector<std::array<char, VK_MAX_EXTENSION_NAME_SIZE>> &enabledExtensions, const DeviceProperties2 &deviceProperties2, const vk::raii::PhysicalDevice &physicalDevice);

        /**
         * @brief Applies driver specific binary patches to the driver (e.g. BCeNabler)
         */
        void ApplyDriverPatches(const vk::raii::Context &context, void *adrenotoolsImportHandle);
    };
}
