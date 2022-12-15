package org.godotengine.openxrloaders.meta;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

/**
 * Godot plugin for the Meta OpenXR loader.
 */
public class GodotOpenXRMeta extends GodotPlugin {

    public GodotOpenXRMeta(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRMeta";
    }

}
