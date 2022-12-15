package org.godotengine.openxrloaders.godotopenxrkhr;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;


public class GodotOpenXRKHR extends GodotPlugin {

    public GodotOpenXRKHR(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRKHR";
    }
}
