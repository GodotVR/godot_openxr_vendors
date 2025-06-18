/**************************************************************************/
/*  magicleap_editor_plugin.cpp                                           */
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

#include "export/magicleap_export_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

MagicleapEditorExportPlugin::MagicleapEditorExportPlugin() {
	set_vendor_name(MAGICLEAP_VENDOR_NAME);
}

void MagicleapEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> MagicleapEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());

	return export_options;
}

String MagicleapEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	if (ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
		contents += "    <uses-permission android:name=\"com.magicleap.permission.HAND_TRACKING\" />\n";
	}

	// Always include this.
	contents += "    <uses-feature android:name=\"com.magicleap.api_level\" android:version=\"20\" />\n";

	return contents;
}
