/**************************************************************************/
/*  openxr_android_unbounded_reference_space_extension.cpp                */
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

#include "extensions/openxr_android_unbounded_reference_space_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define XR_ANDROID_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME "XR_ANDROID_unbounded_reference_space"
static constexpr XrReferenceSpaceType XR_REFERENCE_SPACE_TYPE_UNBOUNDED_ANDROID = (XrReferenceSpaceType)1000467000;

using namespace godot;

OpenXRAndroidUnboundedReferenceSpaceExtension *OpenXRAndroidUnboundedReferenceSpaceExtension::singleton = nullptr;

OpenXRAndroidUnboundedReferenceSpaceExtension *OpenXRAndroidUnboundedReferenceSpaceExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidUnboundedReferenceSpaceExtension());
	}
	return singleton;
}

OpenXRAndroidUnboundedReferenceSpaceExtension::OpenXRAndroidUnboundedReferenceSpaceExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidUnboundedReferenceSpaceExtension singleton already exists.");

	request_extensions[XR_ANDROID_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME] = &androidxr_unbounded_reference_space_ext;
	singleton = this;
}

OpenXRAndroidUnboundedReferenceSpaceExtension::~OpenXRAndroidUnboundedReferenceSpaceExtension() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRAndroidUnboundedReferenceSpaceExtension::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRAndroidUnboundedReferenceSpaceExtension::_on_instance_created(uint64_t p_instance) {
	if (androidxr_unbounded_reference_space_ext) {
		bool result = initialize_androidxr_unbounded_reference_space_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize androidxr_unbounded_reference_space extension");
			androidxr_unbounded_reference_space_ext = false;
		}
	}
}

void OpenXRAndroidUnboundedReferenceSpaceExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRAndroidUnboundedReferenceSpaceExtension::_on_session_created(uint64_t p_instance) {
	if (!androidxr_unbounded_reference_space_ext) {
		return;
	}

	XrReferenceSpaceCreateInfo create_info = {
		XR_TYPE_REFERENCE_SPACE_CREATE_INFO, // type
		nullptr, // next
		XR_REFERENCE_SPACE_TYPE_UNBOUNDED_ANDROID, // referenceSpaceType
		{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } } // poseInReferenceSpace
	};

	XrResult result = xrCreateReferenceSpace(SESSION, &create_info, &unbounded_space);
	if (XR_FAILED(result)) {
		ERR_PRINT(vformat("OpenXR: Failed to create unbounded space [%s]", get_openxr_api()->get_error_string(result)));
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	bool enable_on_startup = (bool)project_settings->get_setting_with_override("xr/openxr/extensions/androidxr/unbounded_reference_space/enable_on_startup");
	if (enable_on_startup) {
		set_unbounded_reference_space_enabled(true);
	}
}

void OpenXRAndroidUnboundedReferenceSpaceExtension::set_unbounded_reference_space_enabled(bool p_enable) {
	if (!androidxr_unbounded_reference_space_ext) {
		return;
	}

	if (p_enable) {
		get_openxr_api()->set_custom_play_space(unbounded_space);
	} else {
		get_openxr_api()->set_custom_play_space(XR_NULL_HANDLE);
	}
}

bool OpenXRAndroidUnboundedReferenceSpaceExtension::is_unbounded_reference_space_available() {
	return androidxr_unbounded_reference_space_ext && (unbounded_space != XR_NULL_HANDLE);
}

bool OpenXRAndroidUnboundedReferenceSpaceExtension::is_play_space_unbounded() {
	return get_openxr_api()->get_play_space() == reinterpret_cast<uint64_t>(unbounded_space);
}

void OpenXRAndroidUnboundedReferenceSpaceExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_unbounded_reference_space_enabled", "enable"), &OpenXRAndroidUnboundedReferenceSpaceExtension::set_unbounded_reference_space_enabled);

	ClassDB::bind_method(D_METHOD("is_unbounded_reference_space_available"), &OpenXRAndroidUnboundedReferenceSpaceExtension::is_unbounded_reference_space_available);

	ClassDB::bind_method(D_METHOD("is_play_space_unbounded"), &OpenXRAndroidUnboundedReferenceSpaceExtension::is_play_space_unbounded);
}

void OpenXRAndroidUnboundedReferenceSpaceExtension::cleanup() {
	androidxr_unbounded_reference_space_ext = false;

	if (unbounded_space != XR_NULL_HANDLE) {
		xrDestroySpace(unbounded_space);
		unbounded_space = XR_NULL_HANDLE;
	}
}

bool OpenXRAndroidUnboundedReferenceSpaceExtension::initialize_androidxr_unbounded_reference_space_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateReferenceSpace);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySpace);

	return true;
}
