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
         * Returns the information of the desired permission.
         * @param context the caller context for this method.
         * @param permission the name of the permission.
         * @return permission info object
         * @throws PackageManager.NameNotFoundException the exception is thrown when a given package, application, or component name cannot be found.
         *
         * TODO: remove this when Godot 4.3 is out with the new `PermissionsUtil.requestPermissions(...)` api
         */
        @Throws(PackageManager.NameNotFoundException::class)
        private fun getPermissionInfo(context: Context, permission: String): PermissionInfo {
            val packageManager = context.packageManager
            return packageManager.getPermissionInfo(permission, 0)
        }

        /**
         * Request a list of dangerous permissions. The requested permissions must be included in the app's AndroidManifest
         * @param permissions list of the permissions to request.
         * @param activity the caller activity for this method.
         * @return true/false. "true" if permissions are already granted, "false" if a permissions request was dispatched.
         *
         * TODO: remove this when Godot 4.3 is out with the new `PermissionsUtil.requestPermissions(...)` api
         */
        private fun requestPermissions(activity: Activity?, permissions: List<String>): Boolean {
            if (activity == null) {
                return false
            }
            if (permissions.isEmpty()) {
                return true
            }
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                // Not necessary, asked on install already
                return true
            }

            val requestedPermissions: MutableSet<String> = HashSet()
            for (permission in permissions) {
                try {
                    if (permission == Manifest.permission.MANAGE_EXTERNAL_STORAGE) {
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R && !Environment.isExternalStorageManager()) {
                            Log.d(TAG, "Requesting permission $permission")
                            try {
                                val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION)
                                intent.setData(Uri.parse(String.format("package:%s", activity.packageName)))
                                activity.startActivityForResult(intent, PermissionsUtil.REQUEST_MANAGE_EXTERNAL_STORAGE_REQ_CODE)
                            } catch (ignored: Exception) {
                                val intent = Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)
                                activity.startActivityForResult(intent, PermissionsUtil.REQUEST_MANAGE_EXTERNAL_STORAGE_REQ_CODE)
                            }
                        }
                    } else {
                        val permissionInfo = getPermissionInfo(activity, permission)
                        val protectionLevel = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) permissionInfo.protection else permissionInfo.protectionLevel
                        if (protectionLevel == PermissionInfo.PROTECTION_DANGEROUS && activity.checkSelfPermission(permission) !== PackageManager.PERMISSION_GRANTED) {
                            Log.d(TAG, "Requesting permission $permission")
                            requestedPermissions.add(permission)
                        }
                    }
                } catch (e: PackageManager.NameNotFoundException) {
                    // Skip this permission and continue.
                    Log.w(TAG, "Unable to identify permission $permission", e)
                }
            }
            if (requestedPermissions.isEmpty()) {
                // If list is empty, all of dangerous permissions were granted.
                return true
            }
            activity.requestPermissions(requestedPermissions.toTypedArray<String>(), PermissionsUtil.REQUEST_ALL_PERMISSION_REQ_CODE)
            return true
        }
    }

    override fun getPluginName(): String {
        return "GodotOpenXRMeta"
    }

    override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/godotopenxrvendors/plugin.gdextension")

    override fun onMainCreate(activity: Activity): View? {
        val permissionsToRequest = ArrayList<String>()
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
            requestPermissions(activity, permissionsToRequest)
        }
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
