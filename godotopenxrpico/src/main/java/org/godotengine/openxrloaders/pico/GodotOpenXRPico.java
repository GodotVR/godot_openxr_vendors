package org.godotengine.openxrloaders.pico;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

/**
 * Godot plugin for the Pico OpenXR loader.
 */
public class GodotOpenXRPico extends GodotPlugin {

    public GodotOpenXRPico(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRPico";
    }
}
