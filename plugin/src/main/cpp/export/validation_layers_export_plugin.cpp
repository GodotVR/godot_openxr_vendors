/**************************************************************************/
/*  validation_layers_export_plugin.cpp                                   */
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

#include "export/validation_layers_export_plugin.h"

#include <godot_cpp/classes/editor_export_platform_android.hpp>
#include <godot_cpp/classes/project_settings.hpp>

OpenXRValidationLayersEditorExportPlugin::OpenXRValidationLayersEditorExportPlugin() {
	{
		Dictionary info;
		info["name"] = "xr_features/enable_openxr_validation_layers";
		info["type"] = Variant::BOOL;
		info["hint"] = PROPERTY_HINT_NONE;
		info["hint_string"] = "";
		info["usage"] = PROPERTY_USAGE_DEFAULT;

		_enable_export_option["option"] = info;
		_enable_export_option["default_value"] = false;
		_enable_export_option["update_visibility"] = false;
	}
}

String OpenXRValidationLayersEditorExportPlugin::_get_name() const {
	return "OpenXRValidationLayers";
}

bool OpenXRValidationLayersEditorExportPlugin::_supports_platform(const Ref<EditorExportPlatform> &platform) const {
	return platform->is_class(EditorExportPlatformAndroid::get_class_static());
}

String OpenXRValidationLayersEditorExportPlugin::_get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const {
	if (!_supports_platform(platform)) {
		return "";
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL_V(project_settings, "");

	if (option == "xr_features/enable_openxr_validation_layers") {
		int debug_utils_level = project_settings->get_setting("xr/openxr/extensions/debug_utils");
		if ((bool)get_option(option) && debug_utils_level < 2) {
			return "Set XR -> OpenXR -> Extensions -> Debug Utils to \"Warning\" or higher in Project Settings for OpenXR validation messages to appear in Godot.";
		}
	}

	return "";
}

TypedArray<Dictionary> OpenXRValidationLayersEditorExportPlugin::_get_export_options(const Ref<EditorExportPlatform> &platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(platform)) {
		return export_options;
	}

	export_options.push_back(_enable_export_option);
	return export_options;
}

PackedStringArray OpenXRValidationLayersEditorExportPlugin::_get_android_libraries(const Ref<EditorExportPlatform> &platform, bool debug) const {
	PackedStringArray dependencies;
	if (!_supports_platform(platform)) {
		return dependencies;
	}

	if ((bool)get_option("xr_features/enable_openxr_validation_layers")) {
		const String debug_label = debug ? "debug" : "release";
		dependencies.append("res://addons/godotopenxrvendors/.bin/android/" + debug_label + "/openxr-validation-layers-" + debug_label + ".aar");
	}

	return dependencies;
}
