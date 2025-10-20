/**************************************************************************/
/*  export_plugin.cpp                                                     */
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

#include "export/export_plugin.h"

#include <godot_cpp/classes/editor_export_platform_android.hpp>
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

OpenXRVendorsEditorExportPlugin::OpenXRVendorsEditorExportPlugin() :
		_plugin_version(PLUGIN_VERSION) {}

void OpenXRVendorsEditorExportPlugin::_bind_methods() {}

Dictionary OpenXRVendorsEditorExportPlugin::_generate_export_option(const String &name, const String &class_name,
		Variant::Type type,
		PropertyHint property_hint,
		const String &hint_string,
		PropertyUsageFlags property_usage,
		const Variant &default_value,
		bool update_visibility) {
	Dictionary option_info;
	option_info["name"] = name;
	option_info["class_name"] = class_name;
	option_info["type"] = type;
	option_info["hint"] = property_hint;
	option_info["hint_string"] = hint_string;
	option_info["usage"] = property_usage;

	Dictionary export_option;
	export_option["option"] = option_info;
	export_option["default_value"] = default_value;
	export_option["update_visibility"] = update_visibility;

	return export_option;
}

String OpenXRVendorsEditorExportPlugin::_get_name() const {
	return "GodotOpenXR" + _vendor.capitalize();
}

String OpenXRVendorsEditorExportPlugin::_get_android_aar_file_path(bool debug) const {
	const String debug_label = debug ? "debug" : "release";
	return "res://addons/godotopenxrvendors/.bin/android/" + debug_label + "/godotopenxr-" + _vendor + "-" + debug_label + ".aar";
}

String OpenXRVendorsEditorExportPlugin::_get_android_maven_central_dependency() const {
	return "org.godotengine:godot-openxr-vendors-" + _vendor + ":" + _plugin_version;
}

String OpenXRVendorsEditorExportPlugin::_get_vendor_toggle_option_name(const String &vendor_name) const {
	return "xr_features/enable_" + vendor_name + "_plugin";
}

Dictionary OpenXRVendorsEditorExportPlugin::_get_vendor_toggle_option(const String &vendor_name) const {
	return _generate_export_option(
			_get_vendor_toggle_option_name(vendor_name),
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			true);
}

bool OpenXRVendorsEditorExportPlugin::_is_hybrid_app_enabled() const {
	return ProjectSettings::get_singleton()->get_setting_with_override("xr/hybrid_app/enabled");
}

OpenXRHybridApp::HybridMode OpenXRVendorsEditorExportPlugin::_get_hybrid_app_launch_mode() const {
	if (!_is_hybrid_app_enabled()) {
		return OpenXRHybridApp::HYBRID_MODE_NONE;
	}
	return (OpenXRHybridApp::HybridMode)(int)ProjectSettings::get_singleton()->get_setting_with_override("xr/hybrid_app/launch_mode");
}

String OpenXRVendorsEditorExportPlugin::_get_opening_activity_tag_for_panel_app() const {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	return vformat(
			"        <activity android:name=\"org.godotengine.openxr.vendors.GodotPanelApp\" "
			"android:process=\":GodotPanelApp\" "
			"android:theme=\"@style/GodotAppSplashTheme\" "
			"android:launchMode=\"singleInstancePerTask\" "
			"android:exported=\"true\" "
			"android:excludeFromRecents=\"%s\" "
			"android:screenOrientation=\"%s\" "
			"android:resizeableActivity=\"%s\" "
			"android:configChanges=\"orientation|keyboardHidden|screenSize|smallestScreenSize|density|keyboard|navigation|screenLayout|uiMode\" "
			"tools:ignore=\"UnusedAttribute\">\n",
			_bool_to_string(_get_bool_option("package/exclude_from_recents")),
			_get_android_orientation_label((DisplayServer::ScreenOrientation)(int)project_settings->get_setting_with_override("display/window/handheld/orientation")),
			_bool_to_string(project_settings->get_setting_with_override("display/window/size/resizable")));
}

String OpenXRVendorsEditorExportPlugin::_get_common_activity_intent_filter_contents() const {
	String contents;

	if (_get_bool_option("package/show_in_app_library")) {
		contents += R"(
						<category android:name="android.intent.category.LAUNCHER" />
)";
	}
	if (_get_bool_option("package/show_as_launcher_app")) {
		contents += R"(
						<category android:name="android.intent.category.HOME" />
)";
	}

	if (_is_hybrid_app_enabled()) {
		contents += R"(
						<category android:name="org.godotengine.xr.hybrid.IMMERSIVE" />
)";
	}
	return contents;
}

bool OpenXRVendorsEditorExportPlugin::_is_openxr_enabled() const {
	return _get_int_option("xr_features/xr_mode", REGULAR_MODE_VALUE) == OPENXR_MODE_VALUE;
}

