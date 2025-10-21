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

#include "export/meta_export_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_interface.hpp>

using namespace godot;

MetaEditorExportPlugin::MetaEditorExportPlugin() {
	set_vendor_name(META_VENDOR_NAME);

	_eye_tracking_option = _generate_export_option(
			"meta_xr_features/eye_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional:1,Required:2",
			PROPERTY_USAGE_DEFAULT,
			EYE_TRACKING_OPTIONAL_VALUE,
			false);
	_face_tracking_option = _generate_export_option(
			"meta_xr_features/face_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional:1,Required:2",
			PROPERTY_USAGE_DEFAULT,
			FACE_TRACKING_OPTIONAL_VALUE,
			false);
	_body_tracking_option = _generate_export_option(
			"meta_xr_features/body_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional:1,Required:2",
			PROPERTY_USAGE_DEFAULT,
			BODY_TRACKING_OPTIONAL_VALUE,
			false);
	_hand_tracking_option = _generate_export_option(
			"meta_xr_features/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional:1,Required:2",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_OPTIONAL_VALUE,
			false);
	_hand_tracking_frequency_option = _generate_export_option(
			"meta_xr_features/hand_tracking_frequency",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Low,High",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_FREQUENCY_LOW_VALUE,
			false);
	_passthrough_option = _generate_export_option(
			"meta_xr_features/passthrough",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional:1,Required:2",
			PROPERTY_USAGE_DEFAULT,
			PASSTHROUGH_OPTIONAL_VALUE,
			false);
	_render_model_option = _generate_export_option(
			"meta_xr_features/render_model",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional:1,Required:2",
			PROPERTY_USAGE_DEFAULT,
			RENDER_MODEL_OPTIONAL_VALUE,
			false);
	_use_overlay_keyboard_option = _generate_export_option(
			"meta_xr_features/use_overlay_keyboard",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false);
	_use_experimental_features_option = _generate_export_option(
			"meta_xr_features/use_experimental_features",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false);
	_boundary_mode_option = _generate_export_option(
			"meta_xr_features/boundary_mode",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Enabled,Disabled",
			PROPERTY_USAGE_DEFAULT,
			BOUNDARY_ENABLED_VALUE,
			false);
	_instant_splash_screen_option = _generate_export_option(
			"meta_xr_features/instant_splash_screen",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false);
	_support_quest_1_option = _generate_export_option(
			"meta_xr_features/quest_1_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			false);
	_support_quest_2_option = _generate_export_option(
			"meta_xr_features/quest_2_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			true,
			false);
	_support_quest_3_option = _generate_export_option(
			"meta_xr_features/quest_3_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			true,
			false);
	_support_quest_pro_option = _generate_export_option(
			"meta_xr_features/quest_pro_support",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			true,
			false);

	ProjectSettings::get_singleton()->connect("settings_changed", callable_mp(this, &MetaEditorExportPlugin::_project_settings_changed));
}

void MetaEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> MetaEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_eye_tracking_option);
	export_options.append(_face_tracking_option);
	export_options.append(_body_tracking_option);
	export_options.append(_hand_tracking_option);
	export_options.append(_hand_tracking_frequency_option);
	export_options.append(_passthrough_option);
	export_options.append(_render_model_option);
	export_options.append(_use_overlay_keyboard_option);
	export_options.append(_use_experimental_features_option);
	export_options.append(_boundary_mode_option);
	export_options.append(_instant_splash_screen_option);
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
		supported_devices.append("quest3s");
	}

	if (_get_bool_option("meta_xr_features/quest_pro_support")) {
		supported_devices.append("questpro");
	}

	return supported_devices;
}

PackedStringArray MetaEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray features = OpenXRVendorsEditorExportPlugin::_get_export_features(platform, debug);
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return features;
	}

	// Add a feature to indicate that this is a hybrid app.
	if (_is_openxr_enabled() && _is_hybrid_app_enabled()) {
		features.append(HYBRID_APP_FEATURE);
	}

	return features;
}

String MetaEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "meta_xr_features/eye_tracking") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
			return "\"Eye Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/face_tracking") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/face_tracking")) {
			return "\"Face Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/body_tracking") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/body_tracking")) {
			return "\"Body Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/hand_tracking") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
			return "\"Hand Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/passthrough") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/passthrough")) {
			return "\"Passthrough\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/render_model") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/render_model")) {
			return "\"Render Model\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/use_experimental_features") {
		if (!openxr_enabled && _get_bool_option(option)) {
			return "\"Use experimental features\" is only valid when \"XR Mode\" is \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/boundary_mode") {
		if (!openxr_enabled && _get_int_option(option, BOUNDARY_ENABLED_VALUE) > BOUNDARY_ENABLED_VALUE) {
			return "Boundary mode changes require \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "meta_xr_features/instant_splash_screen") {
		if (!openxr_enabled && _get_bool_option(option)) {
			return "\"Instant splash screen\" is only valid when \"XR Mode\" is \"OpenXR\".\n";
		}
	} else if (option == "xr_features/enable_meta_plugin") {
		if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/color_space")) {
			if ((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/color_space/starting_color_space") == 0) {
				return "\"Recommended color space is REC709, this can be updated in OpenXR project settings.\"";
			}
		}
	}

	return OpenXRVendorsEditorExportPlugin::_get_export_option_warning(platform, option);
}

bool MetaEditorExportPlugin::_get_export_option_visibility(const Ref<EditorExportPlatform> &p_platform, const String &p_option) const {
	if (p_option == "meta_xr_features/eye_tracking") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
	} else if (p_option == "meta_xr_features/face_tracking") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/face_tracking");
	} else if (p_option == "meta_xr_features/body_tracking") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/body_tracking");
	} else if (p_option == "meta_xr_features/hand_tracking" || p_option == "meta_xr_features/hand_tracking_frequency") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking");
	} else if (p_option == "meta_xr_features/passthrough") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/passthrough");
	} else if (p_option == "meta_xr_features/render_model") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/render_model");
	}

	return true;
}

bool MetaEditorExportPlugin::_should_update_export_options(const Ref<EditorExportPlatform> &p_platform) const {
	if (!_supports_platform(p_platform)) {
		return false;
	}

	if (_should_update_options) {
		_should_update_options = false;
		return true;
	}

	return false;
}

String MetaEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	// Check for eye tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.EYE_TRACKING\" />\n";

		int eye_tracking_value = _get_int_option("meta_xr_features/eye_tracking", EYE_TRACKING_OPTIONAL_VALUE);
		if (eye_tracking_value == EYE_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature android:name=\"oculus.software.eye_tracking\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature android:name=\"oculus.software.eye_tracking\" android:required=\"false\" />\n";
		}
	}

	// Check for face tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/face_tracking")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.FACE_TRACKING\" />\n";

		int face_tracking_value = _get_int_option("meta_xr_features/face_tracking", FACE_TRACKING_OPTIONAL_VALUE);
		if (face_tracking_value == FACE_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.face_tracking\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.face_tracking\" android:required=\"false\" />\n";
		}
	}

	// Check for body tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/body_tracking")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.BODY_TRACKING\" />\n";

		int body_tracking_value = _get_int_option("meta_xr_features/body_tracking", BODY_TRACKING_OPTIONAL_VALUE);
		if (body_tracking_value == BODY_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.software.body_tracking\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.software.body_tracking\" android:required=\"false\" />\n";
		}
	}

	// Check for hand tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.HAND_TRACKING\" />\n";

		int hand_tracking_value = _get_int_option("meta_xr_features/hand_tracking", HAND_TRACKING_OPTIONAL_VALUE);
		if (hand_tracking_value == HAND_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.handtracking\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.handtracking\" android:required=\"false\" />\n";
		}
	}

	// Check for passthrough
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/passthrough")) {
		int passthrough_mode = _get_int_option("meta_xr_features/passthrough", PASSTHROUGH_OPTIONAL_VALUE);
		if (passthrough_mode == PASSTHROUGH_REQUIRED_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.PASSTHROUGH\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.PASSTHROUGH\" android:required=\"false\" />\n";
		}
	}

	// Check for render model
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/render_model")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.RENDER_MODEL\" />\n";

		int render_model_value = _get_int_option("meta_xr_features/render_model", RENDER_MODEL_OPTIONAL_VALUE);
		if (render_model_value == RENDER_MODEL_REQUIRED_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.RENDER_MODEL\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.RENDER_MODEL\" android:required=\"false\" />\n";
		}
	}

	// Check for anchor api
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/anchor_api")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.USE_ANCHOR_API\" />\n";
	}

	// Check for anchor sharing
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/anchor_sharing")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.IMPORT_EXPORT_IOT_MAP_DATA\" />\n";
	}

	// Check for scene api
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/scene_api")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.USE_SCENE\" />\n";
	}

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	// Check for boundary visibility
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/meta/boundary_visibility")) {
		contents += "    <uses-permission android:name=\"com.oculus.permission.BOUNDARY_VISIBILITY\" />\n";
	}
