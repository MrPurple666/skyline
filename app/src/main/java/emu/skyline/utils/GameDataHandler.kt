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

    @Serializable
    class CustomGameData ( var titleId: String?, var version: String?, var title: String?) {
        // Identification
        //var titleId : String = ""
        //var version: String = ""
        // General Values
        var playCount : Int = 0
        var isFavorite : Boolean = false
        // General Settings
        var customSettings : Boolean = false
        // System
        var isDocked : Boolean = true
        var systemLanguage : Int = 1
        var systemRegion : Int = -1
        // Display
        var forceTripleBuffering : Boolean = true
        var disableFrameThrottling : Boolean = false
        var maxRefreshRate : Boolean = false
        var aspectRatio : Int = 0
        var orientation : Int = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
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
