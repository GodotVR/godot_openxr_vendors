/**************************************************************************/
/*  openxr_fb_spatial_entity_sharing_extension_wrapper.cpp                */
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

#include "extensions/openxr_fb_spatial_entity_sharing_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbSpatialEntitySharingExtensionWrapper *OpenXRFbSpatialEntitySharingExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntitySharingExtensionWrapper *OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntitySharingExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntitySharingExtensionWrapper::OpenXRFbSpatialEntitySharingExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntitySharingExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_SHARING_EXTENSION_NAME] = &fb_spatial_entity_sharing_ext;
	singleton = this;
}

OpenXRFbSpatialEntitySharingExtensionWrapper::~OpenXRFbSpatialEntitySharingExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntitySharingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_sharing_supported"), &OpenXRFbSpatialEntitySharingExtensionWrapper::is_spatial_entity_sharing_supported);
}

void OpenXRFbSpatialEntitySharingExtensionWrapper::cleanup() {
	fb_spatial_entity_sharing_ext = false;
}

Dictionary OpenXRFbSpatialEntitySharingExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntitySharingExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_sharing_ext) {
		bool result = initialize_fb_spatial_entity_sharing_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize fb_spatial_entity_sharing extension");
			fb_spatial_entity_sharing_ext = false;
		}
	}
}

void OpenXRFbSpatialEntitySharingExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntitySharingExtensionWrapper::initialize_fb_spatial_entity_sharing_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrShareSpacesFB);

	return true;
}

bool OpenXRFbSpatialEntitySharingExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SHARE_COMPLETE_FB) {
		on_space_share_complete((const XrEventDataSpaceShareCompleteFB *)event);
		return true;
	}

	return false;
}

bool OpenXRFbSpatialEntitySharingExtensionWrapper::share_spaces(const XrSpaceShareInfoFB *p_info, ShareSpacesCompleteCallback p_callback, void *p_userdata) {
	XrAsyncRequestIdFB request_id;

	const XrResult result = xrShareSpacesFB(SESSION, p_info, &request_id);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrShareSpacesFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		p_callback(result, p_userdata);
		return false;
	}

	requests[request_id] = RequestInfo(p_callback, p_userdata);
	return true;
}

void OpenXRFbSpatialEntitySharingExtensionWrapper::on_space_share_complete(const XrEventDataSpaceShareCompleteFB *event) {
	if (!requests.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_SHARE_COMPLETE_FB");
		return;
	}

	RequestInfo *request = requests.getptr(event->requestId);
	request->callback(event->result, request->userdata);
	requests.erase(event->requestId);
}
