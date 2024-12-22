/**************************************************************************/
/*  GodotOpenXRHybridAppInternal.kt                                       */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

package org.godotengine.openxr.vendors

import android.app.Activity
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.util.Log
import android.view.View
import org.godotengine.godot.Godot
import org.godotengine.godot.GodotHost
import org.godotengine.godot.GodotLib
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.plugin.UsedByGodot
import org.godotengine.godot.utils.isHorizonOSDevice
import org.godotengine.godot.utils.isNativeXRDevice

/**
 * A plugin used internally for hybrid apps.
 */
class GodotOpenXRHybridAppInternal(godot: Godot?) : GodotPlugin(godot) {
	internal companion object {
		private val TAG = GodotOpenXRHybridAppInternal::class.java.simpleName

		private const val EXTRA_HYBRID_LAUNCH_DATA = "godot_openxr_vendors_hybrid_launch_data"

		// TODO: These constants were copied from Godot core for compatibility with Godot 4.4.
		// They will be included in Godot 4.5, at which point we can remove them.
		const val HYBRID_APP_PANEL_FEATURE = "godot_openxr_panel_app"
		const val HYBRID_APP_PANEL_CATEGORY = "org.godotengine.xr.hybrid.PANEL"
		const val HYBRID_APP_IMMERSIVE_CATEGORY = "org.godotengine.xr.hybrid.IMMERSIVE"
		const val EXTRA_COMMAND_LINE_PARAMS = "command_line_params"

		private fun getHybridMode(activity: Activity?): HybridMode {
			if (activity !is GodotHost) {
				return HybridMode.NONE
			}

			if (activity.godot.isProjectManagerHint() || activity.godot.isEditorHint()) {
				return HybridMode.NONE
			}

			// Check if hybrid is enabled
			val hybridEnabled = GodotLib.getGlobal("xr/hybrid_app/enabled").toBoolean()
			if (!hybridEnabled) {
				return HybridMode.NONE
			}

			if (activity.supportsFeature(HYBRID_APP_PANEL_FEATURE)) {
				return HybridMode.PANEL
			}

			return HybridMode.IMMERSIVE
		}

		/**
		 * Returns true if running on an Android XR device.
		 * TODO: Remove when this is included in the Godot Android library.
		 */
		private fun isAndroidXRDevice(context: Context): Boolean {
			return context.packageManager.hasSystemFeature("android.software.xr.immersive")
		}

		/**
		 * Update the given command line for an hybrid launch:
		 *  - remove any arguments that may conflict
		 *  - set up the xr mode based on the hybrid launch mode
		 */
		internal fun updateCommandLineForHybridLaunch(
			launchMode: HybridMode,
			originalCommandLine: List<String> = emptyList()
		): MutableList<String> {
			val newCmdline = mutableListOf<String>()
			if (launchMode == HybridMode.NONE) {
				return newCmdline
			}

			// Remove any existing command-line arguments setting the XR mode.
			var skipNext = false
			for (arg in originalCommandLine) {
				if (skipNext) {
					skipNext = false
					continue
				}

				when (arg) {
					"--xr_mode_regular", "--xr_mode_openxr" -> continue
					"--xr-mode" -> {
						skipNext = true
						continue
					}
					else -> newCmdline.add(arg)
				}
			}

			if (launchMode == HybridMode.PANEL) {
				// Add new arguments to force XR to be turned off.
				newCmdline.addAll(listOf("--xr_mode_regular", "--xr-mode", "off"))
			} else {
				// Add new arguments to force XR to be turned on.
				newCmdline.addAll(listOf("--xr_mode_openxr", "--xr-mode", "on"))
			}

			return newCmdline
		}
	}

	/**
	 * Should match OpenXRHybridApp#HybridMode.
	 */
	internal enum class HybridMode(private val nativeValue: Int) {
		NONE( -1),
		IMMERSIVE(0),
		PANEL(1);

		companion object {
			fun fromNative(nativeValue: Int): HybridMode {
				for (mode in HybridMode.entries) {
					if (mode.nativeValue == nativeValue) {
						return mode
					}
				}
				return NONE
			}
		}
	}

	private val hybridMode: HybridMode by lazy { getHybridMode(activity) }
	private var hybridLaunchData: String = ""

	override fun getPluginName() = "GodotOpenXRHybridAppInternal"

	override fun onMainCreate(activity: Activity): View? {
		hybridLaunchData = activity.intent.getStringExtra(EXTRA_HYBRID_LAUNCH_DATA) ?: ""
		return null
	}

	@UsedByGodot
	private fun hybridAppSwitchTo(modeValue: Int, data: String = ""): Boolean {
		val mode = HybridMode.fromNative(modeValue)
		if (hybridMode == mode) return false

		val context = activity ?: return false

		if (!isNativeXRDevice(context) && !isAndroidXRDevice(context)) return false

		val hybridCategory = if (mode == HybridMode.IMMERSIVE) HYBRID_APP_IMMERSIVE_CATEGORY else HYBRID_APP_PANEL_CATEGORY
		val hybridLaunchIntent = Intent().apply {
			addCategory(hybridCategory)
			setPackage(context.packageName)
			addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)

			// Pass the extras from the current activity's intent
			val originalExtras = context.intent.extras
			if (originalExtras != null && !originalExtras.isEmpty) {
				putExtras(originalExtras)
			}

			// Override the hybrid launch data
			putExtra(EXTRA_HYBRID_LAUNCH_DATA, data)

			// Update the command line parameters
			val previousParams = context.intent.getStringArrayExtra(EXTRA_COMMAND_LINE_PARAMS)
			val updatedParams = updateCommandLineForHybridLaunch(
				mode, previousParams?.asList()
					?: emptyList()
			)
			putExtra(EXTRA_COMMAND_LINE_PARAMS, updatedParams.toTypedArray())
		}

		// Resolve the component needed for the launch
		val hybridLaunchComponentName = hybridLaunchIntent.resolveActivity(context.packageManager)
		if (hybridLaunchComponentName == null) {
			Log.e(TAG, "Unable to resolve hybrid mode launch intent $hybridLaunchIntent")
			return false
		} else {
			Log.d(TAG, "Resolved hybrid launch component: $hybridLaunchComponentName")
			hybridLaunchIntent.setComponent(hybridLaunchComponentName)
		}

		val launchIntent = if (mode == HybridMode.PANEL && isHorizonOSDevice(context)) {
			// HorizonOS has a different launch flow for panel mode.
			// Wrap the created Intent in a PendingIntent object
			val pendingPanelIntent =
				PendingIntent.getActivity(
					context,
					0,
					hybridLaunchIntent,
					PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)

			// Create and send the Intent to launch the Home environment, providing the
			// PendingIntent object as extra parameters
			val homeIntent =
				Intent(Intent.ACTION_MAIN)
					.addCategory(Intent.CATEGORY_HOME)
					.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
					.putExtra("extra_launch_in_home_pending_intent", pendingPanelIntent)

			homeIntent
		} else {
			hybridLaunchIntent
		}

		val godot = godot
		if (godot != null) {
			godot.destroyAndKillProcess {
				context.startActivity(launchIntent)
			}
		} else {
			context.startActivity(launchIntent)
			context.finish()
		}

		return true
	}

	@UsedByGodot
	private fun getHybridAppLaunchData(): String {
		return hybridLaunchData
	}
}
