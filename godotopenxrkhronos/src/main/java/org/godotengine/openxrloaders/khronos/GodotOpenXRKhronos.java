package org.godotengine.openxrloaders.khronos;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

/**
 * \brief GodotOpenXRKhronos is the OpenXR Khronos loader plugin for Godot.
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
public class GodotOpenXRKhronos extends GodotPlugin {

    public GodotOpenXRKhronos(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return "GodotOpenXRKhronos";
    }
}
