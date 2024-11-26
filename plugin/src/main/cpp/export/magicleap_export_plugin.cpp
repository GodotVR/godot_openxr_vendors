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

using namespace godot;

void MagicleapEditorPlugin::_bind_methods() {}

void MagicleapEditorPlugin::_enter_tree() {
	// Initialize the editor export plugin
	magicleap_export_plugin.instantiate();
	add_export_plugin(magicleap_export_plugin);
}

void MagicleapEditorPlugin::_exit_tree() {
	// Clean up the editor export plugin
	remove_export_plugin(magicleap_export_plugin);

	magicleap_export_plugin.unref();
}

MagicleapEditorExportPlugin::MagicleapEditorExportPlugin() {
	set_vendor_name(MAGICLEAP_VENDOR_NAME);

	_ml2_hand_tracking_option = _generate_export_option(
			"magicleap_xr_features/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Yes",
			PROPERTY_USAGE_DEFAULT,
			MANIFEST_FALSE_VALUE,
			false);
}

void MagicleapEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> MagicleapEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());

	export_options.append(_ml2_hand_tracking_option);

	return export_options;
}

Dictionary MagicleapEditorExportPlugin::_get_export_options_overrides(const Ref<godot::EditorExportPlatform> &p_platform) const {
	Dictionary overrides;
	if (!_supports_platform(p_platform)) {
		return overrides;
	}

	if (!_is_vendor_plugin_enabled()) {
		overrides["magicleap_xr_features/hand_tracking"] = MANIFEST_FALSE_VALUE;
	}

	return overrides;
}

PackedStringArray MagicleapEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray features;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return features;
	}

	return features;
}

String MagicleapEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "magicleap_xr_features/hand_tracking") {
		if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			return "\"Hand Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	}

	return OpenXREditorExportPlugin::_get_export_option_warning(platform, option);
}

String MagicleapEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	if (_get_int_option("magicleap_xr_features/hand_tracking", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
		contents += "    <uses-permission android:name=\"com.magicleap.permission.HAND_TRACKING\" />\n";
	}

	// Always include this.
	contents += "    <uses-feature android:name=\"com.magicleap.api_level\" android:version=\"20\" />\n";

	return contents;
}
