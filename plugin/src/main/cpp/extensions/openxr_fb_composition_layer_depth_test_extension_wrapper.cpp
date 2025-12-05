/**************************************************************************/
/*  openxr_fb_composition_layer_depth_test_extension_wrapper.cpp          */
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

#include "extensions/openxr_fb_composition_layer_depth_test_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *ENABLE_PROPERTY_NAME = "XR_FB_composition_layer_depth_test/enable";

OpenXRFbCompositionLayerDepthTestExtensionWrapper *OpenXRFbCompositionLayerDepthTestExtensionWrapper::singleton = nullptr;

OpenXRFbCompositionLayerDepthTestExtensionWrapper *OpenXRFbCompositionLayerDepthTestExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerDepthTestExtensionWrapper());
	}
	return singleton;
}

OpenXRFbCompositionLayerDepthTestExtensionWrapper::OpenXRFbCompositionLayerDepthTestExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerDepthTestExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_DEPTH_TEST_EXTENSION_NAME] = &fb_composition_layer_depth_test_ext;
	singleton = this;
}

OpenXRFbCompositionLayerDepthTestExtensionWrapper::~OpenXRFbCompositionLayerDepthTestExtensionWrapper() {
	cleanup();
}

void OpenXRFbCompositionLayerDepthTestExtensionWrapper::_bind_methods() {
}

void OpenXRFbCompositionLayerDepthTestExtensionWrapper::cleanup() {
	fb_composition_layer_depth_test_ext = false;
}

Dictionary OpenXRFbCompositionLayerDepthTestExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerDepthTestExtensionWrapper::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
	if (!fb_composition_layer_depth_test_ext || !(bool)p_property_values.get(ENABLE_PROPERTY_NAME, false)) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);

	if (!layer_structs.has(layer)) {
		layer_structs[layer] = {
			XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB, // type
			p_next_pointer, // next
			true, // depthMask
			XR_COMPARE_OP_LESS_FB // compareOp - Less depth = closer to the screen = keep this fragment
		};
	}

	XrCompositionLayerDepthTestFB *depth_test = layer_structs.getptr(layer);
	depth_test->next = p_next_pointer;
	return reinterpret_cast<uint64_t>(depth_test);
}

void OpenXRFbCompositionLayerDepthTestExtensionWrapper::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_depth_test_ext) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerDepthTestExtensionWrapper::_get_viewport_composition_layer_extension_properties() {
	TypedArray<Dictionary> properties;

	{
		Dictionary depth_enabled;
		depth_enabled["name"] = ENABLE_PROPERTY_NAME;
		depth_enabled["type"] = Variant::BOOL;
		depth_enabled["hint"] = PROPERTY_HINT_NONE;
		properties.push_back(depth_enabled);
	}

	return properties;
}

Dictionary OpenXRFbCompositionLayerDepthTestExtensionWrapper::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[ENABLE_PROPERTY_NAME] = false;
	return defaults;
}
