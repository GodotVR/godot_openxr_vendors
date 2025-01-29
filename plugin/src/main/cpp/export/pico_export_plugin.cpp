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

void PicoEditorPlugin::_bind_methods() {}

void PicoEditorPlugin::_enter_tree() {
	// Initialize the editor export plugin
	pico_export_plugin.instantiate();
	add_export_plugin(pico_export_plugin);
}

void PicoEditorPlugin::_exit_tree() {
	// Clean up the editor export plugin
	remove_export_plugin(pico_export_plugin);

	pico_export_plugin.unref();
}

PicoEditorExportPlugin::PicoEditorExportPlugin() {
	set_vendor_name(PICO_VENDOR_NAME);

	_eye_tracking_option = _generate_export_option(
			"pico_xr_features/eye_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Yes",
			PROPERTY_USAGE_DEFAULT,
			MANIFEST_FALSE_VALUE,
			false);

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
			"No,Low frequency,High frequency (60Hz)",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_NONE_VALUE,
			false);
}

void PicoEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> PicoEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_eye_tracking_option);
	export_options.append(_face_tracking_option);
	export_options.append(_hand_tracking_option);

	return export_options;
}

bool PicoEditorExportPlugin::_is_eye_tracking_enabled() const {
	bool eye_tracking_project_setting_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
	if (!eye_tracking_project_setting_enabled) {
		return false;
	}

	int eye_tracking_option_value = _get_int_option("pico_xr_features/eye_tracking", MANIFEST_FALSE_VALUE);
	return eye_tracking_option_value == MANIFEST_TRUE_VALUE;
}

PackedStringArray PicoEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray features;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return features;
	}

	// Add the eye tracking feature if necessary
	if (_is_eye_tracking_enabled()) {
		features.append(EYE_GAZE_INTERACTION_FEATURE);
	}

	return features;
}

String PicoEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "pico_xr_features/eye_tracking") {
		if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			return "\"Eye tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "pico_xr_features/face_tracking") {
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
		int hand_tracking = _get_int_option(option, HAND_TRACKING_NONE_VALUE);
		if (!openxr_enabled && hand_tracking > HAND_TRACKING_NONE_VALUE) {
			return "\"Hand tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	}

	return OpenXREditorExportPlugin::_get_export_option_warning(platform, option);
}

String PicoEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	// Check for eye tracking
	if (_is_eye_tracking_enabled()) {
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

	// Check for eye tracking
	if (_is_eye_tracking_enabled()) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"picovr.software.eye_tracking\" android:value=\"1\" />\n";
	}

	// Face tracking
	int face_tracking = _get_int_option("pico_xr_features/face_tracking", FACE_TRACKING_NONE_VALUE);
	if (face_tracking > FACE_TRACKING_NONE_VALUE) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"picovr.software.face_tracking\" android:value=\"1\" />\n";
	}

	//Hand tracking
	int hand_tracking = _get_int_option("pico_xr_features/hand_tracking", HAND_TRACKING_NONE_VALUE);
	if (hand_tracking > HAND_TRACKING_NONE_VALUE) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"handtracking\" android:value=\"1\" />\n";
	}
	if (hand_tracking > HAND_TRACKING_LOWFREQUENCY_VALUE) {
		contents += "        <meta-data tools:node=\"replace\" android:name=\"Hand_Tracking_HighFrequency\" android:value=\"1\" />\n";
	}

	return contents;
}
