/**************************************************************************/
/*  openxr_htc_passthrough_extension_wrapper.cpp                          */
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

#include "extensions/openxr_htc_passthrough_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

OpenXRHtcPassthroughExtensionWrapper *OpenXRHtcPassthroughExtensionWrapper::singleton = nullptr;

OpenXRHtcPassthroughExtensionWrapper *OpenXRHtcPassthroughExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRHtcPassthroughExtensionWrapper());
	}
	return singleton;
}

OpenXRHtcPassthroughExtensionWrapper::OpenXRHtcPassthroughExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRHtcPassthroughExtensionWrapper singleton already exists.");

	request_extensions[XR_HTC_PASSTHROUGH_EXTENSION_NAME] = &htc_passthrough_ext;

	singleton = this;
}

OpenXRHtcPassthroughExtensionWrapper::~OpenXRHtcPassthroughExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

void OpenXRHtcPassthroughExtensionWrapper::cleanup() {
	htc_passthrough_ext = false;
}

void OpenXRHtcPassthroughExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_passthrough_supported"), &OpenXRHtcPassthroughExtensionWrapper::is_passthrough_supported);
	ClassDB::bind_method(D_METHOD("is_passthrough_started"), &OpenXRHtcPassthroughExtensionWrapper::is_passthrough_started);
}

godot::Dictionary OpenXRHtcPassthroughExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;

	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}

	return result;
}

void OpenXRHtcPassthroughExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	XrInstance instance = (XrInstance)p_instance;
	if (htc_passthrough_ext) {
		bool result = initialize_htc_passthrough_extension(instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize htc_passthrough extension");
			htc_passthrough_ext = false;
		}
	}
}

void OpenXRHtcPassthroughExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRHtcPassthroughExtensionWrapper::_on_session_created(uint64_t p_session) {
	if (htc_passthrough_ext) {
		// If another extension is already emulating alpha blend mode, then we don't attempt to.
		if (get_openxr_api()->is_environment_blend_mode_alpha_supported() == OpenXRAPIExtension::OPENXR_ALPHA_BLEND_MODE_SUPPORT_EMULATING) {
			// Act as if the extension is not enabled.
			htc_passthrough_ext = false;
			return;
		}

		// Note: With HTC Passthrough, we create our handle and just switch passthrough
		// by providing, or not providing, a passthrough composition layer.
		XrSession session = (XrSession)p_session;

		const XrPassthroughCreateInfoHTC create_info = {
			XR_TYPE_PASSTHROUGH_CREATE_INFO_HTC, // XrStructureType
			nullptr, // next
			XR_PASSTHROUGH_FORM_PLANAR_HTC // XrPassthroughFormHTC
		};

		XrResult result = xrCreatePassthroughHTC(session, &create_info, &passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to create passthrough");
			passthrough_handle = XR_NULL_HANDLE;
			return;
		}

		get_openxr_api()->register_composition_layer_provider(this);
		get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(true);
	}
}

void OpenXRHtcPassthroughExtensionWrapper::_on_session_destroyed() {
	if (htc_passthrough_ext && passthrough_handle != XR_NULL_HANDLE) {
		XrResult result = xrDestroyPassthroughHTC(passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Unable to destroy passthrough feature");
		}
		passthrough_handle = XR_NULL_HANDLE;

		get_openxr_api()->unregister_composition_layer_provider(this);
		get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(false);
	}
}

bool OpenXRHtcPassthroughExtensionWrapper::initialize_htc_passthrough_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePassthroughHTC);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyPassthroughHTC);

	return true;
}

int OpenXRHtcPassthroughExtensionWrapper::_get_composition_layer_count() {
	return is_passthrough_started() ? 1 : 0;
}

uint64_t OpenXRHtcPassthroughExtensionWrapper::_get_composition_layer(int p_index) {
	if (p_index == 0) {
		composition_passthrough_layer.passthrough = passthrough_handle;
		return reinterpret_cast<uint64_t>(&composition_passthrough_layer);
	} else {
		return 0;
	}
}

int OpenXRHtcPassthroughExtensionWrapper::_get_composition_layer_order(int p_index) {
	// Ensure the passthrough layer will be behind the projection layer.
	return -100;
}
