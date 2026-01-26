/**************************************************************************/
/*  openxr_fb_composition_layer_image_layout_extension_wrapper.cpp        */
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

#include "extensions/openxr_fb_composition_layer_image_layout_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

static const char *VERTICAL_FLIP_PROPERTY_NAME = "XR_FB_composition_layer_image_layout/vertical_flip";

OpenXRFbCompositionLayerImageLayoutExtensionWrapper *OpenXRFbCompositionLayerImageLayoutExtensionWrapper::singleton = nullptr;

OpenXRFbCompositionLayerImageLayoutExtensionWrapper *OpenXRFbCompositionLayerImageLayoutExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerImageLayoutExtensionWrapper());
	}
	return singleton;
}

OpenXRFbCompositionLayerImageLayoutExtensionWrapper::OpenXRFbCompositionLayerImageLayoutExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerImageLayoutExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_IMAGE_LAYOUT_EXTENSION_NAME] = &fb_composition_layer_image_layout;
	singleton = this;
}

OpenXRFbCompositionLayerImageLayoutExtensionWrapper::~OpenXRFbCompositionLayerImageLayoutExtensionWrapper() {
	cleanup();
}

void OpenXRFbCompositionLayerImageLayoutExtensionWrapper::_bind_methods() {
}

void OpenXRFbCompositionLayerImageLayoutExtensionWrapper::cleanup() {
	fb_composition_layer_image_layout = false;
}

Dictionary OpenXRFbCompositionLayerImageLayoutExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerImageLayoutExtensionWrapper::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
	if (!fb_composition_layer_image_layout) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);

	if (!layer_structs.has(layer)) {
		layer_structs[layer] = {
			XR_TYPE_COMPOSITION_LAYER_IMAGE_LAYOUT_FB, // type
			p_next_pointer, // next
		};
	}

	XrCompositionLayerImageLayoutFB *image_layout = layer_structs.getptr(layer);

	image_layout->next = p_next_pointer;

	if (p_property_values.get(VERTICAL_FLIP_PROPERTY_NAME, false)) {
		image_layout->flags = XR_COMPOSITION_LAYER_IMAGE_LAYOUT_VERTICAL_FLIP_BIT_FB;
	} else {
		image_layout->flags = 0;
	}

	return reinterpret_cast<uint64_t>(image_layout);
}

void OpenXRFbCompositionLayerImageLayoutExtensionWrapper::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_image_layout) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerImageLayoutExtensionWrapper::_get_viewport_composition_layer_extension_properties() {
	TypedArray<Dictionary> properties;

	{
		Dictionary vertical_flip;
		vertical_flip["name"] = VERTICAL_FLIP_PROPERTY_NAME;
		vertical_flip["type"] = Variant::BOOL;
		properties.push_back(vertical_flip);
	}

	return properties;
}

Dictionary OpenXRFbCompositionLayerImageLayoutExtensionWrapper::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[VERTICAL_FLIP_PROPERTY_NAME] = false;
	return defaults;
}
