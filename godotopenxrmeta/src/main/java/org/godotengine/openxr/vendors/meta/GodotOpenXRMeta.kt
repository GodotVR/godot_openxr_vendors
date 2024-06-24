/**************************************************************************/
/*  GodotOpenXRMeta.kt                                                    */
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

package org.godotengine.openxr.vendors.meta

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.pm.PermissionInfo
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.Settings
import android.util.Log
import android.view.View
import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.utils.PermissionsUtil

/**
 * Godot plugin for the Meta OpenXR loader.
 */
class GodotOpenXRMeta(godot: Godot?) : GodotPlugin(godot) {
    companion object {
        private val TAG = GodotOpenXRMeta::class.java.simpleName

        private const val BODY_TRACKING_PERMISSION = "com.oculus.permission.BODY_TRACKING"
        private const val EYE_TRACKING_PERMISSION = "com.oculus.permission.EYE_TRACKING"
        private const val FACE_TRACKING_PERMISSION = "com.oculus.permission.FACE_TRACKING"
        private const val SCENE_PERMISSION = "com.oculus.permission.USE_SCENE"

        init {
            try {
                Log.v(TAG, "Loading godotopenxrvendors library")
                System.loadLibrary("godotopenxrvendors")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Unable to load godotopenxrvendors shared library")
            }
        }

        /**
         * Dispatch the necessary requests for all plugin's permissions in the app's manifest.
         */
        @JvmStatic
        fun requestAllPluginPermissions(activity: Activity) {
            val permissionsToRequest = ArrayList<String>()
            // Request the body tracking permission if it's included in the manifest
            if (PermissionsUtil.hasManifestPermission(activity, BODY_TRACKING_PERMISSION)) {
                permissionsToRequest.add(BODY_TRACKING_PERMISSION)
            }
            // Request the eye tracking permission if it's included in the manifest
            if (PermissionsUtil.hasManifestPermission(activity, EYE_TRACKING_PERMISSION)) {
                permissionsToRequest.add(EYE_TRACKING_PERMISSION)
            }
            // Request the face tracking permission if it's included in the manifest
            if (PermissionsUtil.hasManifestPermission(activity, FACE_TRACKING_PERMISSION)) {
                permissionsToRequest.add(FACE_TRACKING_PERMISSION)
            }
            // Request the scene API permission if it's included in the manifest
            if (PermissionsUtil.hasManifestPermission(activity, SCENE_PERMISSION)) {
                permissionsToRequest.add(SCENE_PERMISSION)
            }

            if (permissionsToRequest.isNotEmpty()) {
                PermissionsUtil.requestPermissions(activity, permissionsToRequest)
            }
        }
    }

    override fun getPluginName(): String {
        return "GodotOpenXRMeta"
    }

    override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/godotopenxrvendors/plugin.gdextension")

    override fun onMainCreate(activity: Activity): View? {
        requestAllPluginPermissions(activity)
        return null
    }

    override fun supportsFeature(featureTag: String): Boolean {
        if ("PERMISSION_XR_EXT_eye_gaze_interaction" == featureTag) {
            val grantedPermissions = godot?.getGrantedPermissions()
            if (grantedPermissions != null) {
                for (permission in grantedPermissions) {
                    if (EYE_TRACKING_PERMISSION == permission) {
                        return true
                    }
                }
            }
        }
        return false
    }
}
