/**************************************************************************/
/*  meta_editor_plugin.cpp                                                */
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

#include "meta_export_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

void MetaEditorPlugin::_bind_methods() {}

void MetaEditorPlugin::_enter_tree() {
	// Initialize the editor export plugin
	meta_export_plugin = memnew(MetaEditorExportPlugin);
	meta_export_plugin->set_plugin_version(get_plugin_version());

	add_export_plugin(meta_export_plugin);
}

void MetaEditorPlugin::_exit_tree() {
	// Clean up the editor export plugin
	remove_export_plugin(meta_export_plugin);

	memfree(meta_export_plugin);
	meta_export_plugin = nullptr;
}

MetaEditorExportPlugin::MetaEditorExportPlugin() {
	set_vendor_name(META_VENDOR_NAME);

	_eye_tracking_option = _generate_export_option(
			"meta_xr_features/eye_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			EYE_TRACKING_NONE_VALUE,
			false
	);
	_hand_tracking_option = _generate_export_option(
			"meta_xr_features/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_NONE_VALUE,
			false
	);
	_hand_tracking_frequency_option = _generate_export_option(
			"meta_xr_features/hand_tracking_frequency",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Low,High",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_FREQUENCY_LOW_VALUE,
			false
	);
	_passthrough_option = _generate_export_option(
			"meta_xr_features/passthrough",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			PASSTHROUGH_NONE_VALUE,
			false
	);
	_use_anchor_api_option = _generate_export_option(
			"meta_xr_features/use_anchor_api",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false
	);
	_use_scene_api_option = _generate_export_option(
			"meta_xr_features/use_scene_api",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false
	);
	_support_quest_1_option = _generate_export_option(
			"meta_xr_features/quest_1_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false
	);
	_support_quest_2_option = _generate_export_option(
			"meta_xr_features/quest_2_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			true,
			false
	);
	_support_quest_3_option = _generate_export_option(
			"meta_xr_features/quest_3_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			true,
			false
	);
	_support_quest_pro_option = _generate_export_option(
			"meta_xr_features/quest_pro_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			true,
			false
	);
}

void MetaEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> MetaEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_eye_tracking_option);
	export_options.append(_hand_tracking_option);
	export_options.append(_hand_tracking_frequency_option);
	export_options.append(_passthrough_option);
	export_options.append(_use_anchor_api_option);
	export_options.append(_use_scene_api_option);
	export_options.append(_support_quest_1_option);
	export_options.append(_support_quest_2_option);
	export_options.append(_support_quest_3_option);
	export_options.append(_support_quest_pro_option);

	return export_options;
}

PackedStringArray MetaEditorExportPlugin::_get_supported_devices() const {
	PackedStringArray supported_devices;

	if (_get_bool_option("meta_xr_features/quest_1_support")) {
		supported_devices.append("quest");
	}

	if (_get_bool_option("meta_xr_features/quest_2_support")) {
		supported_devices.append("quest2");
	}

	if (_get_bool_option("meta_xr_features/quest_3_support")) {
		supported_devices.append("quest3");
	}

	if (_get_bool_option("meta_xr_features/quest_pro_support")) {
		supported_devices.append("questpro");
	}

	return supported_devices;
}

bool MetaEditorExportPlugin::_is_eye_tracking_enabled() const {
	bool eye_tracking_project_setting_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
	if (!eye_tracking_project_setting_enabled) {
		return false;
	}

	int eye_tracking_option_value = _get_int_option("meta_xr_features/eye_tracking", EYE_TRACKING_NONE_VALUE);
	return eye_tracking_option_value > EYE_TRACKING_NONE_VALUE;
}

PackedStringArray MetaEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray features;
	if (!_supports_platform(platform)) {
		return features;
	}

	// Add the eye tracking feature if necessary
	if (_is_eye_tracking_enabled()) {
		features.append(EYE_GAZE_INTERACTION_FEATURE);
	}

	return features;
}

String MetaEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform)) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "meta_xr_features/eye_tracking") {
		bool eye_tracking_project_setting_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
		int eye_tracking_option_value = _get_int_option("meta_xr_features/eye_tracking", EYE_TRACKING_NONE_VALUE);
		if (eye_tracking_option_value > EYE_TRACKING_NONE_VALUE && !eye_tracking_project_setting_enabled) {
			return "\"Eye Tracking\" project setting must be enabled!\n";
		}
	} else if (option == "meta_xr_features/hand_tracking") {
		if (!openxr_enabled && _get_int_option(option, HAND_TRACKING_NONE_VALUE) > HAND_TRACKING_NONE_VALUE) {
			return "\"Hand Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/passthrough") {
		if (!openxr_enabled && _get_int_option(option, PASSTHROUGH_NONE_VALUE) > PASSTHROUGH_NONE_VALUE) {
			return "\"Passthrough\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/use_anchor_api") {
		if (!openxr_enabled && _get_bool_option(option)) {
			return "\"Use anchor API\" is only valid when \"XR Mode\" is \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/use_scene_api") {
		if (!openxr_enabled && _get_bool_option(option)) {
			return "\"Use scene API\" is only valid when \"XR Mode\" is \"OpenXR\".\n";
		}
	}

	return OpenXREditorExportPlugin::_get_export_option_warning(platform, option);
}

String MetaEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	// Check for eye tracking
	if (_is_eye_tracking_enabled()) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.EYE_TRACKING\" />\n";

		int eye_tracking_value = _get_int_option("meta_xr_features/eye_tracking", EYE_TRACKING_NONE_VALUE);
		if (eye_tracking_value == EYE_TRACKING_OPTIONAL_VALUE) {
			contents += "    <uses-feature android:name=\"oculus.software.eye_tracking\" android:required=\"false\" />\n";
		} else if (eye_tracking_value == EYE_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature android:name=\"oculus.software.eye_tracking\" android:required=\"true\" />\n";
		}
	}

	// Check for hand tracking
	int hand_tracking_value = _get_int_option("meta_xr_features/hand_tracking", HAND_TRACKING_NONE_VALUE);
	if (hand_tracking_value > HAND_TRACKING_NONE_VALUE) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.HAND_TRACKING\" />\n";

		if (hand_tracking_value == HAND_TRACKING_OPTIONAL_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.handtracking\" android:required=\"false\" />\n";
		} else if (hand_tracking_value == HAND_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.handtracking\" android:required=\"true\" />\n";
		}
	}

	// Check for passthrough
	int passthrough_mode = _get_int_option("meta_xr_features/passthrough", PASSTHROUGH_NONE_VALUE);
	if (passthrough_mode == PASSTHROUGH_OPTIONAL_VALUE) {
		contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.PASSTHROUGH\" android:required=\"false\" />\n";
	} else if (passthrough_mode == PASSTHROUGH_REQUIRED_VALUE) {
		contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.PASSTHROUGH\" android:required=\"true\" />\n";
	}

	// Check for anchor api
	bool use_anchor_api = _get_bool_option("meta_xr_features/use_anchor_api");
	if (use_anchor_api) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.USE_ANCHOR_API\" />\n";
	}

	// Check for scene api
	bool use_scene_api = _get_bool_option("meta_xr_features/use_scene_api");
	if (use_scene_api) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.USE_SCENE\" />\n";
	}

	return contents;
}

String MetaEditorExportPlugin::_get_android_manifest_application_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	const String supported_devices = String("|").join(_get_supported_devices());
	contents += "        <meta-data tools:node=\"replace\" android:name=\"com.oculus.supportedDevices\" android:value=\"" + supported_devices + "\" />\n";

	bool hand_tracking_enabled = _get_int_option("meta_xr_features/hand_tracking", HAND_TRACKING_NONE_VALUE) > HAND_TRACKING_NONE_VALUE;
	if (hand_tracking_enabled) {
		int hand_tracking_frequency = _get_int_option("meta_xr_features/hand_tracking_frequency", HAND_TRACKING_FREQUENCY_LOW_VALUE);
		const String hand_tracking_frequency_label = (hand_tracking_frequency == HAND_TRACKING_FREQUENCY_LOW_VALUE) ? "LOW" : "HIGH";
		contents += "        <meta-data tools:node=\"replace\" android:name=\"com.oculus.handtracking.frequency\" android:value=\"" + hand_tracking_frequency_label + "\" />\n";
		contents += "        <meta-data tools:node=\"replace\" android:name=\"com.oculus.handtracking.version\" android:value=\"V2.0\" />\n";
	}

	return contents;
}

String MetaEditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	return R"(
				<intent-filter>
					<action android:name="android.intent.action.MAIN" />
					<category android:name="android.intent.category.LAUNCHER" />

					<!-- Enable access to OpenXR on Oculus mobile devices, no-op on other Android
					platforms. -->
					<category android:name="com.oculus.intent.category.VR" />

					<!-- OpenXR category tag to indicate the activity starts in an immersive OpenXR mode.
					See https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category. -->
					<category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />
				</intent-filter>
)";
}
