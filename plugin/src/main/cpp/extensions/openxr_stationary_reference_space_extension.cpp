/**************************************************************************/
/*  openxr_stationary_reference_space_extension.cpp                       */
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

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
#include "extensions/openxr_stationary_reference_space_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRStationaryReferenceSpaceExtension *OpenXRStationaryReferenceSpaceExtension::singleton = nullptr;

OpenXRStationaryReferenceSpaceExtension *OpenXRStationaryReferenceSpaceExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRStationaryReferenceSpaceExtension());
	}
	return singleton;
}

OpenXRStationaryReferenceSpaceExtension::OpenXRStationaryReferenceSpaceExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRStationaryReferenceSpaceExtension singleton already exists.");

	request_extensions[XR_EXTX2_STATIONARY_REFERENCE_SPACE_EXTENSION_NAME] = &stationary_reference_space_ext;
	singleton = this;
}

OpenXRStationaryReferenceSpaceExtension::~OpenXRStationaryReferenceSpaceExtension() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRStationaryReferenceSpaceExtension::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRStationaryReferenceSpaceExtension::_on_instance_created(uint64_t p_instance) {
	if (stationary_reference_space_ext) {
		bool result = initialize_stationary_reference_space_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize stationary_reference_space extension");
			stationary_reference_space_ext = false;
		}
	}
}

void OpenXRStationaryReferenceSpaceExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRStationaryReferenceSpaceExtension::_on_session_created(uint64_t p_instance) {
	if (!stationary_reference_space_ext) {
		return;
	}

	XrReferenceSpaceCreateInfo create_info = {
		XR_TYPE_REFERENCE_SPACE_CREATE_INFO, // type
		nullptr, // next
		XR_REFERENCE_SPACE_TYPE_STATIONARY_EXTX2, // referenceSpaceType
		{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } } // poseInReferenceSpace
	};

	XrResult result = xrCreateReferenceSpace(SESSION, &create_info, &stationary_space);
	if (XR_FAILED(result)) {
		ERR_PRINT(vformat("OpenXR: Failed to create stationary space [%s]", get_openxr_api()->get_error_string(result)));
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	bool enable_on_startup = (bool)project_settings->get_setting_with_override("xr/openxr/extensions/stationary_reference_space/enable_on_startup");
	if (enable_on_startup) {
		set_stationary_reference_space_enabled(true);
	}
}

void OpenXRStationaryReferenceSpaceExtension::set_stationary_reference_space_enabled(bool p_enable) {
	if (!stationary_reference_space_ext) {
		return;
	}

	if (p_enable) {
		get_openxr_api()->set_custom_play_space(stationary_space);
	} else {
		get_openxr_api()->set_custom_play_space(XR_NULL_HANDLE);
	}
}

StringName OpenXRStationaryReferenceSpaceExtension::get_stationary_reference_space_uuid() {
	if (!stationary_reference_space_ext) {
		return "";
	}

	XrStationaryReferenceSpaceIdResultEXTX2 stationary_reference_space_id_result = {
		XR_TYPE_STATIONARY_REFERENCE_SPACE_ID_RESULT_EXTX2, // type
		nullptr, // next
		0 // generationId
	};

	XrResult result = xrGetStationaryReferenceSpaceIdEXTX2(SESSION, nullptr, &stationary_reference_space_id_result);
	if (XR_FAILED(result)) {
		ERR_PRINT(vformat("OpenXR: Failed to get stationary reference space ID [%s]", get_openxr_api()->get_error_string(result)));
		return "";
	}

	StringName uuid_string_name = OpenXRUtilities::uuid_to_string_name(stationary_reference_space_id_result.generationId);
	return uuid_string_name;
}

bool OpenXRStationaryReferenceSpaceExtension::is_stationary_reference_space_available() {
	return stationary_reference_space_ext && (stationary_space != XR_NULL_HANDLE);
}

void OpenXRStationaryReferenceSpaceExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_stationary_reference_space_enabled", "enable"), &OpenXRStationaryReferenceSpaceExtension::set_stationary_reference_space_enabled);

	ClassDB::bind_method(D_METHOD("get_stationary_reference_space_uuid"), &OpenXRStationaryReferenceSpaceExtension::get_stationary_reference_space_uuid);

	ClassDB::bind_method(D_METHOD("is_stationary_reference_space_available"), &OpenXRStationaryReferenceSpaceExtension::is_stationary_reference_space_available);
}

void OpenXRStationaryReferenceSpaceExtension::cleanup() {
	stationary_reference_space_ext = false;

	if (stationary_space != XR_NULL_HANDLE) {
		xrDestroySpace(stationary_space);
		stationary_space = XR_NULL_HANDLE;
	}
}

bool OpenXRStationaryReferenceSpaceExtension::initialize_stationary_reference_space_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateReferenceSpace);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySpace);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetStationaryReferenceSpaceIdEXTX2);

	return true;
}
#endif // META_HEADERS_ENABLED