String OpenXRVendorsEditorExportPlugin::_bool_to_string(bool p_value) const {
	return p_value ? "true" : "false";
}

String OpenXRVendorsEditorExportPlugin::_get_android_orientation_label(DisplayServer::ScreenOrientation screen_orientation) const {
	switch (screen_orientation) {
		case DisplayServer::SCREEN_PORTRAIT:
			return "portrait";
		case DisplayServer::SCREEN_REVERSE_LANDSCAPE:
			return "reverseLandscape";
		case DisplayServer::SCREEN_REVERSE_PORTRAIT:
			return "reversePortrait";
		case DisplayServer::SCREEN_SENSOR_LANDSCAPE:
			return "userLandscape";
		case DisplayServer::SCREEN_SENSOR_PORTRAIT:
			return "userPortrait";
		case DisplayServer::SCREEN_SENSOR:
			return "fullUser";
		case DisplayServer::SCREEN_LANDSCAPE:
		default:
			return "landscape";
	}
}

TypedArray<Dictionary> OpenXRVendorsEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	return export_options;
}

PackedStringArray OpenXRVendorsEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray features;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return features;
	}

	// Add the eye tracking feature if necessary
	if ((bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
		features.append(EYE_GAZE_INTERACTION_FEATURE);
	}

	return features;
}

String OpenXRVendorsEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform)) {
		return "";
	}

	if (option != _get_vendor_toggle_option_name()) {
		return "";
	}

	if (!_is_openxr_enabled() && _get_bool_option(option)) {
		return "\"Enable " + _vendor.capitalize() + " Plugin\" requires \"XR Mode\" to be \"OpenXR\".\n";
	}

	if (_is_vendor_plugin_enabled()) {
		for (const String vendor_name : VENDORS_LIST) {
			if (vendor_name != _vendor && _is_vendor_plugin_enabled(vendor_name)) {
				return "\"Disable " + _vendor.capitalize() + " Plugin before enabling another. Multiple plugins are not supported!\"";
			}
		}
	}

	return "";
}

Dictionary OpenXRVendorsEditorExportPlugin::_get_export_options_overrides(const Ref<EditorExportPlatform> &platform) const {
	Dictionary overrides;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return overrides;
	}

	OpenXRHybridApp::HybridMode hybrid_launch_mode = _get_hybrid_app_launch_mode();
	if (hybrid_launch_mode != OpenXRHybridApp::HYBRID_MODE_NONE) {
		// Unless this is a hybrid app that launches as a panel, then we want to force this option on.
		// Otherwise, it needs to be off, so that the panel activity can be the one that's launched by default.
		overrides["package/show_in_app_library"] = (hybrid_launch_mode != OpenXRHybridApp::HYBRID_MODE_PANEL);
	}

	return overrides;
}

bool OpenXRVendorsEditorExportPlugin::_supports_platform(const Ref<EditorExportPlatform> &platform) const {
	return platform->is_class(EditorExportPlatformAndroid::get_class_static());
}

bool OpenXRVendorsEditorExportPlugin::_get_bool_option(const String &option) const {
	Variant option_enabled = get_option(option);
	if (option_enabled.get_type() == Variant::Type::BOOL) {
		return option_enabled;
	}
	return false;
}

int OpenXRVendorsEditorExportPlugin::_get_int_option(const String &option, int default_value) const {
	Variant option_value = get_option(option);
	if (option_value.get_type() == Variant::Type::INT) {
		return option_value;
	}
	return default_value;
}

PackedStringArray OpenXRVendorsEditorExportPlugin::_get_android_dependencies(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray dependencies;
	if (!_supports_platform(platform)) {
		return dependencies;
	}

	if (_is_vendor_plugin_enabled() && !_is_android_aar_file_available(debug)) {
		dependencies.append(_get_android_maven_central_dependency());
	}

	return dependencies;
}

PackedStringArray OpenXRVendorsEditorExportPlugin::_get_android_libraries(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray dependencies;
	if (!_supports_platform(platform)) {
		return dependencies;
	}

	if (_is_vendor_plugin_enabled() && _is_android_aar_file_available(debug)) {
		dependencies.append(_get_android_aar_file_path(debug));
	}

	return dependencies;
}

PackedStringArray OpenXRVendorsEditorExportPlugin::_get_android_dependencies_maven_repos(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray maven_repos;
	if (!_supports_platform(platform)) {
		return maven_repos;
	}

	if (_is_vendor_plugin_enabled() && !_is_android_aar_file_available(debug) && _plugin_version.ends_with("-SNAPSHOT")) {
		maven_repos.append("https://central.sonatype.com/repository/maven-snapshots/");
	}
	return maven_repos;
}

String OpenXRVendorsEditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	String contents = R"(
				<intent-filter>
					<action android:name="android.intent.action.MAIN" />

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