#endif // META_HEADERS_ENABLED

	// Check for overlay keyboard
	bool use_overlay_keyboard_option = _get_bool_option("meta_xr_features/use_overlay_keyboard");
	if (use_overlay_keyboard_option) {
		contents += "    <uses-feature android:name=\"oculus.software.overlay_keyboard\" android:required=\"true\" />\n";
	}

	// Check for experimental features
	bool use_experimental_features = _get_bool_option("meta_xr_features/use_experimental_features");
	if (use_experimental_features) {
		contents += "    <uses-feature android:name=\"com.oculus.experimental.enabled\" />\n";
	}

	// Check for boundary mode
	int boundary_mode = _get_int_option("meta_xr_features/boundary_mode", BOUNDARY_ENABLED_VALUE);
	if (boundary_mode == BOUNDARY_DISABLED_VALUE) {
		contents += "    <uses-feature tools:node=\"replace\" android:name=\"com.oculus.feature.BOUNDARYLESS_APP\" android:required=\"true\" />\n";
		// This overrides the default in plugin/src/main/AndroidManifest.xml, changing "required" to "true", which the Horizon store requires.
		contents += "    <uses-feature android:name=\"android.hardware.vr.headtracking\" android:required=\"true\" android:version=\"1\" tools:node=\"replace\" />\n";
	}

	// Add camera permissions if needed.
	if (_get_bool_option("permissions/camera")) {
		contents += "    <uses-permission android:name=\"horizonos.permission.HEADSET_CAMERA\"/>\n";
		contents += "    <uses-feature android:name=\"android.hardware.camera2.any\" android:required=\"false\"/>\n";
	}

	return contents;
}

void MetaEditorExportPlugin::_export_begin(const PackedStringArray &p_features, bool p_is_debug, const String &p_path, uint32_t p_flags) {
	bool instant_splash_screen = _get_bool_option("meta_xr_features/instant_splash_screen");
	if (!instant_splash_screen) {
		return;
	}

	String boot_splash_path = ProjectSettings::get_singleton()->get_setting_with_override("application/boot_splash/image");
	if (!FileAccess::file_exists(boot_splash_path)) {
		return;
	}

	PackedByteArray boot_splash_file = FileAccess::get_file_as_bytes(boot_splash_path);
	add_file("res://vr_splash.png", boot_splash_file, true);
}

String MetaEditorExportPlugin::_get_android_manifest_application_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	const String supported_devices = String("|").join(_get_supported_devices());
	contents += "        <meta-data tools:node=\"replace\" android:name=\"com.oculus.supportedDevices\" android:value=\"" + supported_devices + "\" />\n";

	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
		int hand_tracking_frequency = _get_int_option("meta_xr_features/hand_tracking_frequency", HAND_TRACKING_FREQUENCY_LOW_VALUE);
		const String hand_tracking_frequency_label = (hand_tracking_frequency == HAND_TRACKING_FREQUENCY_LOW_VALUE) ? "LOW" : "HIGH";
		contents += "        <meta-data tools:node=\"replace\" android:name=\"com.oculus.handtracking.frequency\" android:value=\"" + hand_tracking_frequency_label + "\" />\n";
		contents += "        <meta-data tools:node=\"replace\" android:name=\"com.oculus.handtracking.version\" android:value=\"V2.0\" />\n";
	}

	bool instant_splash_screen = _get_bool_option("meta_xr_features/instant_splash_screen");
	if (instant_splash_screen) {
		contents += "        <meta-data android:name=\"com.oculus.ossplash\" android:value=\"true\"/>\n";
	}

	if ((int)project_settings->get_setting_with_override("xr/openxr/environment_blend_mode") != XRInterface::XR_ENV_BLEND_MODE_OPAQUE) {
		// Show the splash screen in passthrough, if the user launches it from passthrough.
		contents += "        <meta-data android:name=\"com.oculus.ossplash.background\" android:value=\"passthrough-contextual\" />\n";
	}

	OpenXRHybridApp::HybridMode hybrid_launch_mode = _get_hybrid_app_launch_mode();
	if (hybrid_launch_mode != OpenXRHybridApp::HYBRID_MODE_NONE) {
		contents += _get_opening_activity_tag_for_panel_app();

		contents +=
				"          <intent-filter>\n"
				"            <action android:name=\"android.intent.action.MAIN\" />\n"
				"            <category android:name=\"android.intent.category.DEFAULT\" />\n"
				"            <category android:name=\"org.godotengine.xr.hybrid.PANEL\" />\n"
				"            <category android:name=\"com.oculus.intent.category.2D\" />\n";

		if (hybrid_launch_mode == OpenXRHybridApp::HYBRID_MODE_PANEL) {
			contents += "            <category android:name=\"android.intent.category.LAUNCHER\" />\n";
		}

		contents +=
				"          </intent-filter>\n"
				"        </activity>\n";
	}

	return contents;
}

String MetaEditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	String contents = R"(
				<intent-filter>
					<action android:name="android.intent.action.MAIN" />

					<category android:name="android.intent.category.DEFAULT" />

					<!-- Enable access to OpenXR on Oculus mobile devices, no-op on other Android
					platforms. -->
					<category android:name="com.oculus.intent.category.VR" />

					<!-- OpenXR category tag to indicate the activity starts in an immersive OpenXR mode.
					See https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category. -->
					<category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />
)";

	contents += _get_common_activity_intent_filter_contents();
	contents += R"(
				</intent-filter>
)";

	return contents;
}
