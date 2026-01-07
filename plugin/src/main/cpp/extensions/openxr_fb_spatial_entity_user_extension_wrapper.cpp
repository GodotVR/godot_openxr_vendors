/**************************************************************************/
/*  openxr_fb_spatial_entity_user_extension_wrapper.cpp                   */
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

#include "extensions/openxr_fb_spatial_entity_user_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbSpatialEntityUserExtensionWrapper *OpenXRFbSpatialEntityUserExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityUserExtensionWrapper *OpenXRFbSpatialEntityUserExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityUserExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityUserExtensionWrapper::OpenXRFbSpatialEntityUserExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntityUserExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_USER_EXTENSION_NAME] = &fb_spatial_entity_user_ext;
	singleton = this;
}

OpenXRFbSpatialEntityUserExtensionWrapper::~OpenXRFbSpatialEntityUserExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityUserExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_user_supported"), &OpenXRFbSpatialEntityUserExtensionWrapper::is_spatial_entity_user_supported);
}

void OpenXRFbSpatialEntityUserExtensionWrapper::cleanup() {
	fb_spatial_entity_user_ext = false;
}

Dictionary OpenXRFbSpatialEntityUserExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityUserExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_user_ext) {
		bool result = initialize_fb_spatial_entity_user_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize fb_spatial_entity_user extension");
			fb_spatial_entity_user_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityUserExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityUserExtensionWrapper::initialize_fb_spatial_entity_user_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSpaceUserFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceUserIdFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySpaceUserFB);

	return true;
}

XrSpaceUserFB OpenXRFbSpatialEntityUserExtensionWrapper::create_user(uint64_t p_user_id) {
	XrSpaceUserCreateInfoFB info = {
		XR_TYPE_SPACE_USER_CREATE_INFO_FB, // type
		nullptr, // next
		p_user_id, // userId
	};

	XrSpaceUserFB user = XR_NULL_HANDLE;

	XrResult result = xrCreateSpaceUserFB(SESSION, &info, &user);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrCreateSpaceUserFB failed: %s", get_openxr_api()->get_error_string(result)));
		return XR_NULL_HANDLE;
	}

	return user;
}

uint64_t OpenXRFbSpatialEntityUserExtensionWrapper::get_user_id(XrSpaceUserFB p_user) {
	XrSpaceUserIdFB user_id = 0;

	XrResult result = xrGetSpaceUserIdFB(p_user, &user_id);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetSpaceUserIdFB failed: %s", get_openxr_api()->get_error_string(result)));
		return 0;
	}

	return user_id;
}

void OpenXRFbSpatialEntityUserExtensionWrapper::destroy_user(XrSpaceUserFB p_user) {
	XrResult result = xrDestroySpaceUserFB(p_user);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrDestroySpaceUserFB failed: %s", get_openxr_api()->get_error_string(result)));
	}
}
