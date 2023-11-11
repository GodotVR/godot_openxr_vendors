/**************************************************************************/
/*  GodotOpenXRKhronos.kt                                                 */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

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
