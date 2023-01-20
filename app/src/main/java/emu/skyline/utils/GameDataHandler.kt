/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline.utils

import android.content.Context
import android.content.pm.ActivityInfo
import android.util.Log
import emu.skyline.data.AppItem
import kotlinx.serialization.Serializable
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import androidx.preference.Preference
import android.preference.PreferenceManager
/**
 * The settings that will be passed to libskyline when running and executable
 */
class GameDataHandler() {

    val gameDataFileName = "gameCustomData"

    fun writeGamesDataString(context : Context?, content : String) {
        context?.openFileOutput(gameDataFileName, 0).use {
            it?.write(content.toByteArray())
        }
    }

    fun readGamesDataString(context : Context?) : String {
        var content : String  = ""
        val json = Json { encodeDefaults = true }
        try {
            context?.openFileInput(gameDataFileName).use { stream ->
                val text = stream?.bufferedReader().use {
                    it?.readText()
                }
                content = text.toString()
            }
        }
        catch(e : Exception){
            var gameDataList : List<CustomGameData> = emptyList()
            writeGamesDataString(context, json.encodeToString(gameDataList))
            return "[]"
        }
        return content
    }


    private fun getGamesData(context : Context?) : List<CustomGameData>  {
        var gameDataList : List<CustomGameData>

        try{
            var gameDataFileString = readGamesDataString(context)
            gameDataList = Json.decodeFromString<List<CustomGameData>>(gameDataFileString)
        }
        catch (e : Exception) {
            Log.i("saveGameData Event", "No se encontró archivo de configuración")
            gameDataList = emptyList()
        }

        return gameDataList
    }

    fun getGameData(context: Context?, item : AppItem?) : CustomGameData  {
        var gameDataList : List<CustomGameData> = getGamesData(context)

        var gameDataElement = CustomGameData(item?.titleId, item?.version, item?.title )
        for (game in gameDataList) {
            if( game.titleId == item?.titleId && game.version == item?.version ) {
                gameDataElement = game
            }
        }
        return gameDataElement
    }

    fun getDefaultSettings(context: Context?) : CustomGameData  {
        var gameDataList : List<CustomGameData> = getGamesData(context)

        var gameDataElement = CustomGameData("default", "default", "default" )
        for (game in gameDataList) {
            if( game.titleId == "default" && game.version == "default" ) {
                gameDataElement = game
            }
        }
        return gameDataElement
    }

    fun saveGameData(context: Context?, gameData : CustomGameData)  {
        var gameDataList : MutableList<CustomGameData> = getGamesData(context).toMutableList()
        var counter : Int = 0
        var found : Boolean = false

        while (counter < gameDataList.count()) {
            if( (gameDataList[counter] as CustomGameData).titleId == gameData?.titleId && (gameDataList[counter] as CustomGameData).version == gameData?.version ) {
                gameDataList[counter] = gameData
                found = true
            }
            counter++
        }
        if(!found) {
            gameDataList.add(gameData)
        }

        val json = Json { encodeDefaults = true }
        writeGamesDataString(context, json.encodeToString(gameDataList.toList()))
    }

    fun loadGameSettings(context: Context?, item : AppItem?)  {
        var gameData = getGameData(context, item)

        var sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)
        var settings = sharedPreferences?.edit()

        settings?.putBoolean("gamep_custom_settings", gameData.customSettings)
        if(gameData.customSettings) {
            settings?.putBoolean("gamep_is_docked", gameData.isDocked)
            settings?.putInt("gamep_system_language", gameData.systemLanguage)
            settings?.putInt("gamep_system_region", gameData.systemRegion)
            settings?.putBoolean("fgamep_force_triple_buffering", gameData.forceTripleBuffering)
            settings?.putBoolean("gamep_disable_frame_throttling", gameData.disableFrameThrottling)
            settings?.putBoolean("gamep_max_refresh_rate", gameData.maxRefreshRate)
            settings?.putInt("gamep_aspect_ratio", gameData.aspectRatio)
            settings?.putInt("gamep_orientation", gameData.orientation)
            settings?.putString("gamep_gpu_driver", gameData.gpuDriver)
            settings?.putInt("gamep_executor_slot_count_scale", gameData.executorSlotCountScale)
            settings?.putInt("gamep_executor_flush_threshold", gameData.executorFlushThreshold)
            settings?.putBoolean("gamep_use_direct_memory_import", gameData.useDirectMemoryImport)
            settings?.putBoolean("gamep_force_max_gpu_clocks", gameData.forceMaxGpuClocks)
            settings?.putBoolean("gamep_enable_fast_gpu_readback_hack", gameData.enableFastGpuReadbackHack)
            settings?.putBoolean("gamep_is_audio_output_disabled", gameData.isAudioOutputDisabled)
            settings?.putBoolean("gamep_validation_layer", gameData.validationLayer)
            settings?.putBoolean("gamep_disable_cache", gameData.disableShaderCache)
	    settings?.putBoolean("gamep_internet_enabled", gameData.internetEnabled)
        }

        settings?.commit()
    }

    @Serializable
    class CustomGameData ( var titleId: String?, var version: String?, var title: String?) {
        // General Values
        var playCount : Int = 0
        var isFavorite : Boolean = false
        // General Settings
        var customSettings : Boolean = false
        // System
        var isDocked : Boolean = true
        var systemLanguage : Int = 1
        var systemRegion : Int = -1
	var internetEnabled : Boolean = true
        // Display
        var forceTripleBuffering : Boolean = true
        var disableFrameThrottling : Boolean = false
        var maxRefreshRate : Boolean = false
        var aspectRatio : Int = 0
        var orientation : Int = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
	var disableShaderCache : Boolean = false
        // GPU
        var gpuDriver : String = PreferenceSettings.SYSTEM_GPU_DRIVER
        var executorSlotCountScale : Int = 4
        var executorFlushThreshold : Int = 256
        var useDirectMemoryImport : Boolean = false
        var forceMaxGpuClocks : Boolean = false
        // Hacks
        var enableFastGpuReadbackHack : Boolean = false
        // Audio
        var isAudioOutputDisabled : Boolean = false
        // Debug
        var validationLayer : Boolean = false
    }
}
