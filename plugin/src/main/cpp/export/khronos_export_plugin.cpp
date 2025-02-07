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

using namespace godot;

void KhronosEditorPlugin::_bind_methods() {}

void KhronosEditorPlugin::_enter_tree() {
	// Initialize the editor export plugin
	khronos_export_plugin.instantiate();
	add_export_plugin(khronos_export_plugin);
}

void KhronosEditorPlugin::_exit_tree() {
	// Clean up the editor export plugin
	remove_export_plugin(khronos_export_plugin);

	khronos_export_plugin.unref();
}

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

	_hand_tracking_option = _generate_export_option(
			"khronos_xr_features/htc/hand_tracking",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Yes",
			PROPERTY_USAGE_DEFAULT,
			MANIFEST_FALSE_VALUE,
			false);
	_tracker_option = _generate_export_option(
			"khronos_xr_features/htc/tracker",
			"",
			Variant::Type::INT,
			PROPERTY_HINT_ENUM,
			"No,Yes",
			PROPERTY_USAGE_DEFAULT,
			MANIFEST_FALSE_VALUE,
			false);
	_eye_tracking_option = _generate_export_option(
			"khronos_xr_features/htc/eye_tracking",
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

	export_options.append(_hand_tracking_option);
	export_options.append(_tracker_option);
	export_options.append(_eye_tracking_option);
	export_options.append(_lip_expression_option);

	return export_options;
}

Dictionary KhronosEditorExportPlugin::_get_export_options_overrides(const Ref<godot::EditorExportPlatform> &p_platform) const {
	if (!_supports_platform(p_platform)) {
		return Dictionary();
	}

	Dictionary overrides = OpenXREditorExportPlugin::_get_export_options_overrides(p_platform);

	if (!_is_vendor_plugin_enabled()) {
		overrides["khronos_xr_features/vendors"] = KHRONOS_VENDOR_OTHER;
	}

	if (!_is_vendor_plugin_enabled() || !_is_khronos_htc_enabled()) {
		// Overrides with the options' default values.
		overrides["khronos_xr_features/htc/hand_tracking"] = MANIFEST_FALSE_VALUE;
		overrides["khronos_xr_features/htc/tracker"] = MANIFEST_FALSE_VALUE;
		overrides["khronos_xr_features/htc/eye_tracking"] = MANIFEST_FALSE_VALUE;
		overrides["khronos_xr_features/htc/lip_expression"] = MANIFEST_FALSE_VALUE;
	}

	return overrides;
}

PackedStringArray KhronosEditorExportPlugin::_get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray features;
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return features;
	}

	if (_is_khronos_htc_enabled()) {
		// Add the eye tracking feature if necessary
		if (_get_int_option("khronos_xr_features/htc/eye_tracking", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			features.append(EYE_GAZE_INTERACTION_FEATURE);
		}
	}

	return features;
}

String KhronosEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform) || !_is_vendor_plugin_enabled()) {
		return "";
	}

	bool openxr_enabled = _is_openxr_enabled();
	if (_is_khronos_htc_enabled()) {
		if (option == "khronos_xr_features/htc/hand_tracking") {
			if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
				return "\"Hand Tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
			}
		} else if (option == "khronos_xr_features/htc/tracker") {
			if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
				return "\"Tracker\" requires \"XR Mode\" to be \"OpenXR\".\n";
			}
		} else if (option == "khronos_xr_features/htc/eye_tracking") {
			if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
				return "\"Eye tracking\" requires \"XR Mode\" to be \"OpenXR\".\n";
			}
		} else if (option == "khronos_xr_features/htc/lip_expression") {
			if (!openxr_enabled && _get_int_option(option, MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
				return "\"Lip expression\" requires \"XR Mode\" to be \"OpenXR\".\n";
			}
		}
	}

	return OpenXREditorExportPlugin::_get_export_option_warning(platform, option);
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

	if (_is_khronos_htc_enabled()) {
		// Check for hand tracking
		if (_get_int_option("khronos_xr_features/htc/hand_tracking", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.handtracking\" android:required=\"true\" />\n";
		}

		// Check for tracker
		if (_get_int_option("khronos_xr_features/htc/tracker", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.tracker\" android:required=\"true\" />\n";
		}

		// Check for eye tracking
		if (_get_int_option("khronos_xr_features/htc/eye_tracking", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.eyetracking\" android:required=\"true\" />\n";
		}

		// Check for lip expression
		if (_get_int_option("khronos_xr_features/htc/lip_expression", MANIFEST_FALSE_VALUE) == MANIFEST_TRUE_VALUE) {
			contents += "    <uses-feature tools:node=\"replace\" android:name=\"wave.feature.lipexpression\" android:required=\"true\" />\n";
		}
	}

	return contents;
}
