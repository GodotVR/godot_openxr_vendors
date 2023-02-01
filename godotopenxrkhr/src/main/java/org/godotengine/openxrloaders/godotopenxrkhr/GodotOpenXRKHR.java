package org.godotengine.openxrloaders.khr;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

/**
 * \brief GodotOpenXRKHR is the OpenXR KHR loader plugin for Godot.
 *
 * When using OpenXR for your application on Android make sure that
 * the IMMERSIVE_HMD category is added to your activities intent-filter.
 *
 *   <intent-filter>
 *       <action android:name="android.intent.action.MAIN" />
 *       <category android:name="android.intent.category.LAUNCHER" />
 *       <category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />
 *   </intent-filter>
 *
 * more details can be found here:
 * https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category
 *
 */
public class GodotOpenXRKHR extends GodotPlugin {

    public GodotOpenXRKHR(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRKHR";
    }
}
