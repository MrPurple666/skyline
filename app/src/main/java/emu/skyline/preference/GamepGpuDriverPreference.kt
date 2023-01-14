/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline.preference

import android.content.Context
import android.content.Intent
import android.util.AttributeSet
import androidx.activity.ComponentActivity
import androidx.activity.result.contract.ActivityResultContracts
import androidx.preference.Preference
import androidx.preference.Preference.SummaryProvider
import androidx.preference.R
import emu.skyline.utils.GpuDriverHelper
import emu.skyline.utils.PreferenceSettings
import emu.skyline.R as SkylineR

/**
 * This preference is used to launch [GamepGpuDriverActivity] using a preference
 */
class GamepGpuDriverPreference @JvmOverloads constructor(context : Context, attrs : AttributeSet? = null, defStyleAttr : Int = R.attr.preferenceStyle) : Preference(context, attrs, defStyleAttr) {
    private val driverCallback = (context as ComponentActivity).registerForActivityResult(ActivityResultContracts.StartActivityForResult()) {
        notifyChanged()
    }

    init {
        val supportsCustomDriverLoading = GpuDriverHelper.supportsCustomDriverLoading()
        if (supportsCustomDriverLoading) {
            summaryProvider = SummaryProvider<GamepGpuDriverPreference> {
                sharedPreferences?.getString(key, PreferenceSettings.SYSTEM_GPU_DRIVER)?.let {
                    var driver = it
                    if (it == PreferenceSettings.SYSTEM_GPU_DRIVER)
                        driver = context.getString(SkylineR.string.system_driver)

                    context.getString(SkylineR.string.gpu_driver_config_desc, driver)
                }
            }
        } else {
            isEnabled = false
            summaryProvider = SummaryProvider<GamepGpuDriverPreference> {
                context.getString(SkylineR.string.gpu_driver_config_desc_unsupported)
            }
        }
    }

    fun setValue (driver : String) {
        summaryProvider = SummaryProvider<GamepGpuDriverPreference> {
            context.getString(SkylineR.string.gpu_driver_config_desc, driver)
        }
        var settings = sharedPreferences?.edit()
        settings?.putString(key, driver)
        settings?.commit()
    }

    /**
     * This launches [GamepGpuDriverActivity] on click to manage driver packages
     */
    override fun onClick() = driverCallback.launch(Intent(context, GamepGpuDriverActivity::class.java))
}
