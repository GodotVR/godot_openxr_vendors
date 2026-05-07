/**************************************************************************/
/*  openxr_fb_composition_layer_settings_extension_wrapper.cpp            */
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

#include "extensions/openxr_fb_composition_layer_settings_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *SUPERSAMPLING_MODE_PROPERTY_NAME = "XR_FB_composition_layer_settings/manual/supersampling_mode";
static const char *SHARPENING_MODE_PROPERTY_NAME = "XR_FB_composition_layer_settings/manual/sharpening_mode";
static const char *ENABLE_AUTO_FILTER_PROPERTY_NAME = "XR_FB_composition_layer_settings/auto/enable_auto_filter";
static const char *AUTO_OPTIONS_PROPERTY_NAME = "XR_FB_composition_layer_settings/auto/options";

OpenXRFbCompositionLayerSettingsExtension *OpenXRFbCompositionLayerSettingsExtension::singleton = nullptr;

OpenXRFbCompositionLayerSettingsExtension *OpenXRFbCompositionLayerSettingsExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerSettingsExtension());
	}
	return singleton;
}

OpenXRFbCompositionLayerSettingsExtension::OpenXRFbCompositionLayerSettingsExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerSettingsExtension singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_SETTINGS_EXTENSION_NAME] = &fb_composition_layer_settings;
	request_extensions[XR_META_AUTOMATIC_LAYER_FILTER_EXTENSION_NAME] = &meta_automatic_layer_filter;

	singleton = this;
}

OpenXRFbCompositionLayerSettingsExtension::~OpenXRFbCompositionLayerSettingsExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRFbCompositionLayerSettingsExtension::_bind_methods() {
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor >= 7) {
		ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbCompositionLayerSettingsExtension::is_enabled);

		ClassDB::bind_method(D_METHOD("set_projection_layer_auto_filter_enabled", "enabled"), &OpenXRFbCompositionLayerSettingsExtension::set_projection_layer_auto_filter_enabled);
		ClassDB::bind_method(D_METHOD("is_projection_layer_auto_filter_enabled"), &OpenXRFbCompositionLayerSettingsExtension::is_projection_layer_auto_filter_enabled);

		ClassDB::bind_method(D_METHOD("set_projection_layer_supersampling_mode", "supersampling_mode"), &OpenXRFbCompositionLayerSettingsExtension::set_projection_layer_supersampling_mode);
		ClassDB::bind_method(D_METHOD("get_projection_layer_supersampling_mode"), &OpenXRFbCompositionLayerSettingsExtension::get_projection_layer_supersampling_mode);

		ClassDB::bind_method(D_METHOD("set_projection_layer_sharpening_mode", "sharpening_mode"), &OpenXRFbCompositionLayerSettingsExtension::set_projection_layer_sharpening_mode);
		ClassDB::bind_method(D_METHOD("get_projection_layer_sharpening_mode"), &OpenXRFbCompositionLayerSettingsExtension::get_projection_layer_sharpening_mode);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "projection_layer_auto_filter_enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_projection_layer_auto_filter_enabled", "is_projection_layer_auto_filter_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_layer_supersampling_mode", PROPERTY_HINT_ENUM, "Disabled,Normal,Quality", PROPERTY_USAGE_NONE), "set_projection_layer_supersampling_mode", "get_projection_layer_supersampling_mode");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_layer_sharpening_mode", PROPERTY_HINT_ENUM, "Disabled,Normal,Quality", PROPERTY_USAGE_NONE), "set_projection_layer_sharpening_mode", "get_projection_layer_sharpening_mode");
	}

	BIND_ENUM_CONSTANT(SUPERSAMPLING_MODE_DISABLED);
	BIND_ENUM_CONSTANT(SUPERSAMPLING_MODE_NORMAL);
	BIND_ENUM_CONSTANT(SUPERSAMPLING_MODE_QUALITY);

	BIND_ENUM_CONSTANT(SHARPENING_MODE_DISABLED);
	BIND_ENUM_CONSTANT(SHARPENING_MODE_NORMAL);
	BIND_ENUM_CONSTANT(SHARPENING_MODE_QUALITY);
}

bool OpenXRFbCompositionLayerSettingsExtension::is_enabled() const {
	return fb_composition_layer_settings;
}

void OpenXRFbCompositionLayerSettingsExtension::set_projection_layer_auto_filter_enabled(bool p_enabled) {
	ERR_FAIL_COND_MSG(!is_enabled(), "XR_FB_composition_layer_settings is not enabled");
	projection_layer_auto_filter_enabled = p_enabled;
}

