package com.godot.godotopenxrmeta;

import org.godotengine.godot.Godot;
import java.util.Set;


public class GodotOpenXRMeta extends org.godotengine.godot.plugin.GodotPlugin{

    public GodotOpenXRMeta(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRMeta";
    }

    @Override
    protected Set<String> getPluginGDNativeLibrariesPaths() {
        return super.getPluginGDNativeLibrariesPaths();
    }
}
