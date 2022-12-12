package com.godot.godotopenxrkhr;

import org.godotengine.godot.Godot;
import java.util.Set;


public class GodotOpenXRKHR extends org.godotengine.godot.plugin.GodotPlugin{

    public GodotOpenXRKHR(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRKHR";
    }

    @Override
    protected Set<String> getPluginGDNativeLibrariesPaths() {
        return super.getPluginGDNativeLibrariesPaths();
    }
}
