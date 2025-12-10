/**************************************************************************/
/*  openxr_android_passthrough_camera_state_extension_wrapper.cpp         */
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

#include "extensions/openxr_android_passthrough_camera_state_extension_wrapper.h"
#include <androidxr/androidxr.h>

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidPassthroughCameraStateExtensionWrapper *OpenXRAndroidPassthroughCameraStateExtensionWrapper::singleton = nullptr;

OpenXRAndroidPassthroughCameraStateExtensionWrapper *OpenXRAndroidPassthroughCameraStateExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidPassthroughCameraStateExtensionWrapper());
	}
	return singleton;
}

OpenXRAndroidPassthroughCameraStateExtensionWrapper::OpenXRAndroidPassthroughCameraStateExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidPassthroughCameraStateExtensionWrapper singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_PASSTHROUGH_CAMERA_STATE_EXTENSION_NAME] = &available;
}

OpenXRAndroidPassthroughCameraStateExtensionWrapper::~OpenXRAndroidPassthroughCameraStateExtensionWrapper() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidPassthroughCameraStateExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRAndroidPassthroughCameraStateExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (!available) {
		return;
	}

	if (!_initialize_androidxr_passthrough_camera_state_extension()) {
		UtilityFunctions::print("Failed to initialize passthrough camera state extension");
		available = false;
	}
}

void OpenXRAndroidPassthroughCameraStateExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_passthrough_camera_state"), &OpenXRAndroidPassthroughCameraStateExtensionWrapper::get_passthrough_camera_state);
	BIND_ENUM_CONSTANT(PASSTHROUGH_CAMERA_STATE_DISABLED);
	BIND_ENUM_CONSTANT(PASSTHROUGH_CAMERA_STATE_INITIALIZING);
	BIND_ENUM_CONSTANT(PASSTHROUGH_CAMERA_STATE_READY);
	BIND_ENUM_CONSTANT(PASSTHROUGH_CAMERA_STATE_ERROR);
}

OpenXRAndroidPassthroughCameraStateExtensionWrapper::PassthroughCameraState OpenXRAndroidPassthroughCameraStateExtensionWrapper::get_passthrough_camera_state() {
	if (!available) {
		return PASSTHROUGH_CAMERA_STATE_ERROR;
	}

	XrPassthroughCameraStateGetInfoANDROID getInfo = {
		XR_TYPE_PASSTHROUGH_CAMERA_STATE_GET_INFO_ANDROID, // type
		nullptr, // next
	};
	XrPassthroughCameraStateANDROID output{};
	XrResult result = xrGetPassthroughCameraStateANDROID(SESSION, &getInfo, &output);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get passthrough camera state; ", get_openxr_api()->get_error_string(result));
		return PASSTHROUGH_CAMERA_STATE_ERROR;
	}

	switch (output) {
		case XR_PASSTHROUGH_CAMERA_STATE_DISABLED_ANDROID:
			return PASSTHROUGH_CAMERA_STATE_DISABLED;
		case XR_PASSTHROUGH_CAMERA_STATE_INITIALIZING_ANDROID:
			return PASSTHROUGH_CAMERA_STATE_INITIALIZING;
		case XR_PASSTHROUGH_CAMERA_STATE_READY_ANDROID:
			return PASSTHROUGH_CAMERA_STATE_READY;
		case XR_PASSTHROUGH_CAMERA_STATE_ERROR_ANDROID:
		case XR_PASSTHROUGH_CAMERA_STATE_MAX_ENUM_ANDROID:
		default:
			return PASSTHROUGH_CAMERA_STATE_ERROR;
	}
}

bool OpenXRAndroidPassthroughCameraStateExtensionWrapper::_initialize_androidxr_passthrough_camera_state_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetPassthroughCameraStateANDROID);
	return true;
}
