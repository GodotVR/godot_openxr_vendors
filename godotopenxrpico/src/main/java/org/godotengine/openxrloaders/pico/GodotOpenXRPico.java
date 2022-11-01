package org.godotengine.openxrloaders.pico;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

public class GodotOpenXRPico extends GodotPlugin {

    public GodotOpenXRPico(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRPico";
    }
}
