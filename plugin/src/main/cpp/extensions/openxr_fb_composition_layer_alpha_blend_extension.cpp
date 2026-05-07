/**************************************************************************/
/*  openxr_fb_composition_layer_alpha_blend_extension_wrapper.cpp         */
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

#include "extensions/openxr_fb_composition_layer_alpha_blend_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *ENABLE_ALPHA_BLEND_EXTENSION_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/enable_alpha_blend_extension";
static const char *SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/source_color_blend_factor";
static const char *DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/destination_color_blend_factor";
static const char *SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/source_alpha_blend_factor";
static const char *DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/destination_alpha_blend_factor";

OpenXRFbCompositionLayerAlphaBlendExtension *OpenXRFbCompositionLayerAlphaBlendExtension::singleton = nullptr;

OpenXRFbCompositionLayerAlphaBlendExtension *OpenXRFbCompositionLayerAlphaBlendExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerAlphaBlendExtension());
	}
	return singleton;
}

OpenXRFbCompositionLayerAlphaBlendExtension::OpenXRFbCompositionLayerAlphaBlendExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerAlphaBlendExtension singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_ALPHA_BLEND_EXTENSION_NAME] = &fb_composition_layer_alpha_blend;
	singleton = this;
}

OpenXRFbCompositionLayerAlphaBlendExtension::~OpenXRFbCompositionLayerAlphaBlendExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRFbCompositionLayerAlphaBlendExtension::_bind_methods() {
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor >= 7) {
		ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbCompositionLayerAlphaBlendExtension::is_enabled);

		ClassDB::bind_method(D_METHOD("set_projection_layer_alpha_blend_enabled", "enabled"), &OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_alpha_blend_enabled);
		ClassDB::bind_method(D_METHOD("is_projection_layer_alpha_blend_enabled"), &OpenXRFbCompositionLayerAlphaBlendExtension::is_projection_layer_alpha_blend_enabled);

		ClassDB::bind_method(D_METHOD("set_projection_layer_source_color_blend_factor", "source_color_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_source_color_blend_factor);
		ClassDB::bind_method(D_METHOD("get_projection_layer_source_color_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_source_color_blend_factor);

		ClassDB::bind_method(D_METHOD("set_projection_layer_destination_color_blend_factor", "destination_color_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_destination_color_blend_factor);
		ClassDB::bind_method(D_METHOD("get_projection_layer_destination_color_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_destination_color_blend_factor);

		ClassDB::bind_method(D_METHOD("set_projection_layer_source_alpha_blend_factor", "source_alpha_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_source_alpha_blend_factor);
		ClassDB::bind_method(D_METHOD("get_projection_layer_source_alpha_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_source_alpha_blend_factor);

		ClassDB::bind_method(D_METHOD("set_projection_layer_destination_alpha_blend_factor", "destination_alpha_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_destination_alpha_blend_factor);
		ClassDB::bind_method(D_METHOD("get_projection_layer_destination_alpha_blend_factor"), &OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_destination_alpha_blend_factor);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "projection_layer_alpha_blend_enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_projection_layer_alpha_blend_enabled", "is_projection_layer_alpha_blend_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_layer_source_color_blend_factor", PROPERTY_HINT_ENUM, "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha", PROPERTY_USAGE_NONE), "set_projection_layer_source_color_blend_factor", "get_projection_layer_source_color_blend_factor");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_layer_destination_color_blend_factor", PROPERTY_HINT_ENUM, "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha", PROPERTY_USAGE_NONE), "set_projection_layer_destination_color_blend_factor", "get_projection_layer_destination_color_blend_factor");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_layer_source_alpha_blend_factor", PROPERTY_HINT_ENUM, "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha", PROPERTY_USAGE_NONE), "set_projection_layer_source_alpha_blend_factor", "get_projection_layer_source_alpha_blend_factor");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_layer_destination_alpha_blend_factor", PROPERTY_HINT_ENUM, "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha", PROPERTY_USAGE_NONE), "set_projection_layer_destination_alpha_blend_factor", "get_projection_layer_destination_alpha_blend_factor");
	}

	BIND_ENUM_CONSTANT(BLEND_FACTOR_ZERO);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_DST_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_DST_ALPHA);
}

void OpenXRFbCompositionLayerAlphaBlendExtension::_on_session_created(uint64_t p_session) {
	if (!fb_composition_layer_alpha_blend) {
		return;
	}
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor >= 7) {
		get_openxr_api()->register_projection_layer_extension(this);
	}
}

void OpenXRFbCompositionLayerAlphaBlendExtension::_on_session_destroyed() {
	if (!fb_composition_layer_alpha_blend) {
		return;
	}
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor >= 7) {
		get_openxr_api()->unregister_projection_layer_extension(this);
	}
}

void OpenXRFbCompositionLayerAlphaBlendExtension::_on_state_ready() {
	// Only works with Godot 4.7 or later.
	if (godot::gdextension_interface::godot_version.minor < 7) {
		return;
	}

	if (!fb_composition_layer_alpha_blend) {
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	set_projection_layer_alpha_blend_enabled(project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/alpha_blend/enable"));
	set_projection_layer_source_color_blend_factor(BlendFactor((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/alpha_blend/source_color_blend_factor")));
	set_projection_layer_destination_color_blend_factor(BlendFactor((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/alpha_blend/destination_color_blend_factor")));
	set_projection_layer_source_alpha_blend_factor(BlendFactor((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/alpha_blend/source_alpha_blend_factor")));
	set_projection_layer_destination_alpha_blend_factor(BlendFactor((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/alpha_blend/destination_alpha_blend_factor")));
}

void OpenXRFbCompositionLayerAlphaBlendExtension::cleanup() {
	fb_composition_layer_alpha_blend = false;
}

Dictionary OpenXRFbCompositionLayerAlphaBlendExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

bool OpenXRFbCompositionLayerAlphaBlendExtension::is_enabled() const {
	return fb_composition_layer_alpha_blend;
}

void OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_alpha_blend_enabled(bool p_enabled) {
	ERR_FAIL_COND_MSG(!fb_composition_layer_alpha_blend, "XR_FB_composition_layer_alpha_blend is not enabled");
	projection_layer_alpha_blend_enabled = p_enabled;
}

bool OpenXRFbCompositionLayerAlphaBlendExtension::is_projection_layer_alpha_blend_enabled() const {
	return fb_composition_layer_alpha_blend && projection_layer_alpha_blend_enabled;
}

void OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_source_color_blend_factor(OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor p_source_color_blend_factor) {
	ERR_FAIL_COND_MSG(!fb_composition_layer_alpha_blend, "XR_FB_composition_layer_alpha_blend is not enabled");
	projection_layer_source_color_blend_factor = p_source_color_blend_factor;
}

OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_source_color_blend_factor() const {
	return projection_layer_source_color_blend_factor;
}

void OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_destination_color_blend_factor(OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor p_destination_color_blend_factor) {
	ERR_FAIL_COND_MSG(!fb_composition_layer_alpha_blend, "XR_FB_composition_layer_alpha_blend is not enabled");
	projection_layer_destination_color_blend_factor = p_destination_color_blend_factor;
}

OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_destination_color_blend_factor() const {
	return projection_layer_destination_color_blend_factor;
}

void OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_source_alpha_blend_factor(OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor p_source_alpha_blend_factor) {
	ERR_FAIL_COND_MSG(!fb_composition_layer_alpha_blend, "XR_FB_composition_layer_alpha_blend is not enabled");
	projection_layer_source_alpha_blend_factor = p_source_alpha_blend_factor;
}

OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_source_alpha_blend_factor() const {
	return projection_layer_source_alpha_blend_factor;
}

void OpenXRFbCompositionLayerAlphaBlendExtension::set_projection_layer_destination_alpha_blend_factor(OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor p_destination_alpha_blend_factor) {
	ERR_FAIL_COND_MSG(!fb_composition_layer_alpha_blend, "XR_FB_composition_layer_alpha_blend is not enabled");
	projection_layer_destination_alpha_blend_factor = p_destination_alpha_blend_factor;
}

OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor OpenXRFbCompositionLayerAlphaBlendExtension::get_projection_layer_destination_alpha_blend_factor() const {
	return projection_layer_destination_alpha_blend_factor;
}

XrBlendFactorFB OpenXRFbCompositionLayerAlphaBlendExtension::_from_blend_factor(OpenXRFbCompositionLayerAlphaBlendExtension::BlendFactor p_blend_factor) const {
	switch (p_blend_factor) {
		case BLEND_FACTOR_ZERO:
		default:
			return XR_BLEND_FACTOR_ZERO_FB;
		case BLEND_FACTOR_ONE:
			return XR_BLEND_FACTOR_ONE_FB;
		case BLEND_FACTOR_SRC_ALPHA:
			return XR_BLEND_FACTOR_SRC_ALPHA_FB;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
			return XR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA_FB;
		case BLEND_FACTOR_DST_ALPHA:
			return XR_BLEND_FACTOR_DST_ALPHA_FB;
		case BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
			return XR_BLEND_FACTOR_ONE_MINUS_DST_ALPHA_FB;
	}
}

uint64_t OpenXRFbCompositionLayerAlphaBlendExtension::_set_projection_layer_and_get_next_pointer(void *p_next_pointer) {
	if (!fb_composition_layer_alpha_blend || !projection_layer_alpha_blend_enabled) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	projection_layer_alpha_blend.next = p_next_pointer;

	projection_layer_alpha_blend.srcFactorColor = _from_blend_factor(projection_layer_source_color_blend_factor);
	projection_layer_alpha_blend.dstFactorColor = _from_blend_factor(projection_layer_destination_color_blend_factor);
	projection_layer_alpha_blend.srcFactorAlpha = _from_blend_factor(projection_layer_source_alpha_blend_factor);
	projection_layer_alpha_blend.dstFactorAlpha = _from_blend_factor(projection_layer_destination_alpha_blend_factor);

	return reinterpret_cast<uint64_t>(&projection_layer_alpha_blend);
}

uint64_t OpenXRFbCompositionLayerAlphaBlendExtension::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
	if (!fb_composition_layer_alpha_blend || !p_property_values.get(ENABLE_ALPHA_BLEND_EXTENSION_PROPERTY_NAME, false)) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);

	if (!layer_structs.has(layer)) {
		layer_structs[layer] = {
			XR_TYPE_COMPOSITION_LAYER_ALPHA_BLEND_FB, // type
			p_next_pointer, // next
		};
	}

	XrCompositionLayerAlphaBlendFB *alpha_blend = layer_structs.getptr(layer);

	alpha_blend->srcFactorColor = _from_blend_factor((BlendFactor)(int)p_property_values.get(SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ONE));
	alpha_blend->dstFactorColor = _from_blend_factor((BlendFactor)(int)p_property_values.get(DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ZERO));
	alpha_blend->srcFactorAlpha = _from_blend_factor((BlendFactor)(int)p_property_values.get(SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ONE));
	alpha_blend->dstFactorAlpha = _from_blend_factor((BlendFactor)(int)p_property_values.get(DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ZERO));

	alpha_blend->next = p_next_pointer;
	return reinterpret_cast<uint64_t>(alpha_blend);
}

void OpenXRFbCompositionLayerAlphaBlendExtension::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_alpha_blend) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerAlphaBlendExtension::_get_viewport_composition_layer_extension_properties() {
	TypedArray<Dictionary> properties;

	{
		Dictionary enable_alpha_blend_extension;
		enable_alpha_blend_extension["name"] = ENABLE_ALPHA_BLEND_EXTENSION_PROPERTY_NAME;
		enable_alpha_blend_extension["type"] = Variant::BOOL;
		properties.push_back(enable_alpha_blend_extension);
	}

	{
		Dictionary source_color_blend_factor;
		source_color_blend_factor["name"] = SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME;
		source_color_blend_factor["type"] = Variant::INT;
		source_color_blend_factor["hint"] = PROPERTY_HINT_ENUM;
		source_color_blend_factor["hint_string"] = "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha";
		properties.push_back(source_color_blend_factor);
	}

	{
		Dictionary destination_color_blend_factor;
		destination_color_blend_factor["name"] = DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME;
		destination_color_blend_factor["type"] = Variant::INT;
		destination_color_blend_factor["hint"] = PROPERTY_HINT_ENUM;
		destination_color_blend_factor["hint_string"] = "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha";
		properties.push_back(destination_color_blend_factor);
	}

	{
		Dictionary source_alpha_blend_factor;
		source_alpha_blend_factor["name"] = SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME;
		source_alpha_blend_factor["type"] = Variant::INT;
		source_alpha_blend_factor["hint"] = PROPERTY_HINT_ENUM;
		source_alpha_blend_factor["hint_string"] = "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha";
		properties.push_back(source_alpha_blend_factor);
	}

	{
		Dictionary destination_alpha_blend_factor;
		destination_alpha_blend_factor["name"] = DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME;
		destination_alpha_blend_factor["type"] = Variant::INT;
		destination_alpha_blend_factor["hint"] = PROPERTY_HINT_ENUM;
		destination_alpha_blend_factor["hint_string"] = "Zero,One,Source Alpha,One Minus Source Alpha,Destination Alpha,One Minus Destination Alpha";
		properties.push_back(destination_alpha_blend_factor);
	}

	return properties;
}

Dictionary OpenXRFbCompositionLayerAlphaBlendExtension::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[ENABLE_ALPHA_BLEND_EXTENSION_PROPERTY_NAME] = false;
	defaults[SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ONE;
	defaults[DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ZERO;
	defaults[SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ONE;
	defaults[DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ZERO;
	return defaults;
}
