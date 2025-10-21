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
import org.godotengine.openxr.vendors.utils.*

/**
 * Godot OpenXR plugin.
 *
 * When using OpenXR for your application on Android make sure that
 * the IMMERSIVE_HMD category is added to your activities intent-filter.
 *
 * <intent-filter>
 * <action android:name="android.intent.action.MAIN"></action>
 * <category android:name="android.intent.category.LAUNCHER"></category>
 * <category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD"></category>
 * </intent-filter>
 *
 * more details can be found here:
 * https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category
 *
 */
class GodotOpenXR(godot: Godot?) : GodotPlugin(godot) {
	companion object {
		private val TAG = GodotOpenXR::class.java.simpleName

		init {
			try {
				Log.v(TAG, "Loading openxr_loader library")
				System.loadLibrary("openxr_loader")

				Log.v(TAG, "Loading godotopenxrvendors library")
				System.loadLibrary("godotopenxrvendors")
			} catch (e: UnsatisfiedLinkError) {
				Log.e(TAG, "Unable to load native libraries")
			}
		}
	}

	override fun getPluginName() = "GodotOpenXR"

	override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/godotopenxrvendors/plugin.gdextension")

	override fun onGodotSetupCompleted() {
		super.onGodotSetupCompleted()

		val context = activity ?: return

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
				PermissionsUtil.requestPermissions(context, permissionsToRequest)
			}
		}
	}

	private fun getPluginPermissionsToEnable(): MutableList<String> {
		val permissionsToEnable = mutableListOf<String>()
		// Go through the list of permissions, and request the ones that are included in the
		// manifest.
		val vendorPermissions = getVendorPermissions(BuildConfig.FLAVOR)
		for (permission in vendorPermissions) {
			if (PermissionsUtil.hasManifestPermission(activity, permission)) {
				permissionsToEnable.add(permission)
			}
		}

		return permissionsToEnable
	}

	override fun supportsFeature(featureTag: String): Boolean {
		when (featureTag) {
			META_HORIZON_OS -> {
				return BuildConfig.FLAVOR == META_VENDOR_NAME || isHorizonOSDevice(context)
			}

			PICO_OS -> {
				return BuildConfig.FLAVOR == PICO_VENDOR_NAME || isPicoOSDevice()
			}

			EYE_GAZE_INTERACTION_FEATURE_TAG -> {
				when (BuildConfig.FLAVOR) {
					META_VENDOR_NAME -> {
						val grantedPermissions = godot?.getGrantedPermissions()
						if (grantedPermissions != null) {
							for (permission in grantedPermissions) {
								if (META_EYE_TRACKING_PERMISSION == permission) {
									return true
								}
							}
						}
					}

					PICO_VENDOR_NAME -> {
						val grantedPermissions = godot?.getGrantedPermissions()
						if (grantedPermissions != null) {
							for (permission in grantedPermissions) {
								if (PICO_EYE_TRACKING_PERMISSION == permission) {
									return true
								}
							}
						}
					}

					else -> {
						// Other vendors don't require the permission.
						return true;
					}
				}
			}
		}
		return super.supportsFeature(featureTag)
	}
}
