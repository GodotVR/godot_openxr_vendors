/**************************************************************************/
/*  pico_editor_plugin.cpp                                                */
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

#include "export/pico_export_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

PicoEditorExportPlugin::PicoEditorExportPlugin() {
	set_vendor_name(PICO_VENDOR_NAME);

	_face_tracking_option = _generate_export_option(
			"pico_xr_features/face_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Face only,Lipsync only,Hybrid",
			PROPERTY_USAGE_DEFAULT,
			FACE_TRACKING_NONE_VALUE,
			false);

	_hand_tracking_option = _generate_export_option(
			"pico_xr_features/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Low frequency:1,High frequency (60Hz):2",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_LOWFREQUENCY_VALUE,
			false);

	ProjectSettings::get_singleton()->connect("settings_changed", callable_mp(this, &PicoEditorExportPlugin::_project_settings_changed));
}

void PicoEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> PicoEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_face_tracking_option);
	export_options.append(_hand_tracking_option);

	return export_options;
}

String PicoEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "pico_xr_features/face_tracking") {
		int face_tracking = _get_int_option(option, FACE_TRACKING_NONE_VALUE);
		if (!openxr_enabled && face_tracking > FACE_TRACKING_NONE_VALUE) {
			return "\"Face tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
		bool record_audio = _get_bool_option("permissions/record_audio");
		if (!record_audio && face_tracking == FACE_TRACKING_LIPSYNCONLY_VALUE) {
			return "\"Lipsync face tracking\" requires \"Record Audio\" to be checked.\n";
		}
		if (!record_audio && face_tracking == FACE_TRACKING_HYBRID_VALUE) {
			return "\"Hybrid face tracking\" requires \"Record Audio\" to be checked.\n";
		}
	} else if (option == "pico_xr_features/hand_tracking") {
		if (!openxr_enabled && (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
			return "\"Hand tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	}

	return OpenXRVendorsEditorExportPlugin::_get_export_option_warning(platform, option);
}

bool PicoEditorExportPlugin::_get_export_option_visibility(const Ref<EditorExportPlatform> &p_platform, const String &p_option) const {
	if (p_option == "pico_xr_features/hand_tracking") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking");
	}

	return true;
}

bool PicoEditorExportPlugin::_should_update_export_options(const Ref<EditorExportPlatform> &p_platform) const {
	if (!_supports_platform(p_platform)) {
		return false;
	}

	if (_should_update_options) {
		_should_update_options = false;
		return true;
	}

	return false;
}

String PicoEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	// Check for eye tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
		contents += "    <uses-permission android:name=\"com.picovr.permission.EYE_TRACKING\" />\n";
	}

	// Face tracking
	int face_tracking = _get_int_option("pico_xr_features/face_tracking", FACE_TRACKING_NONE_VALUE);
	if (face_tracking == FACE_TRACKING_FACEONLY_VALUE || face_tracking == FACE_TRACKING_HYBRID_VALUE) {
		contents += "    <uses-permission android:name=\"com.picovr.permission.FACE_TRACKING\" />\n";
	}

	return contents;
}

String PicoEditorExportPlugin::_get_android_manifest_application_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	// Check for eye tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"picovr.software.eye_tracking\" android:value=\"1\" />\n";
	}

	// Face tracking
	int face_tracking = _get_int_option("pico_xr_features/face_tracking", FACE_TRACKING_NONE_VALUE);
	if (face_tracking > FACE_TRACKING_NONE_VALUE) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"picovr.software.face_tracking\" android:value=\"1\" />\n";
	}

	//Hand tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"handtracking\" android:value=\"1\" />\n";

		int hand_tracking = _get_int_option("pico_xr_features/hand_tracking", HAND_TRACKING_LOWFREQUENCY_VALUE);
		if (hand_tracking == HAND_TRACKING_HIGHFREQUENCY_VALUE) {
			contents += "        <meta-data tools:node=\"replace\" android:name=\"Hand_Tracking_HighFrequency\" android:value=\"1\" />\n";
		}
	}

	return contents;
}
