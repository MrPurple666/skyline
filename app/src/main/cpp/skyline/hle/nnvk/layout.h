// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace nnvk::layout {
    struct MipLevelLayout {
        vk::Extent3D dimensions; //!< The dimensions of the mipmapped level, these are exact dimensions and not aligned to a GOB
        size_t linearSize; //!< The size of a linear image with this mipmapped level in bytes
        size_t targetLinearSize; //!< The size of a linear image with this mipmapped level in bytes and using the target format, this will only differ from linearSize if the target format is supplied
        size_t blockLinearSize; //!< The size of a blocklinear image with this mipmapped level in bytes
        size_t blockHeight, blockDepth; //!< The block height and block depth set for the level

        constexpr MipLevelLayout(vk::Extent3D dimensions,
                                 size_t linearSize, size_t targetLinearSize, size_t blockLinearSize,
                                 size_t blockHeight, size_t blockDepth)
            : dimensions{dimensions},
              linearSize{linearSize}, targetLinearSize{targetLinearSize}, blockLinearSize{blockLinearSize},
              blockHeight{blockHeight}, blockDepth{blockDepth} {}
    };

    /**
     * @return The size of a layer of the specified non-mipmapped block-slinear surface in bytes
     */
    size_t GetBlockLinearLayerSize(vk::Extent3D dimensions,
                                   size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb,
                                   size_t gobBlockHeight, size_t gobBlockDepth);

    /**
     * @param isMultiLayer If the texture has more than one layer, a multi-layer texture requires alignment to a block at layer end
     * @return The size of a layer of the specified block-linear surface in bytes
     */
    size_t GetBlockLinearLayerSize(vk::Extent3D dimensions,
                                   size_t formatBlockHeight, size_t formatBlockWidth, size_t formatBpb,
                                   size_t gobBlockHeight, size_t gobBlockDepth,
                                   size_t levelCount, bool isMultiLayer);

    /**
     * @note The target format is the format of the texture after it has been decoded, if bpb is 0, the target format is the same as the source format
     * @return A vector of metadata about every mipmapped level of the supplied block-linear surface
     */
    std::vector<MipLevelLayout> GetBlockLinearMipLayout(vk::Extent3D dimensions,
                                                        size_t formatBlockHeight, size_t formatBlockWidth, size_t formatBpb,
                                                        size_t targetFormatBlockHeight, size_t targetFormatBlockWidth, size_t targetFormatBpb,
                                                        size_t gobBlockHeight, size_t gobBlockDepth,
                                                        size_t levelCount);

    /**
     * @brief Copies the contents of a blocklinear texture to a linear output buffer
     */
    void CopyBlockLinearToLinear(vk::Extent3D dimensions,
                                 size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb,
                                 size_t gobBlockHeight, size_t gobBlockDepth,
                                 u8 *blockLinear, u8 *linear);

    /**
     * @brief Copies the contents of a blocklinear texture to a pitch texture
     */
    void CopyBlockLinearToPitch(vk::Extent3D dimensions,
                                size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb, u32 pitchAmount,
                                size_t gobBlockHeight, size_t gobBlockDepth,
                                u8 *blockLinear, u8 *pitch);

    /**
     * @brief Copies the contents of a part of a blocklinear texture to a pitch texture
     */
    void CopyBlockLinearToPitchSubrect(vk::Extent3D pitchDimensions, vk::Extent3D blockLinearDimensions,
                                       size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb, u32 pitchAmount,
                                       size_t gobBlockHeight, size_t gobBlockDepth,
                                       u8 *blockLinear, u8 *pitch,
                                       u32 originX, u32 originY);

    /**
     * @brief Copies the contents of a linear buffer to a blocklinear texture
     */
    void CopyLinearToBlockLinear(vk::Extent3D dimensions,
                                 size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb,
                                 size_t gobBlockHeight, size_t gobBlockDepth,
                                 u8 *linear, u8 *blockLinear);

    /**
     * @brief Copies the contents of a pitch texture to a blocklinear texture
     */
    void CopyPitchToBlockLinear(vk::Extent3D dimensions,
                                size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb, u32 pitchAmount,
                                size_t gobBlockHeight, size_t gobBlockDepth,
                                u8 *pitch, u8 *blockLinear);

    /**
     * @brief Copies the contents of a linear texture to a part of a blocklinear texture
     */
    void CopyLinearToBlockLinearSubrect(vk::Extent3D linearDimensions, vk::Extent3D blockLinearDimensions,
                                        size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb,
                                        size_t gobBlockHeight, size_t gobBlockDepth,
                                        u8 *linear, u8 *blockLinear,
                                        u32 originX, u32 originY);

    /**
     * @brief Copies the contents of a pitch texture to a part of a blocklinear texture
     */
    void CopyPitchToBlockLinearSubrect(vk::Extent3D pitchDimensions, vk::Extent3D blockLinearDimensions,
                                       size_t formatBlockWidth, size_t formatBlockHeight, size_t formatBpb, u32 pitchAmount,
                                       size_t gobBlockHeight, size_t gobBlockDepth,
                                       u8 *pitch, u8 *blockLinear,
                                       u32 originX, u32 originY);



    u8 SelectBlockLinearTileHeight(size_t height, size_t formatBlockHeight);

    u8 SelectBlockLinearTileDepth(size_t depth);
}
