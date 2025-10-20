/**************************************************************************/
/*  HybridAppUtils.kt                                                     */
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

/**
 * Contains utility methods and constants for hybrid apps.
 */
@file:JvmName("HybridAppUtils")

package org.godotengine.openxr.vendors.utils

import android.util.Log
import org.godotengine.godot.GodotLib

private const val TAG = "HybridAppUtils"

/**
 * Should match OpenXRHybridApp#HybridMode.
 */
enum class HybridMode(private val nativeValue: Int) {
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

const val HYBRID_APP_FEATURE = "godot_openxr_hybrid_app"
const val HYBRID_APP_PANEL_FEATURE = "godot_openxr_panel_app"
const val HYBRID_APP_PANEL_CATEGORY = "org.godotengine.xr.hybrid.PANEL"
const val HYBRID_APP_IMMERSIVE_CATEGORY = "org.godotengine.xr.hybrid.IMMERSIVE"

fun isHybridAppEnabled() = GodotLib.getGlobal("xr/hybrid_app/enabled").toBoolean()

fun getHybridAppLaunchMode(): HybridMode {
	if (!isHybridAppEnabled()) {
		return HybridMode.NONE
	}

	try {
		val launchModeValue = GodotLib.getGlobal("xr/hybrid_app/launch_mode").toInt()
		return HybridMode.fromNative(launchModeValue)
	} catch (e: Exception) {
		Log.w(TAG, "Unable to retrieve 'xr/hybrid_app/launch_mode' project setting", e)
		return HybridMode.NONE
	}
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
