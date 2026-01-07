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

#include "extensions/openxr_fb_composition_layer_settings_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *SUPERSAMPLING_MODE_PROPERTY_NAME = "XR_FB_composition_layer_settings/manual/supersampling_mode";
static const char *SHARPENING_MODE_PROPERTY_NAME = "XR_FB_composition_layer_settings/manual/sharpening_mode";
static const char *ENABLE_AUTO_FILTER_PROPERTY_NAME = "XR_FB_composition_layer_settings/auto/enable_auto_filter";
static const char *AUTO_OPTIONS_PROPERTY_NAME = "XR_FB_composition_layer_settings/auto/options";

OpenXRFbCompositionLayerSettingsExtensionWrapper *OpenXRFbCompositionLayerSettingsExtensionWrapper::singleton = nullptr;

OpenXRFbCompositionLayerSettingsExtensionWrapper *OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerSettingsExtensionWrapper());
	}
	return singleton;
}

OpenXRFbCompositionLayerSettingsExtensionWrapper::OpenXRFbCompositionLayerSettingsExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerSettingsExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_SETTINGS_EXTENSION_NAME] = &fb_composition_layer_settings;
	request_extensions[XR_META_AUTOMATIC_LAYER_FILTER_EXTENSION_NAME] = &meta_automatic_layer_filter;

	singleton = this;
}

OpenXRFbCompositionLayerSettingsExtensionWrapper::~OpenXRFbCompositionLayerSettingsExtensionWrapper() {
	cleanup();
}

void OpenXRFbCompositionLayerSettingsExtensionWrapper::_bind_methods() {
	BIND_ENUM_CONSTANT(SUPERSAMPLING_MODE_DISABLED);
	BIND_ENUM_CONSTANT(SUPERSAMPLING_MODE_NORMAL);
	BIND_ENUM_CONSTANT(SUPERSAMPLING_MODE_QUALITY);

	BIND_ENUM_CONSTANT(SHARPENING_MODE_DISABLED);
	BIND_ENUM_CONSTANT(SHARPENING_MODE_NORMAL);
	BIND_ENUM_CONSTANT(SHARPENING_MODE_QUALITY);
}

void OpenXRFbCompositionLayerSettingsExtensionWrapper::cleanup() {
	fb_composition_layer_settings = false;
	meta_automatic_layer_filter = false;
}

Dictionary OpenXRFbCompositionLayerSettingsExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerSettingsExtensionWrapper::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
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

	switch ((SupersamplingMode)(int)p_property_values.get(SUPERSAMPLING_MODE_PROPERTY_NAME, SUPERSAMPLING_MODE_DISABLED)) {
		case SUPERSAMPLING_MODE_NORMAL: {
			settings->layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_NORMAL_SUPER_SAMPLING_BIT_FB;
		} break;
		case SUPERSAMPLING_MODE_QUALITY: {
			settings->layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_QUALITY_SUPER_SAMPLING_BIT_FB;
		} break;
		case SUPERSAMPLING_MODE_DISABLED: {
			// Do not enable any supersampling mode flags.
		} break;
	}

	switch ((SharpeningMode)(int)p_property_values.get(SHARPENING_MODE_PROPERTY_NAME, SHARPENING_MODE_DISABLED)) {
		case SHARPENING_MODE_NORMAL: {
			settings->layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_NORMAL_SHARPENING_BIT_FB;
		} break;
		case SHARPENING_MODE_QUALITY: {
			settings->layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_QUALITY_SHARPENING_BIT_FB;
		} break;
		case SHARPENING_MODE_DISABLED: {
			// Do not enable any sharpening mode flags.
		} break;
	}

	if (settings->layerFlags == 0) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	settings->next = p_next_pointer;
	return reinterpret_cast<uint64_t>(settings);
}

void OpenXRFbCompositionLayerSettingsExtensionWrapper::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_settings) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerSettingsExtensionWrapper::_get_viewport_composition_layer_extension_properties() {
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

Dictionary OpenXRFbCompositionLayerSettingsExtensionWrapper::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[SUPERSAMPLING_MODE_PROPERTY_NAME] = (int)SUPERSAMPLING_MODE_DISABLED;
	defaults[SHARPENING_MODE_PROPERTY_NAME] = (int)SHARPENING_MODE_DISABLED;
	defaults[ENABLE_AUTO_FILTER_PROPERTY_NAME] = false;
	defaults[AUTO_OPTIONS_PROPERTY_NAME] = 0;
	return defaults;
}
