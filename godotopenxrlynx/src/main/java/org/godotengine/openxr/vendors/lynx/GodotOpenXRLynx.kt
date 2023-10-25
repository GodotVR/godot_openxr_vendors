package org.godotengine.openxr.vendors.lynx

import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin

/**
 * \brief GodotOpenXRLynx is the OpenXR Lynx plugin for Godot.
 */
class GodotOpenXRLynx(godot: Godot?) : GodotPlugin(godot) {
    override fun getPluginName(): String {
        return "GodotOpenXRLynx"
    }
}
