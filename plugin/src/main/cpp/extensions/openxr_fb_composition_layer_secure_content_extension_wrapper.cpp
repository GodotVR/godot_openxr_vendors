/**************************************************************************/
/*  openxr_fb_composition_layer_secure_content_extension_wrapper.cpp      */
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

#include "extensions/openxr_fb_composition_layer_secure_content_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *EXTERNAL_OUTPUT_PROPERTY_NAME = "XR_FB_composition_layer_secure_content/external_output";

OpenXRFbCompositionLayerSecureContentExtensionWrapper *OpenXRFbCompositionLayerSecureContentExtensionWrapper::singleton = nullptr;

OpenXRFbCompositionLayerSecureContentExtensionWrapper *OpenXRFbCompositionLayerSecureContentExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerSecureContentExtensionWrapper());
	}
	return singleton;
}

OpenXRFbCompositionLayerSecureContentExtensionWrapper::OpenXRFbCompositionLayerSecureContentExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerSecureContentExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_SECURE_CONTENT_EXTENSION_NAME] = &fb_composition_layer_secure_content;
	singleton = this;
}

OpenXRFbCompositionLayerSecureContentExtensionWrapper::~OpenXRFbCompositionLayerSecureContentExtensionWrapper() {
	cleanup();
}

void OpenXRFbCompositionLayerSecureContentExtensionWrapper::_bind_methods() {
}

void OpenXRFbCompositionLayerSecureContentExtensionWrapper::cleanup() {
	fb_composition_layer_secure_content = false;
}

Dictionary OpenXRFbCompositionLayerSecureContentExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerSecureContentExtensionWrapper::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
	if (!fb_composition_layer_secure_content) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	ExternalOutput external_output = (ExternalOutput)(int)p_property_values.get(EXTERNAL_OUTPUT_PROPERTY_NAME, EXTERNAL_OUTPUT_DISPLAY);
	if (external_output == EXTERNAL_OUTPUT_DISPLAY) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);

	if (!layer_structs.has(layer)) {
		layer_structs[layer] = {
			XR_TYPE_COMPOSITION_LAYER_SECURE_CONTENT_FB, // type
			p_next_pointer, // next
			0, // flags
		};
	}

	XrCompositionLayerSecureContentFB *secure_content = layer_structs.getptr(layer);

	switch (external_output) {
		case EXTERNAL_OUTPUT_DISPLAY: {
			// We'll never reach this - it would have been handled above.
		} break;
		case EXTERNAL_OUTPUT_EXCLUDE: {
			secure_content->flags = XR_COMPOSITION_LAYER_SECURE_CONTENT_EXCLUDE_LAYER_BIT_FB;
		} break;
		case EXTERNAL_OUTPUT_REPLACE: {
			secure_content->flags = XR_COMPOSITION_LAYER_SECURE_CONTENT_REPLACE_LAYER_BIT_FB;
		} break;
	};

	secure_content->next = p_next_pointer;
	return reinterpret_cast<uint64_t>(secure_content);
}

void OpenXRFbCompositionLayerSecureContentExtensionWrapper::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_secure_content) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerSecureContentExtensionWrapper::_get_viewport_composition_layer_extension_properties() {
	TypedArray<Dictionary> properties;

	{
		Dictionary external_output;
		external_output["name"] = EXTERNAL_OUTPUT_PROPERTY_NAME;
		external_output["type"] = Variant::INT;
		external_output["hint"] = PROPERTY_HINT_ENUM;
		external_output["hint_string"] = "Display,Exclude,Replace";
		properties.push_back(external_output);
	}

	return properties;
}

Dictionary OpenXRFbCompositionLayerSecureContentExtensionWrapper::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[EXTERNAL_OUTPUT_PROPERTY_NAME] = (int)EXTERNAL_OUTPUT_DISPLAY;
	return defaults;
}
