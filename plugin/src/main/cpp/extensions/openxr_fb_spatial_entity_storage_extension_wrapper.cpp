/**************************************************************************/
/*  openxr_fb_spatial_entity_storage_extension_wrapper.cpp                  */
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

#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbSpatialEntityStorageExtensionWrapper *OpenXRFbSpatialEntityStorageExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityStorageExtensionWrapper *OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityStorageExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityStorageExtensionWrapper::OpenXRFbSpatialEntityStorageExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRSpatialEntityStorageExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME] = &fb_spatial_entity_storage_ext;
	singleton = this;
}

OpenXRFbSpatialEntityStorageExtensionWrapper::~OpenXRFbSpatialEntityStorageExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_storage_supported"), &OpenXRFbSpatialEntityStorageExtensionWrapper::is_spatial_entity_storage_supported);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::cleanup() {
	fb_spatial_entity_storage_ext = false;
}

Dictionary OpenXRFbSpatialEntityStorageExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_storage_ext) {
		bool result = initialize_fb_spatial_entity_storage_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity_storage extension");
			fb_spatial_entity_storage_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::initialize_fb_spatial_entity_storage_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrSaveSpaceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrEraseSpaceFB);

	return true;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB) {
		on_space_save_complete((const XrEventDataSpaceSaveCompleteFB *)event);
		return true;
	}

	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB) {
		on_space_erase_complete((const XrEventDataSpaceEraseCompleteFB *)event);
		return true;
	}

	return false;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::save_space(const XrSpaceSaveInfoFB *p_info, StorageRequestCompleteCallback p_callback, void *p_userdata) {
	XrAsyncRequestIdFB request_id;

	const XrResult result = xrSaveSpaceFB(SESSION, p_info, &request_id);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrSaveSpaceFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		p_callback(result, p_info->location, p_userdata);
		return false;
	}

	requests[request_id] = RequestInfo(p_callback, p_userdata);
	return true;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::erase_space(const XrSpaceEraseInfoFB *p_info, StorageRequestCompleteCallback p_callback, void *p_userdata) {
	XrAsyncRequestIdFB request_id;

	const XrResult result = xrEraseSpaceFB(SESSION, p_info, &request_id);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrEraseSpaceFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		p_callback(result, p_info->location, p_userdata);
		return false;
	}

	requests[request_id] = RequestInfo(p_callback, p_userdata);
	return true;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::on_space_save_complete(const XrEventDataSpaceSaveCompleteFB *event) {
	if (!requests.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB");
		return;
	}

	RequestInfo *request = requests.getptr(event->requestId);
	request->callback(event->result, event->location, request->userdata);
	requests.erase(event->requestId);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::on_space_erase_complete(const XrEventDataSpaceEraseCompleteFB *event) {
	if (!requests.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB");
		return;
	}

	RequestInfo *request = requests.getptr(event->requestId);
	request->callback(event->result, event->location, request->userdata);
	requests.erase(event->requestId);
}
