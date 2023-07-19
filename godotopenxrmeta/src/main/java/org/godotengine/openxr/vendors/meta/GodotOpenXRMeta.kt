package org.godotengine.openxr.vendors.meta

import android.util.Log
import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin

/**
 * Godot plugin for the Meta OpenXR loader.
 */
class GodotOpenXRMeta(godot: Godot?) : GodotPlugin(godot) {
    companion object {
        val TAG = GodotOpenXRMeta::class.java.simpleName

        init {
            try {
                Log.v(TAG, "Loading godotopenxrmeta library")
                System.loadLibrary("godotopenxrmeta")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Unable to load godotopenxrmeta shared library")
            }
        }
    }

    override fun getPluginName(): String {
        return "GodotOpenXRMeta"
    }

    override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/godotopenxrvendors/meta/plugin.gdextension")

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
