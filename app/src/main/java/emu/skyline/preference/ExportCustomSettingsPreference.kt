/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline.preference

import android.content.Context
import android.util.AttributeSet
import androidx.preference.Preference
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import android.content.ClipData
import android.content.ClipboardManager
import emu.skyline.settings.EmulationSettings

/**
 * Copies global emulation settings into the current shared preferences, showing a dialog to confirm the action
 * This preference recreates the activity to update the UI after modifying shared preferences
 */
class ExportCustomSettingsPreference @JvmOverloads constructor(context : Context, attrs : AttributeSet? = null, defStyleAttr : Int = androidx.preference.R.attr.preferenceStyle) : Preference(context, attrs, defStyleAttr) {

    init {
        setOnPreferenceClickListener {
            var emulationSettings = EmulationSettings.forPrefName(preferenceManager.sharedPreferencesName)

            if (!emulationSettings.useCustomSettings) {
                emulationSettings = EmulationSettings.global
            }

            val systemIsDocked = emulationSettings.isDocked;

            val gpuDriver = emulationSettings.gpuDriver;
            val gpuTripleBuffering = emulationSettings.forceTripleBuffering;
            val gpuExecSlotCount = emulationSettings.executorSlotCountScale
            val gpuExecFlushThreshold = emulationSettings.executorFlushThreshold;
            val gpuDMI = emulationSettings.useDirectMemoryImport;
            val gpuFreeGuestTextureMemory = emulationSettings.freeGuestTextureMemory;
            val gpuDisableShaderCache = emulationSettings.disableShaderCache;
            val gpuForceMaxGpuClocks = emulationSettings.forceMaxGpuClocks

            val hackFastGpuReadback = emulationSettings.enableFastGpuReadbackHack;
            val hackFastReadbackWrite = emulationSettings.enableFastReadbackWrites;
            val hackDisableSubgroupShuffle = emulationSettings.disableSubgroupShuffle;

            val settingsAsText = String.format(
                """
                SYSTEM
                - Docked: $systemIsDocked
                
                GPU
                - Driver: $gpuDriver (executors: $gpuExecSlotCount slots, threshold of $gpuExecFlushThreshold)
                - Triple buffering: $gpuTripleBuffering, DMI: $gpuDMI
                - Max clocks: $gpuForceMaxGpuClocks, free guest texture memory: $gpuFreeGuestTextureMemory
                - Disable shader cache: $gpuDisableShaderCache
                
                HACKS
                - Fast GPU readback: $hackFastGpuReadback, fast readback writes $hackFastReadbackWrite
                - Disable GPU subgroup shuffle: $hackDisableSubgroupShuffle
                """.trimIndent()
            )

            MaterialAlertDialogBuilder(context)
                .setTitle(title)
                .setMessage(settingsAsText)
                .setPositiveButton(android.R.string.copy) { _, _ ->
                    // Copy the current settings as text to the system clipboard
                    val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                    val clip = ClipData.newPlainText("label", settingsAsText)
                    clipboard.setPrimaryClip(clip)

                }
                .setNegativeButton(android.R.string.ok, null)
                .show()

            true
        }
    }
}
