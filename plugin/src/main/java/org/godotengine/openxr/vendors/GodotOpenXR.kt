/**************************************************************************/
/*  GodotOpenXR.kt                                                        */
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

import android.util.Log
import org.godotengine.godot.Godot
import org.godotengine.godot.GodotLib
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.utils.PermissionsUtil

/**
 * Base class for the Godot OpenXR plugin.
 */
abstract class GodotOpenXR(godot: Godot?) : GodotPlugin(godot) {
	companion object {
		protected val TAG = GodotOpenXR::class.java.simpleName

		init {
			try {
				Log.v(TAG, "Loading openxr_loader library")
				System.loadLibrary("openxr_loader")
			} catch (e: UnsatisfiedLinkError) {
				Log.e(TAG, "Unable to load openxr_loader shared library")
			}

			try {
				Log.v(TAG, "Loading godotopenxrvendors library")
				System.loadLibrary("godotopenxrvendors")
			} catch (e: UnsatisfiedLinkError) {
				Log.e(TAG, "Unable to load godotopenxrvendors shared library")
			}
		}
	}

	override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/godotopenxrvendors/plugin.gdextension")

	override fun onGodotSetupCompleted() {
		super.onGodotSetupCompleted()

		// Check if automatic permissions request is enabled.
		val automaticallyRequestPermissionsSetting = GodotLib.getGlobal("xr/openxr/extensions/automatically_request_runtime_permissions")
		// We only request permissions when the project setting is enabled.
		// If the project setting is not defined, we fall-back to the default behavior which is
		// to automatically request permissions.
		val requestPermissionsEnabled = automaticallyRequestPermissionsSetting.isNullOrEmpty() ||
				automaticallyRequestPermissionsSetting.toBoolean()
		if (requestPermissionsEnabled) {
			val permissionsToRequest = getPluginPermissionsToEnable()
			if (permissionsToRequest.isNotEmpty()) {
				Log.v(TAG, "Performing automatic request of runtime permissions")
				PermissionsUtil.requestPermissions(activity, permissionsToRequest)
			}
		}
	}

	protected open fun getPluginPermissionsToEnable(): MutableList<String> {
		return mutableListOf()
	}
}
