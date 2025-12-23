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

#include "extensions/openxr_fb_composition_layer_alpha_blend_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *ENABLE_ALPHA_BLEND_EXTENSION_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/enable_alpha_blend_extension";
static const char *SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/source_color_blend_factor";
static const char *DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/destination_color_blend_factor";
static const char *SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/source_alpha_blend_factor";
static const char *DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME = "XR_FB_composition_layer_alpha_blend/destination_alpha_blend_factor";

OpenXRFbCompositionLayerAlphaBlendExtensionWrapper *OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::singleton = nullptr;

OpenXRFbCompositionLayerAlphaBlendExtensionWrapper *OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerAlphaBlendExtensionWrapper());
	}
	return singleton;
}

OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::OpenXRFbCompositionLayerAlphaBlendExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerAlphaBlendExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_ALPHA_BLEND_EXTENSION_NAME] = &fb_composition_layer_alpha_blend;
	singleton = this;
}

OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::~OpenXRFbCompositionLayerAlphaBlendExtensionWrapper() {
	cleanup();
}

void OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::_bind_methods() {
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ZERO);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_DST_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_DST_ALPHA);
}

void OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::cleanup() {
	fb_composition_layer_alpha_blend = false;
}

Dictionary OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
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

	switch ((BlendFactor)(int)p_property_values.get(SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ONE)) {
		case BLEND_FACTOR_ZERO: {
			alpha_blend->srcFactorColor = XR_BLEND_FACTOR_ZERO_FB;
		} break;
		case BLEND_FACTOR_ONE: {
			alpha_blend->srcFactorColor = XR_BLEND_FACTOR_ONE_FB;
		} break;
		case BLEND_FACTOR_SRC_ALPHA: {
			alpha_blend->srcFactorColor = XR_BLEND_FACTOR_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: {
			alpha_blend->srcFactorColor = XR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_DST_ALPHA: {
			alpha_blend->srcFactorColor = XR_BLEND_FACTOR_DST_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_DST_ALPHA: {
			alpha_blend->srcFactorColor = XR_BLEND_FACTOR_ONE_MINUS_DST_ALPHA_FB;
		} break;
	}

	switch ((BlendFactor)(int)p_property_values.get(DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ZERO)) {
		case BLEND_FACTOR_ZERO: {
			alpha_blend->dstFactorColor = XR_BLEND_FACTOR_ZERO_FB;
		} break;
		case BLEND_FACTOR_ONE: {
			alpha_blend->dstFactorColor = XR_BLEND_FACTOR_ONE_FB;
		} break;
		case BLEND_FACTOR_SRC_ALPHA: {
			alpha_blend->dstFactorColor = XR_BLEND_FACTOR_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: {
			alpha_blend->dstFactorColor = XR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_DST_ALPHA: {
			alpha_blend->dstFactorColor = XR_BLEND_FACTOR_DST_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_DST_ALPHA: {
			alpha_blend->dstFactorColor = XR_BLEND_FACTOR_ONE_MINUS_DST_ALPHA_FB;
		} break;
	}

	switch ((BlendFactor)(int)p_property_values.get(SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ONE)) {
		case BLEND_FACTOR_ZERO: {
			alpha_blend->srcFactorAlpha = XR_BLEND_FACTOR_ZERO_FB;
		} break;
		case BLEND_FACTOR_ONE: {
			alpha_blend->srcFactorAlpha = XR_BLEND_FACTOR_ONE_FB;
		} break;
		case BLEND_FACTOR_SRC_ALPHA: {
			alpha_blend->srcFactorAlpha = XR_BLEND_FACTOR_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: {
			alpha_blend->srcFactorAlpha = XR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_DST_ALPHA: {
			alpha_blend->srcFactorAlpha = XR_BLEND_FACTOR_DST_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_DST_ALPHA: {
			alpha_blend->srcFactorAlpha = XR_BLEND_FACTOR_ONE_MINUS_DST_ALPHA_FB;
		} break;
	}

	switch ((BlendFactor)(int)p_property_values.get(DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME, BLEND_FACTOR_ZERO)) {
		case BLEND_FACTOR_ZERO: {
			alpha_blend->dstFactorAlpha = XR_BLEND_FACTOR_ZERO_FB;
		} break;
		case BLEND_FACTOR_ONE: {
			alpha_blend->dstFactorAlpha = XR_BLEND_FACTOR_ONE_FB;
		} break;
		case BLEND_FACTOR_SRC_ALPHA: {
			alpha_blend->dstFactorAlpha = XR_BLEND_FACTOR_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: {
			alpha_blend->dstFactorAlpha = XR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA_FB;
		} break;
		case BLEND_FACTOR_DST_ALPHA: {
			alpha_blend->dstFactorAlpha = XR_BLEND_FACTOR_DST_ALPHA_FB;
		} break;
		case BLEND_FACTOR_ONE_MINUS_DST_ALPHA: {
			alpha_blend->dstFactorAlpha = XR_BLEND_FACTOR_ONE_MINUS_DST_ALPHA_FB;
		} break;
	}

	alpha_blend->next = p_next_pointer;
	return reinterpret_cast<uint64_t>(alpha_blend);
}

void OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_alpha_blend) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::_get_viewport_composition_layer_extension_properties() {
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

Dictionary OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[ENABLE_ALPHA_BLEND_EXTENSION_PROPERTY_NAME] = false;
	defaults[SOURCE_COLOR_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ONE;
	defaults[DESTINATION_COLOR_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ZERO;
	defaults[SOURCE_ALPHA_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ONE;
	defaults[DESTINATION_ALPHA_BLEND_FACTOR_PROPERTY_NAME] = (int)BLEND_FACTOR_ZERO;
	return defaults;
}
