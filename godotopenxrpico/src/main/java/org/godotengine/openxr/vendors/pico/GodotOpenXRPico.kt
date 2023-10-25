package org.godotengine.openxr.vendors.pico

import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin

/**
 * Godot plugin for the Pico OpenXR loader.
 */
class GodotOpenXRPico(godot: Godot?) : GodotPlugin(godot) {
    override fun getPluginName(): String {
        return "GodotOpenXRPico"
    }
}
