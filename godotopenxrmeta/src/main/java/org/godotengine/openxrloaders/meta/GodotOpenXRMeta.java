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

    @Override
    public boolean supportsFeature(String featureTag) {
        if ("PERMISSION_XR_EXT_eye_gaze_interaction".equals(featureTag)) {
            String[] grantedPermissions = getGodot().getGrantedPermissions();
            if (grantedPermissions != null) {
                for (String permission : grantedPermissions) {
                    if ("com.oculus.permission.EYE_TRACKING".equals(permission)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

}
