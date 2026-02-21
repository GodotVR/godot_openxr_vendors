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

#include "extensions/openxr_fb_composition_layer_depth_test_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *ENABLE_PROPERTY_NAME = "XR_FB_composition_layer_depth_test/enable";

OpenXRFbCompositionLayerDepthTestExtension *OpenXRFbCompositionLayerDepthTestExtension::singleton = nullptr;

OpenXRFbCompositionLayerDepthTestExtension *OpenXRFbCompositionLayerDepthTestExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbCompositionLayerDepthTestExtension());
	}
	return singleton;
}

OpenXRFbCompositionLayerDepthTestExtension::OpenXRFbCompositionLayerDepthTestExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbCompositionLayerDepthTestExtension singleton already exists.");

	request_extensions[XR_FB_COMPOSITION_LAYER_DEPTH_TEST_EXTENSION_NAME] = &fb_composition_layer_depth_test_ext;
	singleton = this;
}

OpenXRFbCompositionLayerDepthTestExtension::~OpenXRFbCompositionLayerDepthTestExtension() {
	cleanup();
}

void OpenXRFbCompositionLayerDepthTestExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbCompositionLayerDepthTestExtension::is_composition_layer_depth_test_supported);
	ClassDB::bind_method(D_METHOD("set_projection_layer_depth_test_enabled", "enabled"), &OpenXRFbCompositionLayerDepthTestExtension::set_projection_layer_depth_test_enabled);
	ClassDB::bind_method(D_METHOD("is_projection_layer_depth_test_enabled"), &OpenXRFbCompositionLayerDepthTestExtension::is_projection_layer_depth_test_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "projection_layer_depth_test_enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_projection_layer_depth_test_enabled", "is_projection_layer_depth_test_enabled");
}

void OpenXRFbCompositionLayerDepthTestExtension::cleanup() {
	fb_composition_layer_depth_test_ext = false;
	projection_layer_depth_test_enabled = false;
}

void OpenXRFbCompositionLayerDepthTestExtension::_on_session_created(uint64_t p_session) {
	if (!fb_composition_layer_depth_test_ext) {
		return;
	}
	get_openxr_api()->register_projection_layer_extension(this);
}

void OpenXRFbCompositionLayerDepthTestExtension::_on_session_destroyed() {
	if (!fb_composition_layer_depth_test_ext) {
		return;
	}
	get_openxr_api()->unregister_projection_layer_extension(this);
}

void OpenXRFbCompositionLayerDepthTestExtension::_on_state_ready() {
	if (!fb_composition_layer_depth_test_ext) {
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	bool depth_test_enabled = project_settings->get_setting_with_override("xr/openxr/extensions/meta/composition_layer_settings/main_projection_layer/enable_depth_test");
	set_projection_layer_depth_test_enabled(depth_test_enabled);

	if (depth_test_enabled) {
		// Check if depth buffer submission is enabled.
		bool depth_buffer_submission_enabled = project_settings->get_setting_with_override("xr/openxr/submit_depth_buffer");
		if (!depth_buffer_submission_enabled) {
			WARN_PRINT("'xr/openxr/submit_depth_buffer' must be enabled for depth testing to take effect.");
		}
	}
}

void OpenXRFbCompositionLayerDepthTestExtension::set_projection_layer_depth_test_enabled(bool p_projection_layer_depth_test_enabled) {
	ERR_FAIL_COND_MSG(!fb_composition_layer_depth_test_ext, "XR_FB_composition_layer_depth_test is not enabled");
	projection_layer_depth_test_enabled = p_projection_layer_depth_test_enabled;
}

bool OpenXRFbCompositionLayerDepthTestExtension::is_projection_layer_depth_test_enabled() const {
	ERR_FAIL_COND_V_MSG(!fb_composition_layer_depth_test_ext, false, "XR_FB_composition_layer_depth_test is not enabled");
	return projection_layer_depth_test_enabled;
}

Dictionary OpenXRFbCompositionLayerDepthTestExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRFbCompositionLayerDepthTestExtension::_set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) {
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

uint64_t OpenXRFbCompositionLayerDepthTestExtension::_set_projection_layer_and_get_next_pointer(void *p_next_pointer) {
	if (!fb_composition_layer_depth_test_ext || !projection_layer_depth_test_enabled) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	projection_layer_depth_test.next = p_next_pointer;
	return reinterpret_cast<uint64_t>(&projection_layer_depth_test);
}

void OpenXRFbCompositionLayerDepthTestExtension::_on_viewport_composition_layer_destroyed(const void *p_layer) {
	if (fb_composition_layer_depth_test_ext) {
		const XrCompositionLayerBaseHeader *layer = reinterpret_cast<const XrCompositionLayerBaseHeader *>(p_layer);
		layer_structs.erase(layer);
	}
}

TypedArray<Dictionary> OpenXRFbCompositionLayerDepthTestExtension::_get_viewport_composition_layer_extension_properties() {
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

Dictionary OpenXRFbCompositionLayerDepthTestExtension::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[ENABLE_PROPERTY_NAME] = false;
	return defaults;
}
