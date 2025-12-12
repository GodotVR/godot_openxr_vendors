/**************************************************************************/
/*  android_xr_export_plugin.cpp                                          */
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

#include "export/android_xr_export_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

AndroidXREditorExportPlugin::AndroidXREditorExportPlugin() {
	set_vendor_name(ANDROID_XR_VENDOR_NAME);

	_eye_tracking_option = _generate_export_option(
			"android_xr_features/eye_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			EYE_TRACKING_OPTIONAL_VALUE,
			false);
	_face_tracking_option = _generate_export_option(
			"android_xr_features/face_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			FACE_TRACKING_OPTIONAL_VALUE,
			false);
	_hand_tracking_option = _generate_export_option(
			"android_xr_features/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			HAND_TRACKING_OPTIONAL_VALUE,
			false);
	_tracked_controllers_option = _generate_export_option(
			"android_xr_features/tracked_controllers",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Optional,Required",
			PROPERTY_USAGE_DEFAULT,
			TRACKED_CONTROLLERS_OPTIONAL_VALUE,
			false);
	_recommended_boundary_type_option = _generate_export_option(
			"android_xr_features/recommended_boundary_type",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"None,Large",
			PROPERTY_USAGE_DEFAULT,
			RECOMMENDED_BOUNDARY_TYPE_NONE_VALUE,
			false);

	ProjectSettings::get_singleton()->connect("settings_changed", callable_mp(this, &AndroidXREditorExportPlugin::_project_settings_changed));
}

void AndroidXREditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> AndroidXREditorExportPlugin::_get_export_options(const Ref<godot::EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_eye_tracking_option);
	export_options.append(_face_tracking_option);
	export_options.append(_hand_tracking_option);
	export_options.append(_tracked_controllers_option);
	export_options.append(_recommended_boundary_type_option);

	return export_options;
}

PackedStringArray AndroidXREditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
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

String AndroidXREditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	bool openxr_enabled = _is_openxr_enabled();
	if (option == "android_xr_features/eye_tracking") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
			return "\"Eye Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "android_xr_features/face_tracking") {
		if (!openxr_enabled && _get_int_option(option, FACE_TRACKING_OPTIONAL_VALUE) != FACE_TRACKING_OPTIONAL_VALUE) {
			return "\"Face Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "android_xr_features/hand_tracking") {
		if (!openxr_enabled && (bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
			return "\"Hand Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	} else if (option == "android_xr_features/tracked_controllers") {
		if (!openxr_enabled && _get_int_option(option, TRACKED_CONTROLLERS_NONE_VALUE) != TRACKED_CONTROLLERS_NONE_VALUE) {
			return "\"Tracked Controllers\" requires \"XR Mode\" to be \"OpenXR\".\n";
		}
	}

	return OpenXRVendorsEditorExportPlugin::_get_export_option_warning(platform, option);
}

bool AndroidXREditorExportPlugin::_get_export_option_visibility(const Ref<godot::EditorExportPlatform> &p_platform, const godot::String &p_option) const {
	if (p_option == "android_xr_features/eye_tracking") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
	} else if (p_option == "android_xr_features/hand_tracking") {
		return (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking");
	}

	return true;
}

bool AndroidXREditorExportPlugin::_should_update_export_options(const Ref<godot::EditorExportPlatform> &p_platform) const {
	if (!_supports_platform(p_platform)) {
		return false;
	}

	if (_should_update_options) {
		_should_update_options = false;
		return true;
	}

	return false;
}

String AndroidXREditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	String contents = R"(
				<intent-filter>
					<action android:name="android.intent.action.MAIN" />
					<category android:name="android.intent.category.DEFAULT" />

					<!-- OpenXR category tag to indicate the activity starts in an immersive OpenXR mode.
					See https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category. -->
					<category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />
)";

	contents += _get_common_activity_intent_filter_contents();
	contents += R"(
				</intent-filter>
				<property
					android:name="android.window.PROPERTY_XR_ACTIVITY_START_MODE"
					android:value="XR_ACTIVITY_START_MODE_FULL_SPACE_UNMANAGED" />
)";

	return contents;
}

