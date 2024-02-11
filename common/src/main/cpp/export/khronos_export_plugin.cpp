/**************************************************************************/
/*  khronos_editor_plugin.cpp                                             */
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

#include "export/khronos_export_plugin.h"

using namespace godot;

void KhronosEditorPlugin::_bind_methods() {}

void KhronosEditorPlugin::_enter_tree() {
	// Initialize the editor export plugin
	khronos_export_plugin.instantiate();
	add_export_plugin(khronos_export_plugin);
}

void KhronosEditorPlugin::_exit_tree() {
	// Clean up the editor export plugin
	remove_export_plugin(khronos_export_plugin);

	khronos_export_plugin.unref();
}

KhronosEditorExportPlugin::KhronosEditorExportPlugin() {
	set_vendor_name(KHRONOS_VENDOR_NAME);
}

void KhronosEditorExportPlugin::_bind_methods() {}

String KhronosEditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	return R"(
				<intent-filter>
					<action android:name="android.intent.action.MAIN" />
					<category android:name="android.intent.category.LAUNCHER" />

					<!-- OpenXR category tag to indicate the activity starts in an immersive OpenXR mode.
					See https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category. -->
					<category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />

					<!-- Enable VR access on HTC Vive Focus devices. -->
					<category android:name="com.htc.intent.category.VRAPP" />
				</intent-filter>
)";
}
