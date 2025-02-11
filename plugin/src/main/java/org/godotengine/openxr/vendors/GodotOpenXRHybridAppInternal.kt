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
import android.content.ComponentName
import android.content.Intent
import android.util.Log
import android.view.View;
import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.plugin.UsedByGodot
import org.godotengine.godot.utils.isNativeXRDevice

/**
 * A plugin used internally for hybrid apps.
 */
class GodotOpenXRHybridAppInternal(godot: Godot?) : GodotPlugin(godot) {
	companion object {
		protected val TAG = GodotOpenXRHybridAppInternal::class.java.simpleName

		private const val IMMERSIVE_ACTIVITY = "com.godot.game.GodotApp"
		private const val PANEL_ACTIVITY = "org.godotengine.openxr.vendors.GodotPanelApp"

		private const val IMMERSIVE_MODE = 0
		private const val PANEL_MODE = 1

		private const val INTENT_EXTRA_DATA = "godot_openxr_vendors_data"
	}

	private var hybridMode: Int = IMMERSIVE_MODE
	private var hybridLaunchData: String = ""

	override fun getPluginName() = "GodotOpenXRHybridAppInternal"

	override fun onMainCreate(activity: Activity): View? {
		if (activity::class.qualifiedName == PANEL_ACTIVITY) {
			hybridMode = PANEL_MODE
		} else {
			hybridMode = IMMERSIVE_MODE
		}

		hybridLaunchData = activity.intent.getStringExtra(INTENT_EXTRA_DATA) ?: ""

		return null
	}

	@UsedByGodot
	fun hybridAppSwitchTo(mode: Int, data: String = ""): Boolean {
		if (hybridMode == mode) return false

		val context = getActivity() ?: return false

		if (!isNativeXRDevice(context)) return false

		val activityName = if (mode == IMMERSIVE_MODE) IMMERSIVE_ACTIVITY else PANEL_ACTIVITY
		val newInstance = Intent()
			.setComponent(ComponentName(context, activityName))
			.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
			.putExtra(INTENT_EXTRA_DATA, data)

		val godot = godot
		if (godot != null) {
			godot.destroyAndKillProcess {
				context.startActivity(newInstance)
			}
		} else {
			context.startActivity(newInstance)
			context.finish()
		}

		return true
	}

	@UsedByGodot
	fun getHybridAppLaunchData(): String {
		return hybridLaunchData
	}
}
