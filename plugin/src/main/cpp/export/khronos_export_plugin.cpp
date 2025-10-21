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

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

KhronosEditorExportPlugin::KhronosEditorExportPlugin() {
	set_vendor_name(KHRONOS_VENDOR_NAME);

	_khronos_vendors_option = _generate_export_option(
			"khronos_xr_features/vendors",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"Other,HTC",
			PROPERTY_USAGE_DEFAULT,
			KHRONOS_VENDOR_OTHER,
			true);

	_tracker_option = _generate_export_option(
			"khronos_xr_features/htc/tracker",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Yes",
			PROPERTY_USAGE_DEFAULT,
			MANIFEST_FALSE_VALUE,
			false);
	_lip_expression_option = _generate_export_option(
			"khronos_xr_features/htc/lip_expression",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Yes",
			PROPERTY_USAGE_DEFAULT,
			MANIFEST_FALSE_VALUE,
			false);
}

void KhronosEditorExportPlugin::_bind_methods() {}

TypedArray<Dictionary> KhronosEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.append(_get_vendor_toggle_option());
	export_options.append(_khronos_vendors_option);

	export_options.append(_tracker_option);
	export_options.append(_lip_expression_option);

	return export_options;
}

Dictionary KhronosEditorExportPlugin::_get_export_options_overrides(const Ref<godot::EditorExportPlatform> &p_platform) const {
	if (!_supports_platform(p_platform)) {
		return Dictionary();
	}

	Dictionary overrides = OpenXRVendorsEditorExportPlugin::_get_export_options_overrides(p_platform);

	if (!_is_vendor_plugin_enabled()) {
		overrides["khronos_xr_features/vendors"] = KHRONOS_VENDOR_OTHER;
	}

	if (!_is_vendor_plugin_enabled() || !_is_khronos_htc_enabled()) {
		// Overrides with the options' default values.
		overrides["khronos_xr_features/htc/tracker"] = MANIFEST_FALSE_VALUE;
		overrides["khronos_xr_features/htc/lip_expression"] = MANIFEST_FALSE_VALUE;
	}

	return overrides;
}

String KhronosEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (_is_khronos_htc_enabled()) {
		if (option == "khronos_xr_features/htc/tracker") {
			if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
				return "\"Tracker\" requires \"XR Mode\" to be \"OpenXR\".\n";
			}
		} else if (option == "khronos_xr_features/htc/lip_expression") {
			if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
				return "\"Lip expression\" requires \"XR Mode\" to be \"OpenXR\".\n";
			}
		}
	}

	return OpenXRVendorsEditorExportPlugin::_get_export_option_warning(platform, option);
}

String KhronosEditorExportPlugin::_get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	contents += R"(
				<intent-filter>
					<action android:name="android.intent.action.MAIN" />

					<!-- OpenXR category tag to indicate the activity starts in an immersive OpenXR mode.
					See https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category. -->
					<category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />

)";

	contents += _get_common_activity_intent_filter_contents();

	if (_is_khronos_htc_enabled()) {
		contents += R"(
					<!-- Enable VR access on HTC Vive Focus devices. -->
					<category android:name="com.htc.intent.category.VRAPP" />

)";
	}

	contents += R"(
				</intent-filter>
)";
	return contents;
}

String KhronosEditorExportPlugin::_get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const {
	String contents;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return contents;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	if (_is_khronos_htc_enabled()) {
		// Check for hand tracking
		if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/hand_tracking")) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.handtracking\" android:required=\"true\" />\n";
		}

		// Check for tracker
		if (_get_int_option("khronos_xr_features/htc/tracker", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.tracker\" android:required=\"true\" />\n";
		}

		// Check for eye tracking
		if ((bool)project_settings->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction")) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.eyetracking\" android:required=\"true\" />\n";
		}

		// Check for lip expression
		if (_get_int_option("khronos_xr_features/htc/lip_expression", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.lipexpression\" android:required=\"true\" />\n";
		}
	}

	return contents;
}
