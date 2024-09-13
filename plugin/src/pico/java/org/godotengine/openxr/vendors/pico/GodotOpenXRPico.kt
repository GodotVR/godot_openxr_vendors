/**************************************************************************/
/*  GodotOpenXRPico.kt                                                    */
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

package org.godotengine.openxr.vendors.pico

import org.godotengine.godot.Godot
import org.godotengine.godot.utils.PermissionsUtil
import org.godotengine.openxr.vendors.GodotOpenXR

/**
 * Godot plugin for the Pico OpenXR loader.
 */
class GodotOpenXRPico(godot: Godot?) : GodotOpenXR(godot) {
    companion object {
        private const val EYE_TRACKING_PERMISSION = "com.picovr.permission.EYE_TRACKING"
        private const val FACE_TRACKING_PERMISSION = "com.picovr.permission.FACE_TRACKING"
    }

    override fun getPluginName(): String {
        return "GodotOpenXRPico"
    }

    override fun getPluginPermissionsToEnable(): MutableList<String> {
        val permissionsToRequest = super.getPluginPermissionsToEnable()

        // Request the eye tracking permission if it's included in the manifest
        if (PermissionsUtil.hasManifestPermission(activity, EYE_TRACKING_PERMISSION)) {
            permissionsToRequest.add(EYE_TRACKING_PERMISSION)
        }
        // Request the face tracking permission if it's included in the manifest
        if (PermissionsUtil.hasManifestPermission(activity, FACE_TRACKING_PERMISSION)) {
            permissionsToRequest.add(FACE_TRACKING_PERMISSION)
        }

        return permissionsToRequest
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
