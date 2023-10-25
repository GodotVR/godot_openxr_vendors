package org.godotengine.openxr.vendors.meta

import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin

/**
 * Godot plugin for the Meta OpenXR loader.
 */
class GodotOpenXRMeta(godot: Godot?) : GodotPlugin(godot) {
    override fun getPluginName(): String {
        return "GodotOpenXRMeta"
    }

    override fun supportsFeature(featureTag: String): Boolean {
        if ("PERMISSION_XR_EXT_eye_gaze_interaction" == featureTag) {
            val grantedPermissions = godot?.getGrantedPermissions()
            if (grantedPermissions != null) {
                for (permission in grantedPermissions) {
                    if ("com.oculus.permission.EYE_TRACKING" == permission) {
                        return true
                    }
                }
            }
        }
        return false
    }
}
