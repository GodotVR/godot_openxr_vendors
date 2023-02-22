package org.godotengine.openxrloaders.lynx;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

/**
 * \brief GodotOpenXRLynx is the OpenXR Lynx loader plugin for Godot.
 */
public class GodotOpenXRLynx extends GodotPlugin {

    public GodotOpenXRLynx(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRLynx";
    }
}
