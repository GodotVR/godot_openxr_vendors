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
import android.content.ComponentName
import android.content.Intent
import android.util.Log
import android.view.View
import org.godotengine.godot.Godot
import org.godotengine.godot.GodotHost
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.plugin.UsedByGodot
import org.godotengine.openxr.vendors.utils.*

/**
 * A plugin used internally for hybrid apps.
 */
class GodotOpenXRHybridAppInternal(godot: Godot?) : GodotPlugin(godot) {
	internal companion object {
		private val TAG = GodotOpenXRHybridAppInternal::class.java.simpleName

		private fun getHybridMode(activity: Activity?): HybridMode {
			if (activity !is GodotHost) {
				return HybridMode.NONE
			}

			if (activity.godot?.isProjectManagerHint() == true || activity.godot?.isEditorHint() == true) {
				return HybridMode.NONE
			}

			// Check if hybrid is enabled
			if (!isHybridAppEnabled()) {
				return HybridMode.NONE
			}

			if (supportsFeature(activity, HYBRID_APP_PANEL_FEATURE)) {
				return HybridMode.PANEL
			}

			if (supportsFeature(activity, HYBRID_APP_SPATIAL_FEATURE)) {
				return HybridMode.SPATIAL
			}

			return HybridMode.NONE
		}

		@JvmStatic
		internal fun supportsFeature(activity: Activity?, featureTag: String): Boolean {
			return when (featureTag) {
				HYBRID_APP_PANEL_FEATURE, HYBRID_APP_SPATIAL_FEATURE -> {
					val hostActivity = activity ?: return false
					if (hostActivity is GodotHost && hostActivity.supportsFeature(featureTag)) {
						return true
					}

					val hybridAppCategory = if (featureTag == HYBRID_APP_PANEL_FEATURE) {
						HYBRID_APP_PANEL_CATEGORY
					} else {
						HYBRID_APP_SPATIAL_CATEGORY
					}

					// Validate the host activity has the matching category.
					val pm = hostActivity.packageManager ?: return false
					val resolveIntent = Intent().setPackage(hostActivity.packageName).addCategory(hybridAppCategory)
                    val resolvedComponentName = resolveIntent.resolveActivity(pm) ?: return false

                    var hostActivityComponentName = hostActivity.componentName
					val hostActivityInfo = pm.getActivityInfo(hostActivityComponentName, 0)
					if (hostActivityInfo.targetActivity != null) {
						hostActivityComponentName = ComponentName(hostActivityComponentName.packageName, hostActivityInfo.targetActivity)
					}
                    resolvedComponentName == hostActivityComponentName
				}

				else -> {
					val host = activity as? GodotHost ?: return false
					val godot = host.godot ?: return false
					GodotOpenXR.supportsFeature(godot, featureTag)
				}
			}
		}

		@JvmStatic
		internal fun hybridAppSwitchTo(activity: Activity, newMode: HybridMode, data: String = ""): Boolean {
			if (newMode == HybridMode.NONE) {
				return false
			}

			val currentMode = getHybridMode(activity)
			if (currentMode == newMode) {
				return false
			}

			val hybridCategory = if (newMode == HybridMode.SPATIAL) HYBRID_APP_SPATIAL_CATEGORY else HYBRID_APP_PANEL_CATEGORY
			val hybridLaunchIntent = Intent().apply {
				addCategory(hybridCategory)
				setPackage(activity.packageName)
				addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)

				// Pass the extras from the current activity's intent
				val originalExtras = activity.intent.extras
				if (originalExtras != null && !originalExtras.isEmpty) {
					putExtras(originalExtras)
				}

				// Update the command line parameters
				val previousParams = activity.intent.getStringArrayExtra(EXTRA_COMMAND_LINE_PARAMS)
				val launchMode = if (newMode == HybridMode.PANEL) {
					HybridLaunchMode.PANEL
				} else {
					HybridLaunchMode.SPATIAL
				}
				val updatedParams = updateCommandLineForHybridLaunch(
					launchMode,
					previousParams?.asList() ?: emptyList(),
					data,
					true
				)
				putExtra(EXTRA_COMMAND_LINE_PARAMS, updatedParams.toTypedArray())
			}

			// Resolve the component needed for the launch
			val hybridLaunchComponentName = hybridLaunchIntent.resolveActivity(activity.packageManager)
			if (hybridLaunchComponentName == null) {
				Log.e(TAG, "Unable to resolve hybrid mode launch intent $hybridLaunchIntent")
				return false
			} else {
				Log.d(TAG, "Resolved hybrid launch component: $hybridLaunchComponentName")
				hybridLaunchIntent.setComponent(hybridLaunchComponentName)
			}

			val launchIntent = if (newMode == HybridMode.PANEL && isHorizonOSDevice(activity)) {
				// HorizonOS has a different launch flow for panel mode.
				// Wrap the created Intent in a PendingIntent object
				val pendingPanelIntent =
					PendingIntent.getActivity(
						activity,
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

			val godot = (activity as? GodotHost)?.godot
			if (godot != null) {
				godot.destroyAndKillProcess {
					activity.startActivity(launchIntent)
				}
			} else {
				activity.startActivity(launchIntent)
				activity.finish()
			}

			return true
		}

		/**
		 * Check the hybrid launch mode, and switch to the right mode if set to AUTO.
		 */
		@JvmStatic
		internal fun triggerAutoLaunchIfNeeded(activity: Activity?) {
			if (activity == null) {
				return
			}

			if (!isHybridAppEnabled()) {
				return
			}

			if (activity is GodotHost) {
				if (activity.godot?.isProjectManagerHint() == true || activity.godot?.isEditorHint() == true) {
					return
				}
			}

			// If the application explicitly switch to this activity, abort.
			if (hasExplicitHybridSwitchFlag(activity.intent)) {
				return
			}

			// Retrieve the hybrid launch mode.
			val hybridLaunchMode = getHybridAppLaunchMode()
			if (hybridLaunchMode != HybridLaunchMode.AUTO) {
				return
			}

			// If the launch mode is AUTO:
			// - switch to the SPATIAL mode if we're running on a XR device
			// - switch to the PANEL mode if we're not running on a XR device.
			val autoMode = if (isNativeXRDevice(activity)) {
				HybridMode.SPATIAL
			} else {
				HybridMode.PANEL
			}
			hybridAppSwitchTo(activity, autoMode)
		}
	}

	private val hybridMode: HybridMode by lazy { getHybridMode(activity) }
	private var hybridLaunchData: String = ""

	override fun getPluginName() = "GodotOpenXRHybridAppInternal"

	override fun onMainCreate(activity: Activity?): View? {
		triggerAutoLaunchIfNeeded(activity)
		hybridLaunchData = retrieveHybridData(activity?.intent)
		return null
	}

	@UsedByGodot
	private fun hybridAppSwitchTo(modeValue: Int, data: String = ""): Boolean {
		val hostActivity = activity ?: return false
		if (!isNativeXRDevice(hostActivity)) {
			return false
		}
		val newMode = HybridMode.fromNative(modeValue)
		return hybridAppSwitchTo(hostActivity, newMode, data)
	}

	@UsedByGodot
	private fun getHybridAppLaunchData(): String {
		return hybridLaunchData
	}

	override fun supportsFeature(featureTag: String): Boolean {
		return supportsFeature(activity, featureTag) || super.supportsFeature(featureTag)
	}
}
