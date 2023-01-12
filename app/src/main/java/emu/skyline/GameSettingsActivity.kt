/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline

import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.*
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.view.*
import androidx.preference.CheckBoxPreference
import androidx.preference.Preference
import androidx.preference.PreferenceCategory
import androidx.preference.PreferenceFragmentCompat
import emu.skyline.data.AppItem
import emu.skyline.databinding.GameSettingsActivityBinding
import emu.skyline.preference.IntegerListPreference
import emu.skyline.utils.GpuDriverHelper
import emu.skyline.utils.WindowInsetsHelper
import org.json.JSONException
import org.json.JSONObject
import java.io.File
import java.io.FileWriter
import java.io.PrintWriter
import java.nio.charset.Charset

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

            setCategoriesVisibility(areCustomSettingsEnabled.isChecked)
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
            val path = "/json/microsoft.json"

            val json = JSONObject()

            try {
                json.put("name", "Microsoft")
                json.put("employees", 182268)
                json.put("offices", listOf("California", "Washington", "Virginia"))
            } catch (e: JSONException) {
                e.printStackTrace()
            }

            val filename = "customGameSetting"
            val fileContents = "Hello world!"
            context?.openFileOutput(filename, 0).use {
                if (it != null) {
                    it.write(fileContents.toByteArray())
                }
            }

            var files: Array<String>? = context?.fileList() ?: null


            try {
                PrintWriter(FileWriter(context?.filesDir, Charset.defaultCharset()))
                    .use { it.write(json.toString()) }
            } catch (e: Exception) {
                Log.i("Error al escribir archivo ", e.toString())
                e.printStackTrace()
            }
            /*findPreference<Preference>("gamep_category_emulator")?.isVisible = visible
            findPreference<Preference>("gamep_category_system")?.isVisible = visible
            findPreference<Preference>("gamep_category_presentation")?.isVisible = visible
            findPreference<Preference>("gamep_category_hacks")?.isVisible = visible
            findPreference<Preference>("gamep_category_audio")?.isVisible = visible
            findPreference<Preference>("gamep_category_debug")?.isVisible = visible*/
            super.onStop()
        }

        override fun onPreferenceTreeClick(preference : Preference) : Boolean {
            //Log.i("onPreferenceTreeClick Event", "Se cambio la configuración")
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
