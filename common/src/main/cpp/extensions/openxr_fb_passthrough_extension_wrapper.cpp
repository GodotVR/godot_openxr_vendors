/**************************************************************************/
/*  openxr_fb_passthrough_extension_wrapper.cpp                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "extensions/openxr_fb_passthrough_extension_wrapper.h"

#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbPassthroughExtensionWrapper *OpenXRFbPassthroughExtensionWrapper::singleton = nullptr;

OpenXRFbPassthroughExtensionWrapper *OpenXRFbPassthroughExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbPassthroughExtensionWrapper());
	}
	return singleton;
}

OpenXRFbPassthroughExtensionWrapper::OpenXRFbPassthroughExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbPassthroughExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_PASSTHROUGH_EXTENSION_NAME] = &fb_passthrough_ext;
	request_extensions[XR_FB_TRIANGLE_MESH_EXTENSION_NAME] = &fb_triangle_mesh_ext;

	singleton = this;
}

OpenXRFbPassthroughExtensionWrapper::~OpenXRFbPassthroughExtensionWrapper() {
	cleanup();
}

void OpenXRFbPassthroughExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_passthrough_supported"), &OpenXRFbPassthroughExtensionWrapper::is_passthrough_supported);
	ClassDB::bind_method(D_METHOD("is_passthrough_enabled"), &OpenXRFbPassthroughExtensionWrapper::is_passthrough_enabled);
}

godot::Dictionary OpenXRFbPassthroughExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext: request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbPassthroughExtensionWrapper::cleanup() {
	fb_passthrough_ext = false;
	fb_triangle_mesh_ext = false;
}

void OpenXRFbPassthroughExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	XrInstance instance = (XrInstance)p_instance;
	if (fb_passthrough_ext) {
		bool result = initialize_fb_passthrough_extension(instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_passthrough extension");
			fb_passthrough_ext = false;
		}
	}

	if (fb_triangle_mesh_ext) {
		bool result = initialize_fb_triangle_mesh_extension(instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_triangle_mesh extension");
			fb_triangle_mesh_ext = false;
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_session_created(uint64_t p_session) {
	XrSession session = (XrSession)p_session;
	if (fb_passthrough_ext) {
		// If it's already supported, then we don't want to emulate it.
		if (get_openxr_api()->is_environment_blend_mode_alpha_supported() != OpenXRAPIExtension::OPENXR_ALPHA_BLEND_MODE_SUPPORT_NONE) {
			return;
		}

		// Create the passthrough feature and start it.
		XrPassthroughCreateInfoFB passthrough_create_info = {
			XR_TYPE_PASSTHROUGH_CREATE_INFO_FB,
			nullptr,
			0,
		};

		XrResult result = xrCreatePassthroughFB(session, &passthrough_create_info, &passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create passthrough");
			passthrough_handle = XR_NULL_HANDLE;
			return;
		}

		get_openxr_api()->register_composition_layer_provider(this);
		get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(true);
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_state_ready() {
	XRInterface::EnvironmentBlendMode blend_mode = get_blend_mode();
	if (blend_mode == XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND) {
		start_passthrough();
	}
	previous_blend_mode = blend_mode;
}

void OpenXRFbPassthroughExtensionWrapper::_on_process() {
	XRInterface::EnvironmentBlendMode blend_mode = get_blend_mode();

	if (previous_blend_mode != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && blend_mode == XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND) {
		start_passthrough();
	} else if (previous_blend_mode == XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && blend_mode != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND) {
		stop_passthrough();
	}

	previous_blend_mode = blend_mode;
}

void OpenXRFbPassthroughExtensionWrapper::_on_session_destroyed() {
	if (fb_passthrough_ext) {
		stop_passthrough();

		XrResult result;
		if (passthrough_handle != XR_NULL_HANDLE) {
			result = xrDestroyPassthroughFB(passthrough_handle);
			if (XR_FAILED(result)) {
				UtilityFunctions::print("Unable to destroy passthrough feature");
			}
			passthrough_handle = XR_NULL_HANDLE;

			get_openxr_api()->unregister_composition_layer_provider(this);
			get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(false);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbPassthroughExtensionWrapper::is_passthrough_enabled() {
	return fb_passthrough_ext && passthrough_handle != XR_NULL_HANDLE && passthrough_layer != XR_NULL_HANDLE;
}

bool OpenXRFbPassthroughExtensionWrapper::start_passthrough() {
	if (passthrough_handle == XR_NULL_HANDLE) {
		return false;
	}

	if (is_passthrough_enabled()) {
		return true;
	}

	// Start the passthrough feature
	XrResult result = xrPassthroughStartFB(passthrough_handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to start passthrough");
		stop_passthrough();
		return false;
	}

	// Create the passthrough layer
	XrPassthroughLayerCreateInfoFB passthrough_layer_config = {
		XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB,
		nullptr,
		passthrough_handle,
		XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB,
		XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB,
	};
	result = xrCreatePassthroughLayerFB(SESSION, &passthrough_layer_config, &passthrough_layer);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create the passthrough layer");
		stop_passthrough();
		return false;
	}

	return true;
}

uint64_t OpenXRFbPassthroughExtensionWrapper::_get_composition_layer() {
	if (is_passthrough_enabled()) {
		composition_passthrough_layer.layerHandle = passthrough_layer;
		return reinterpret_cast<uint64_t>(&composition_passthrough_layer);
	} else {
		return 0;
	}
}

void OpenXRFbPassthroughExtensionWrapper::stop_passthrough() {
	if (!fb_passthrough_ext) {
		return;
	}

	XrResult result;
	if (passthrough_layer != XR_NULL_HANDLE) {
		// Destroy the layer
		result = xrDestroyPassthroughLayerFB(passthrough_layer);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Unable to destroy passthrough layer");
		}
		passthrough_layer = XR_NULL_HANDLE;
	}

	if (passthrough_handle != XR_NULL_HANDLE) {
		result = xrPassthroughPauseFB(passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Unable to stop passthrough feature");
		}
	}
}

XRInterface::EnvironmentBlendMode OpenXRFbPassthroughExtensionWrapper::get_blend_mode() {
	Ref<XRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	if (xr_interface.is_valid()) {
		return xr_interface->get_environment_blend_mode();
	}
	return XRInterface::XR_ENV_BLEND_MODE_OPAQUE;
}

bool OpenXRFbPassthroughExtensionWrapper::initialize_fb_passthrough_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePassthroughFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyPassthroughFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughStartFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughPauseFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePassthroughLayerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyPassthroughLayerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughLayerPauseFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughLayerResumeFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughLayerSetStyleFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateGeometryInstanceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyGeometryInstanceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGeometryInstanceSetTransformFB);

	return true;
}

bool OpenXRFbPassthroughExtensionWrapper::initialize_fb_triangle_mesh_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateTriangleMeshFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyTriangleMeshFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshGetVertexBufferFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshGetIndexBufferFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshBeginUpdateFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshEndUpdateFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshBeginVertexBufferUpdateFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshEndVertexBufferUpdateFB);

	return true;
}
