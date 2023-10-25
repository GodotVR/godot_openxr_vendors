package org.godotengine.openxr.vendors.khronos

import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin

/**
 * \brief GodotOpenXRKhronos is the OpenXR Khronos plugin for Godot.
 *
 * When using OpenXR for your application on Android make sure that
 * the IMMERSIVE_HMD category is added to your activities intent-filter.
 *
 * <intent-filter>
 * <action android:name="android.intent.action.MAIN"></action>
 * <category android:name="android.intent.category.LAUNCHER"></category>
 * <category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD"></category>
</intent-filter> *
 *
 * more details can be found here:
 * https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category
 *
 */
class GodotOpenXRKhronos(godot: Godot?) : GodotPlugin(godot) {
    override fun getPluginName(): String {
        return "GodotOpenXRKhronos"
    }
}