String AndroidXREditorExportPlugin::_get_android_manifest_application_element_contents(const Ref<godot::EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	// Android XR required
	bool android_xr_required = _get_hybrid_app_launch_mode() != OpenXRHybridApp::HYBRID_MODE_PANEL;
	contents += vformat("    <uses-native-library android:name=\"libopenxr.google.so\" android:required=\"%s\" />\n", _bool_to_string(android_xr_required));

	int recommended_boundary_type_option = _get_int_option("android_xr_features/recommended_boundary_type", RECOMMENDED_BOUNDARY_TYPE_NONE_VALUE);
	if (recommended_boundary_type_option == RECOMMENDED_BOUNDARY_TYPE_LARGE_VALUE) {
		contents += "    <property\n"
					"           android:name=\"android.window.PROPERTY_XR_BOUNDARY_TYPE_RECOMMENDED\"\n"
					"           android:value=\"XR_BOUNDARY_TYPE_LARGE\" />\n";
	} else {
		contents += "    <property\n"
					"           android:name=\"android.window.PROPERTY_XR_BOUNDARY_TYPE_RECOMMENDED\"\n"
					"           android:value=\"XR_BOUNDARY_TYPE_NO_RECOMMENDATION\" />\n";
	}

	OpenXRHybridApp::HybridMode hybrid_launch_mode = _get_hybrid_app_launch_mode();
	if (hybrid_launch_mode != OpenXRHybridApp::HYBRID_MODE_NONE) {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();

		contents += vformat(
				R"(
		<activity android:name="org.godotengine.openxr.vendors.GodotPanelApp"
			android:process=":GodotPanelApp"
			android:theme="@style/GodotAppSplashTheme"
			android:launchMode="singleInstancePerTask"
			android:exported="false"
			android:excludeFromRecents="%s"
			android:screenOrientation="%s"
			android:resizeableActivity="%s"
			android:configChanges="orientation|keyboardHidden|screenSize|smallestScreenSize|density|keyboard|navigation|screenLayout|uiMode"
			tools:ignore="UnusedAttribute">
			<intent-filter>
				<action android:name="android.intent.action.MAIN" />

				<category android:name="android.intent.category.DEFAULT" />
				<category android:name="org.godotengine.xr.hybrid.PANEL" />
			</intent-filter>
		</activity>
)",
				_bool_to_string(_get_bool_option("package/exclude_from_recents")),
				_get_android_orientation_label((DisplayServer::ScreenOrientation)(int)project_settings->get_setting_with_override("display/window/handheld/orientation")),
				_bool_to_string(project_settings->get_setting_with_override("display/window/size/resizable")));

		if (hybrid_launch_mode == OpenXRHybridApp::HYBRID_MODE_PANEL) {
			contents += R"(
		<activity-alias
			android:name="org.godotengine.openxr.vendors.GodotPanelAppLauncher"
			android:exported="true"
			android:targetActivity="org.godotengine.openxr.vendors.GodotPanelApp">
			<intent-filter>
				<action android:name="android.intent.action.MAIN" />

				<category android:name="android.intent.category.DEFAULT" />
				<category android:name="android.intent.category.LAUNCHER" />
			</intent-filter>
		</activity-alias>
)";
		}
	}

	return contents;
}

String AndroidXREditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	// Android XR required
	bool android_xr_required = _get_hybrid_app_launch_mode() != OpenXRHybridApp::HYBRID_MODE_PANEL;
	contents += vformat("    <uses-feature android:name=\"android.software.xr.api.openxr\" android:required=\"%s\" />\n", _bool_to_string(android_xr_required));

	int tracked_controllers_option = _get_int_option("android_xr_features/tracked_controllers", TRACKED_CONTROLLERS_NONE_VALUE);
	if (tracked_controllers_option > TRACKED_CONTROLLERS_NONE_VALUE) {
		// 6DoF motion controllers
		bool required = tracked_controllers_option == TRACKED_CONTROLLERS_REQUIRED_VALUE;
		contents += vformat("    <uses-feature android:name=\"android.hardware.xr.input.controller\" android:required=\"%s\" />\n", _bool_to_string(required));
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	// Check for eye tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
		contents += "    <uses-permission android:name=\"android.permission.EYE_TRACKING_FINE\" />\n";

		int eye_tracking_value = _get_int_option("android_xr_features/eye_tracking", EYE_TRACKING_OPTIONAL_VALUE);
		if (eye_tracking_value == EYE_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.eye_tracking\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.eye_tracking\" android:required=\"false\" />\n";
		}
	}

	// Check for face tracking
	if (FACE_TRACKING_REQUIRED_VALUE == _get_int_option("android_xr_features/face_tracking", FACE_TRACKING_OPTIONAL_VALUE)) {
		contents += "    <uses-permission android:name=\"android.permission.FACE_TRACKING\" />\n";
	}

	// Check for hand tracking
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
		contents += "    <uses-permission android:name=\"android.permission.HAND_TRACKING\" />\n";

		int hand_tracking_value = _get_int_option("android_xr_features/hand_tracking", HAND_TRACKING_OPTIONAL_VALUE);
		if (hand_tracking_value == HAND_TRACKING_REQUIRED_VALUE) {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.hand_tracking\" android:required=\"true\" />\n";
		} else {
			contents += "    <uses-feature android:name=\"android.hardware.xr.input.hand_tracking\" android:required=\"false\" />\n";
		}
	}

	// SCENE_UNDERSTANDING_FINE is required for depth texture and other texture access features.
	// When SCENE_UNDERSTANDING_FINE is requested, SCENE_UNDERSTANDING_COARSE is not needed.
	// Spatial entity and most of scene understanding features only require
	// SCENE_UNDERSTANDING_COARSE.
	if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/androidxr/environment_depth") || (bool)project_settings->get_setting_with_override("xr/openxr/extensions/androidxr/scene_meshing")) {
		contents += "    <uses-permission android:name=\"android.permission.SCENE_UNDERSTANDING_FINE\" />\n";
	} else if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/spatial_entity/enabled")) {
		contents += "    <uses-permission android:name=\"android.permission.SCENE_UNDERSTANDING_COARSE\" />\n";
	}

	return contents;
}