bool OpenXRFbCompositionLayerSettingsExtension::is_projection_layer_auto_filter_enabled() const {
	return is_enabled() && projection_layer_auto_filter_enabled;
}

void OpenXRFbCompositionLayerSettingsExtension::set_projection_layer_sharpening_mode(OpenXRFbCompositionLayerSettingsExtension::SharpeningMode p_sharpening_mode) {
	ERR_FAIL_COND_MSG(!is_enabled(), "XR_FB_composition_layer_settings is not enabled");
	projection_layer_sharpening_mode = p_sharpening_mode;
}

OpenXRFbCompositionLayerSettingsExtension::SharpeningMode OpenXRFbCompositionLayerSettingsExtension::get_projection_layer_sharpening_mode() const {
	return projection_layer_sharpening_mode;
}

void OpenXRFbCompositionLayerSettingsExtension::set_projection_layer_supersampling_mode(OpenXRFbCompositionLayerSettingsExtension::SupersamplingMode p_supersampling_mode) {
	ERR_FAIL_COND_MSG(!is_enabled(), "XR_FB_composition_layer_settings is not enabled");
	projection_layer_supersampling_mode = p_supersampling_mode;
}

OpenXRFbCompositionLayerSettingsExtension::SupersamplingMode OpenXRFbCompositionLayerSettingsExtension::get_projection_layer_supersampling_mode() const {
	return projection_layer_supersampling_mode;
}

void OpenXRFbCompositionLayerSettingsExtension::_on_session_created(uint64_t p_session) {
	if (!is_enabled()) {
		return;
	}
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor >= 7) {
		get_openxr_api()->register_projection_layer_extension(this);
	}
}

void OpenXRFbCompositionLayerSettingsExtension::_on_session_destroyed() {
	if (!is_enabled()) {
		return;
	}
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor >= 7) {
		get_openxr_api()->unregister_projection_layer_extension(this);
	}
}

