/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline

import android.annotation.SuppressLint
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.*
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.view.*
import androidx.preference.*
import emu.skyline.data.AppItem
import emu.skyline.databinding.GameSettingsActivityBinding
import emu.skyline.di.getSettings
import emu.skyline.preference.GamepGpuDriverPreference
import emu.skyline.preference.IntegerListPreference
import emu.skyline.utils.GpuDriverHelper
import emu.skyline.utils.WindowInsetsHelper
import emu.skyline.utils.GameDataHandler
import emu.skyline.utils.PreferenceSettings

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
class GameSettingsActivity : AppCompatActivity() {

    val binding by lazy { GameSettingsActivityBinding.inflate(layoutInflater) }

    /**
     * This is the instance of [PreferenceFragment] that is shown inside [R.id.settings]
     */
    private val preferenceFragment = PreferenceFragment()

    private var appItem : AppItem? = null

    /**
     * This initializes all of the elements in the activity and displays the settings fragment
     */
    override fun onCreate(savedInstanceState : Bundle?) {
        super.onCreate(savedInstanceState)

        appItem = intent.extras?.get("item") as AppItem

        setContentView(binding.root)
        WindowCompat.setDecorFitsSystemWindows(window, false)
        WindowInsetsHelper.applyToActivity(binding.root)

        setSupportActionBar(binding.titlebar.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        var layoutDone = false // Tracks if the layout is complete to avoid retrieving invalid attributes
        binding.coordinatorLayout.viewTreeObserver.addOnTouchModeChangeListener { isTouchMode ->
            val layoutUpdate = {
                val params = binding.gameSettings.layoutParams as CoordinatorLayout.LayoutParams
                if (!isTouchMode) {
                    binding.titlebar.appBarLayout.setExpanded(true)
                    params.height = binding.coordinatorLayout.height - binding.titlebar.toolbar.height
                } else {
                    params.height = CoordinatorLayout.LayoutParams.MATCH_PARENT
                }

                binding.gameSettings.layoutParams = params
                binding.gameSettings.requestLayout()
            }

            if (!layoutDone) {
                binding.coordinatorLayout.viewTreeObserver.addOnGlobalLayoutListener(object : ViewTreeObserver.OnGlobalLayoutListener {
                    override fun onGlobalLayout() {
                        // We need to wait till the layout is done to get the correct height of the toolbar
                        binding.coordinatorLayout.viewTreeObserver.removeOnGlobalLayoutListener(this)
                        layoutUpdate()
                        layoutDone = true
                    }
                })
            } else {
                layoutUpdate()
            }
        }

        supportFragmentManager
            .beginTransaction()
            .replace(R.id.game_settings, preferenceFragment)
            .commit()
    }

    /**
     * This fragment is used to display all of the preferences
     */
    class PreferenceFragment : PreferenceFragmentCompat() {
        var preferenceSettings : PreferenceSettings? = context?.getSettings()

        companion object {
            private const val DIALOG_FRAGMENT_TAG = "androidx.preference.PreferenceFragment.DIALOG"
            private var appItem : AppItem? = null
        }

        override fun onViewCreated(view : View, savedInstanceState : Bundle?) {
            super.onViewCreated(view, savedInstanceState)
            val recyclerView = view.findViewById<View>(R.id.recycler_view)
            WindowInsetsHelper.setPadding(recyclerView, bottom = true)
        }

        /**
         * This constructs the preferences from [R.xml.game_preferences]
         */
        override fun onCreatePreferences(savedInstanceState : Bundle?, rootKey : String?) {
            appItem = (context as GameSettingsActivity).appItem as AppItem

            setPreferencesFromResource(R.xml.game_preferences, rootKey)

            // Uncheck `disable_frame_throttling` if `force_triple_buffering` gets disabled
            val areCustomSettingsEnabled = findPreference<CheckBoxPreference>("gamep_custom_settings")!!

            areCustomSettingsEnabled.onPreferenceChangeListener = Preference.OnPreferenceChangeListener { preference, newValue ->
                Log.i( "test","Pref " + preference.key + " changed to " + newValue.toString())
                setCategoriesVisibility(newValue as Boolean)
                true
            }

            findPreference<PreferenceCategory>("gamep_category_game")?.addPreference(Preference(context!!).apply {
                title = (appItem as AppItem).title
                summary = (appItem as AppItem).version
            })

            if (!GpuDriverHelper.supportsForceMaxGpuClocks()) {
                val forceMaxGpuClocksPref = findPreference<CheckBoxPreference>("gamep_force_max_gpu_clocks")!!
                forceMaxGpuClocksPref.isSelectable = false
                forceMaxGpuClocksPref.isChecked = false
                forceMaxGpuClocksPref.summary = context!!.getString(R.string.force_max_gpu_clocks_desc_unsupported)
            }

            loadGameCustomSettings()

            setCategoriesVisibility(areCustomSettingsEnabled.isChecked)
        }

        private fun loadGameCustomSettings() {
            var gameDataHandler = GameDataHandler()
            val gameData = gameDataHandler.getGameData(context, (appItem as AppItem))

            findPreference<CheckBoxPreference>("gamep_custom_settings")!!.isChecked = gameData.customSettings
            findPreference<GamepGpuDriverPreference>("gamep_gpu_driver")!!.setValue(gameData.gpuDriver)
            findPreference<CheckBoxPreference>("gamep_is_docked")!!.isChecked = gameData.isDocked
            findPreference<IntegerListPreference>("gamep_system_language")!!.value = gameData.systemLanguage
            findPreference<IntegerListPreference>("gamep_system_region")!!.value = gameData.systemRegion
            findPreference<CheckBoxPreference>("gamep_force_triple_buffering")!!.isChecked = gameData.forceTripleBuffering
            findPreference<CheckBoxPreference>("gamep_disable_frame_throttling")!!.isChecked = gameData.disableFrameThrottling
            findPreference<CheckBoxPreference>("gamep_max_refresh_rate")!!.isChecked = gameData.maxRefreshRate
            findPreference<IntegerListPreference>("gamep_aspect_ratio")!!.value = gameData.aspectRatio
            findPreference<IntegerListPreference>("gamep_orientation")!!.value = gameData.orientation
            findPreference<SeekBarPreference>("gamep_executor_slot_count_scale")!!.value = gameData.executorSlotCountScale
            findPreference<SeekBarPreference>("gamep_executor_flush_threshold")!!.value = gameData.executorFlushThreshold
            findPreference<CheckBoxPreference>("gamep_use_direct_memory_import")!!.isChecked = gameData.useDirectMemoryImport
            findPreference<CheckBoxPreference>("gamep_force_max_gpu_clocks")!!.isChecked = gameData.forceMaxGpuClocks
            findPreference<CheckBoxPreference>("gamep_enable_fast_gpu_readback_hack")!!.isChecked = gameData.enableFastGpuReadbackHack
            findPreference<CheckBoxPreference>("gamep_is_audio_output_disabled")!!.isChecked = gameData.isAudioOutputDisabled
            findPreference<CheckBoxPreference>("gamep_validation_layer")!!.isChecked = gameData.validationLayer
            findPreference<CheckBoxPreference>("gamep_shader_cache")!!.isChecked = gameData.disableShaderCache
	    findPreference<CheckBoxPreference>("gamep_internet_enabled")!!.isChecked = gameData.internetEnabled
	    findPreference<CheckBoxPreference>("gamep_free_guest_texture_memory")!!.isChecked = gameData.freeGuestTextureMemory
	    findPreference<CheckBoxPreference>("gamep_disable_subgroup_shuffle")!!.isChecked = gameData.disableSubgroupShuffle
	    findPreference<CheckBoxPreference>("gamep_enable_fast_readback_writes")!!.isChecked = gameData.enableFastReadbackWrites
        }

        override fun onDisplayPreferenceDialog(preference : Preference) {
            if (preference is IntegerListPreference) {
                // Check if dialog is already showing
                if (parentFragmentManager.findFragmentByTag(DIALOG_FRAGMENT_TAG) != null)
                    return

                val dialogFragment = IntegerListPreference.IntegerListPreferenceDialogFragmentCompat.newInstance(preference.getKey())
                @Suppress("DEPRECATION")
                dialogFragment.setTargetFragment(this, 0) // androidx.preference.PreferenceDialogFragmentCompat depends on the target fragment being set correctly even though it's deprecated
                dialogFragment.show(parentFragmentManager, DIALOG_FRAGMENT_TAG)
            } else {
                super.onDisplayPreferenceDialog(preference)
            }
        }

        override fun onStop() {
            Log.i("onStop Event", "Se termina la edicion")
            var gameDataHandler = GameDataHandler()
            var gameData = gameDataHandler.getGameData(context, appItem)

            gameData.customSettings = context?.let { PreferenceSettings(it).gamepCustomSettings }!!
            gameData.gpuDriver = context?.let { PreferenceSettings(it).gamepGpuDriver }!!
            gameData.isDocked = context?.let { PreferenceSettings(it).gamepIsDocked }!!
            gameData.systemLanguage = context?.let { PreferenceSettings(it).gamepSystemLanguage }!!
            gameData.systemRegion = context?.let { PreferenceSettings(it).gamepSystemRegion }!!
            gameData.forceTripleBuffering = context?.let { PreferenceSettings(it).gamepForceTripleBuffering }!!
            gameData.disableFrameThrottling = context?.let { PreferenceSettings(it).gamepDisableFrameThrottling }!!
            gameData.maxRefreshRate = context?.let { PreferenceSettings(it).gamepMaxRefreshRate }!!
            gameData.aspectRatio = context?.let { PreferenceSettings(it).gamepAspectRatio }!!
            gameData.orientation = context?.let { PreferenceSettings(it).gamepOrientation }!!
            gameData.executorSlotCountScale = context?.let { PreferenceSettings(it).gamepExecutorSlotCountScale }!!
            gameData.executorFlushThreshold = context?.let { PreferenceSettings(it).gamepExecutorFlushThreshold }!!
            gameData.useDirectMemoryImport = context?.let { PreferenceSettings(it).gamepUseDirectMemoryImport }!!
            gameData.forceMaxGpuClocks = context?.let { PreferenceSettings(it).gamepForceMaxGpuClocks }!!
            gameData.enableFastGpuReadbackHack = context?.let { PreferenceSettings(it).gamepEnableFastGpuReadbackHack }!!
            gameData.isAudioOutputDisabled = context?.let { PreferenceSettings(it).gamepIsAudioOutputDisabled }!!
            gameData.validationLayer = context?.let { PreferenceSettings(it).gamepValidationLayer }!!
            gameData.disableShaderCache = context?.let { PreferenceSettings(it).gamepDisableShaderCache }!!
	    gameData.internetEnabled = context?.let { PreferenceSettings(it).gamepInternetEnabled }!!
	    gameData.freeGuestTextureMemory = context?.let { PreferenceSettings(it).gamepFreeGuestTextureMemory }!!
	    gameData.disableSubgroupShuffle = context?.let { PreferenceSettings(it).gamepDisableSubgroupShuffle }!!
	    gameData.enableFastReadbackWrites = context?.let { PreferenceSettings(it).gamepEnableFastReadbackWrites }!!

            gameDataHandler.saveGameData(context, gameData)

            super.onStop()
        }

        override fun onPreferenceTreeClick(preference : Preference) : Boolean {
            return super.onPreferenceTreeClick(preference)
        }

        private fun setCategoriesVisibility (visible : Boolean) {
            findPreference<Preference>("gamep_category_emulator")?.isVisible = visible
            findPreference<Preference>("gamep_category_system")?.isVisible = visible
            findPreference<Preference>("gamep_category_presentation")?.isVisible = visible
            findPreference<Preference>("gamep_category_hacks")?.isVisible = visible
            findPreference<Preference>("gamep_category_audio")?.isVisible = visible
            findPreference<Preference>("gamep_category_debug")?.isVisible = visible
        }
    }

    /**
     * This handles on calling [onBackPressed] when [KeyEvent.KEYCODE_BUTTON_B] is lifted
     */
    override fun onKeyUp(keyCode : Int, event : KeyEvent?) : Boolean {
        if (keyCode == KeyEvent.KEYCODE_BUTTON_B) {
            onBackPressed()
            return true
        }

        return super.onKeyUp(keyCode, event)
    }

    override fun finish() {
        setResult(RESULT_OK)
        super.finish()
    }
}
