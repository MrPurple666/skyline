/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline.utils

import android.content.Context
import emu.skyline.BuildConfig

/**
 * The settings that will be passed to libskyline when running and executable
 */
class NativeSettings(context : Context, pref : PreferenceSettings) {
    // System
    var isDocked : Boolean = if (pref.gamepCustomSettings) pref.gamepIsDocked else pref.isDocked
    var usernameValue : String = pref.usernameValue
    var profilePictureValue : String = pref.profilePictureValue
    var systemLanguage : Int = if (pref.gamepCustomSettings) pref.gamepSystemLanguage else pref.systemLanguage
    var systemRegion : Int = if (pref.gamepCustomSettings) pref.gamepSystemRegion else pref.systemRegion
    var internetEnabled : Boolean = if (pref.gamepCustomSettings) pref.gamepInternetEnabled else pref.internetEnabled

    // Display
    var forceTripleBuffering : Boolean = if (pref.gamepCustomSettings) pref.gamepForceTripleBuffering else pref.forceTripleBuffering
    var disableFrameThrottling : Boolean = if (pref.gamepCustomSettings) pref.gamepDisableFrameThrottling else pref.disableFrameThrottling
    var disableShaderCache : Boolean = if (pref.gamepCustomSettings) pref.gamepDisableShaderCache else pref.disableShaderCache

    // GPU
    var selectedGpuDriver : String = if (pref.gamepCustomSettings) pref.gamepGpuDriver else pref.gpuDriver
    var gpuDriver : String = if (selectedGpuDriver == PreferenceSettings.SYSTEM_GPU_DRIVER) "" else selectedGpuDriver
    var gpuDriverLibraryName : String = if (selectedGpuDriver == PreferenceSettings.SYSTEM_GPU_DRIVER) "" else GpuDriverHelper.getLibraryName(context, selectedGpuDriver)
    var executorSlotCountScale : Int = if (pref.gamepCustomSettings) pref.gamepExecutorSlotCountScale else pref.executorSlotCountScale
    var executorFlushThreshold : Int = if (pref.gamepCustomSettings) pref.gamepExecutorFlushThreshold else pref.executorFlushThreshold
    var useDirectMemoryImport : Boolean = if (pref.gamepCustomSettings) pref.gamepUseDirectMemoryImport else pref.useDirectMemoryImport
    var forceMaxGpuClocks : Boolean = if (pref.gamepCustomSettings) pref.gamepForceMaxGpuClocks else pref.forceMaxGpuClocks

    // Hacks
    var enableFastGpuReadbackHack : Boolean = if (pref.gamepCustomSettings) pref.gamepEnableFastGpuReadbackHack else pref.enableFastGpuReadbackHack
    var enableFastReadbackWrites : Boolean = pref.enableFastReadbackWrites
    var disableSubgroupShuffle : Boolean = pref.disableSubgroupShuffle

    // Audio
    var isAudioOutputDisabled : Boolean = if (pref.gamepCustomSettings) pref.gamepIsAudioOutputDisabled else pref.isAudioOutputDisabled

    // Debug
    var validationLayer : Boolean = BuildConfig.BUILD_TYPE != "release" && if (pref.gamepCustomSettings) pref.gamepValidationLayer else pref.validationLayer

    /**
     * Updates settings in libskyline during emulation
     */
    external fun updateNative()

    companion object {
        /**
         * Sets libskyline logger level to the given one
         */
        external fun setLogLevel(logLevel : Int)
    }
}