void OpenXRFbCompositionLayerSettingsExtension::_on_state_ready() {
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor < 7) {
		return;
	}

	if (!is_enabled()) {
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	set_projection_layer_auto_filter_enabled(project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/filtering/enable_auto_filtering"));
	set_projection_layer_sharpening_mode(SharpeningMode((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/filtering/sharpening_mode")));
	set_projection_layer_supersampling_mode(SupersamplingMode((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/filtering/supersampling_mode")));
}

void OpenXRFbCompositionLayerSettingsExtension::cleanup() {
	fb_composition_layer_settings = false;
	meta_automatic_layer_filter = false;
}

XrCompositionLayerSettingsFlagsFB OpenXRFbCompositionLayerSettingsExtension::_from_sharpening_mode(OpenXRFbCompositionLayerSettingsExtension::SharpeningMode p_sharpening_mode) const {
	switch (p_sharpening_mode) {
		case SHARPENING_MODE_DISABLED:
		default:
			return 0;
		case SHARPENING_MODE_NORMAL:
			return XR_COMPOSITION_LAYER_SETTINGS_NORMAL_SHARPENING_BIT_FB;
		case SHARPENING_MODE_QUALITY:
			return XR_COMPOSITION_LAYER_SETTINGS_QUALITY_SHARPENING_BIT_FB;
	}
}

XrCompositionLayerSettingsFlagsFB OpenXRFbCompositionLayerSettingsExtension::_from_supersampling_mode(OpenXRFbCompositionLayerSettingsExtension::SupersamplingMode p_supersampling_mode) const {
	switch (p_supersampling_mode) {
		case SUPERSAMPLING_MODE_DISABLED:
		default:
			return 0;
		case SUPERSAMPLING_MODE_NORMAL:
			return XR_COMPOSITION_LAYER_SETTINGS_NORMAL_SUPER_SAMPLING_BIT_FB;
		case SUPERSAMPLING_MODE_QUALITY:
			return XR_COMPOSITION_LAYER_SETTINGS_QUALITY_SUPER_SAMPLING_BIT_FB;
	}
}

Dictionary OpenXRFbCompositionLayerSettingsExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerSettingsExtension::_set_projection_layer_and_get_next_pointer(void *p_next_pointer) {
	if (!is_enabled()) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	projection_layer_settings.layerFlags = 0;
	projection_layer_settings.layerFlags |= _from_sharpening_mode(projection_layer_sharpening_mode);
	projection_layer_settings.layerFlags |= _from_supersampling_mode(projection_layer_supersampling_mode);

	if (projection_layer_settings.layerFlags == 0) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	if (meta_automatic_layer_filter && projection_layer_auto_filter_enabled) {
		projection_layer_settings.layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_AUTO_LAYER_FILTER_BIT_META;
	}

	projection_layer_settings.next = p_next_pointer;
	return reinterpret_cast<uint64_t>(&projection_layer_settings);
}

uint64_t OpenXRFbCompositionLayerSettingsExtension::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
	if (!fb_composition_layer_settings) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);

	if (!layer_structs.has(layer)) {
		layer_structs[layer] = {
			XR_TYPE_COMPOSITION_LAYER_SETTINGS_FB, // type
			p_next_pointer, // next
			0, // layerFlags
		};
	}

	XrCompositionLayerSettingsFB *settings = layer_structs.getptr(layer);

	settings->layerFlags = 0;

	// Auto will always take priority over manual if auto is enabled and at least one auto option flag is selected.
	if (meta_automatic_layer_filter && p_property_values.get(ENABLE_AUTO_FILTER_PROPERTY_NAME, false) && (int)p_property_values.get(AUTO_OPTIONS_PROPERTY_NAME, 0)) {
		settings->layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_AUTO_LAYER_FILTER_BIT_META;
		settings->layerFlags |= (int)p_property_values.get(AUTO_OPTIONS_PROPERTY_NAME, 0);

		return reinterpret_cast<uint64_t>(settings);
	}

	settings->layerFlags |= _from_supersampling_mode((SupersamplingMode)(int)p_property_values.get(SUPERSAMPLING_MODE_PROPERTY_NAME, SUPERSAMPLING_MODE_DISABLED));
	settings->layerFlags |= _from_sharpening_mode((SharpeningMode)(int)p_property_values.get(SHARPENING_MODE_PROPERTY_NAME, SHARPENING_MODE_DISABLED));

	if (settings->layerFlags == 0) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	settings->next = p_next_pointer;
	return reinterpret_cast<uint64_t>(settings);
}

void OpenXRFbCompositionLayerSettingsExtension::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_settings) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerSettingsExtension::_get_viewport_composition_layer_extension_properties() {
	TypedArray<Dictionary> properties;

	{
		Dictionary supersampling_mode;
		supersampling_mode["name"] = SUPERSAMPLING_MODE_PROPERTY_NAME;
		supersampling_mode["type"] = Variant::INT;
		supersampling_mode["hint"] = PROPERTY_HINT_ENUM;
		supersampling_mode["hint_string"] = "Disabled,Normal,Quality";
		properties.push_back(supersampling_mode);
	}

	{
		Dictionary sharpening_mode;
		sharpening_mode["name"] = SHARPENING_MODE_PROPERTY_NAME;
		sharpening_mode["type"] = Variant::INT;
		sharpening_mode["hint"] = PROPERTY_HINT_ENUM;
		sharpening_mode["hint_string"] = "Disabled,Normal,Quality";
		properties.push_back(sharpening_mode);
	}

	{
		Dictionary enable_auto_filter;
		enable_auto_filter["name"] = ENABLE_AUTO_FILTER_PROPERTY_NAME;
		enable_auto_filter["type"] = Variant::BOOL;
		enable_auto_filter["hint"] = PROPERTY_HINT_NONE;
		properties.push_back(enable_auto_filter);
	}

	{
		Dictionary auto_options;
		auto_options["name"] = AUTO_OPTIONS_PROPERTY_NAME;
		auto_options["type"] = Variant::INT;
		auto_options["hint"] = PROPERTY_HINT_FLAGS;
		auto_options["hint_string"] = "Normal Supersampling,Quality Supersampling,Normal Sharpening,Quality Sharpening";
		properties.push_back(auto_options);
	}

	return properties;
}

Dictionary OpenXRFbCompositionLayerSettingsExtension::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[SUPERSAMPLING_MODE_PROPERTY_NAME] = (int)SUPERSAMPLING_MODE_DISABLED;
	defaults[SHARPENING_MODE_PROPERTY_NAME] = (int)SHARPENING_MODE_DISABLED;
	defaults[ENABLE_AUTO_FILTER_PROPERTY_NAME] = false;
	defaults[AUTO_OPTIONS_PROPERTY_NAME] = 0;
	return defaults;
}
