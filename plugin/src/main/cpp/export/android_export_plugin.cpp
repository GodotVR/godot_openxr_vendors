/**************************************************************************/
/*  android_export_plugin.cpp                                             */
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

#include "export/android_export_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

void AndroidEditorPlugin::_bind_methods() {}

void AndroidEditorPlugin::_enter_tree() {
	// Initialize the editor export plugin
	android_export_plugin.instantiate();
	add_export_plugin(android_export_plugin);
}

void AndroidEditorPlugin::_exit_tree() {
	// Clean up the editor export plugin
	remove_export_plugin(android_export_plugin);
	android_export_plugin.unref();
}

AndroidEditorExportPlugin::AndroidEditorExportPlugin() {
	set_vendor_name(ANDROID_VENDOR_NAME);

	_eye_tracking_option = _generate_export_option(
			"android_xr_features/eye_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			EYE_TRACKING_NONE_VALUE,
			false);
	_hand_tracking_option = _generate_export_option(
			"android_xr_features/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_NONE_VALUE,
			false);
}

void AndroidEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> AndroidEditorExportPlugin::_get_export_options(const Ref<godot::EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_eye_tracking_option);
	export_options.append(_hand_tracking_option);

	return export_options;
}

Dictionary AndroidEditorExportPlugin::_get_export_options_overrides(const Ref<godot::EditorExportPlatform> &p_platform) const {
	Dictionary overrides;
	if (!_supports_platform(p_platform)) {
		return overrides;
	}

	if (!_is_vendor_plugin_enabled()) {
		overrides["android_xr_features/eye_tracking"] = EYE_TRACKING_NONE_VALUE;
		overrides["android_xr_features/hand_tracking"] = HAND_TRACKING_NONE_VALUE;
	}

	return overrides;
}

bool AndroidEditorExportPlugin::_is_eye_tracking_enabled() const {
	bool eye_tracking_project_setting_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
	if (!eye_tracking_project_setting_enabled) {
		return false;
	}

	int eye_tracking_option_value = _get_int_option("android_xr_features/eye_tracking", EYE_TRACKING_NONE_VALUE);
	return eye_tracking_option_value > EYE_TRACKING_NONE_VALUE;
}

PackedStringArray AndroidEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
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

String AndroidEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "android_xr_features/eye_tracking") {
		bool eye_tracking_project_setting_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
		int eye_tracking_option_value = _get_int_option("android_xr_features/eye_tracking", EYE_TRACKING_NONE_VALUE);
		if (eye_tracking_option_value > EYE_TRACKING_NONE_VALUE && !eye_tracking_project_setting_enabled) {
			return "\"Eye Tracking\" project setting must be enabled!\n";
		}
	} else if (option == "android_xr_features/hand_tracking") {
		if (!openxr_enabled && _get_int_option(option, HAND_TRACKING_NONE_VALUE) > HAND_TRACKING_NONE_VALUE) {
			return "\"Hand Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	}

	return OpenXREditorExportPlugin::_get_export_option_warning(platform, option);
}

String AndroidEditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
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
				</intent-filter>
				<property
					android:name="android.window.PROPERTY_XR_ACTIVITY_START_MODE"
					android:value="XR_ACTIVITY_START_MODE_FULL_SPACE_UNMANAGED" />
)";
}

String AndroidEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	// Android XR required
	contents += "    <uses-feature android:name=\"android.software.xr.immersive\" android:required=\"true\" />\n";

	// 6DoF motion controllers required
	contents += "    <uses-feature android:name=\"android.hardware.xr.input.controller\" android:required=\"true\" />\n";

	// Check for eye tracking
	if (_is_eye_tracking_enabled()) {
		contents += "    <uses-permission android:name=\"android.permission.EYE_TRACKING_FINE\" />\n";

		int eye_tracking_value = _get_int_option("android_xr_features/eye_tracking", EYE_TRACKING_NONE_VALUE);
		if (eye_tracking_value == EYE_TRACKING_OPTIONAL_VALUE) {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.eye_tracking\" android:required=\"false\" />\n";
		} else if (eye_tracking_value == EYE_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.eye_tracking\" android:required=\"true\" />\n";
		}
	}

	// Check for hand tracking
	int hand_tracking_value = _get_int_option("android_xr_features/hand_tracking", HAND_TRACKING_NONE_VALUE);
	if (hand_tracking_value > HAND_TRACKING_NONE_VALUE) {
		contents += "    <uses-permission android:name=\"android.permission.HAND_TRACKING\" />\n";

		if (hand_tracking_value == HAND_TRACKING_OPTIONAL_VALUE) {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.hand_tracking\" android:required=\"false\" />\n";
		} else if (hand_tracking_value == HAND_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.hand_tracking\" android:required=\"true\" />\n";
		}
	}

	return contents;
}
