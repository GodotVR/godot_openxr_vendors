/**************************************************************************/
/*  openxr_fb_scene_capture_extension_wrapper.cpp                         */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Original contributed implementation:                                   */
/*   Copyright (c) 2022-2023 MattaKis Consulting Kft. (Migeran)           */
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

#include "extensions/openxr_fb_scene_capture_extension_wrapper.h"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbSceneCaptureExtensionWrapper *OpenXRFbSceneCaptureExtensionWrapper::singleton = nullptr;

OpenXRFbSceneCaptureExtensionWrapper *OpenXRFbSceneCaptureExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSceneCaptureExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSceneCaptureExtensionWrapper::OpenXRFbSceneCaptureExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSceneCaptureExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SCENE_CAPTURE_EXTENSION_NAME] = &fb_scene_capture_ext;
	singleton = this;
}

OpenXRFbSceneCaptureExtensionWrapper::~OpenXRFbSceneCaptureExtensionWrapper() {
	cleanup();
}

void OpenXRFbSceneCaptureExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_scene_capture_supported"), &OpenXRFbSceneCaptureExtensionWrapper::is_scene_capture_supported);
	ClassDB::bind_method(D_METHOD("is_scene_capture_enabled"), &OpenXRFbSceneCaptureExtensionWrapper::is_scene_capture_enabled);
	ClassDB::bind_method(D_METHOD("request_scene_capture"), &OpenXRFbSceneCaptureExtensionWrapper::_request_scene_capture_bind);

	ADD_SIGNAL(MethodInfo("scene_capture_completed"));
}

void OpenXRFbSceneCaptureExtensionWrapper::cleanup() {
	fb_scene_capture_ext = false;
}

godot::Dictionary OpenXRFbSceneCaptureExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbSceneCaptureExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_scene_capture_ext) {
		bool result = initialize_fb_scene_capture_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_scene_capture extension");
			fb_scene_capture_ext = false;
		}
	}
}

void OpenXRFbSceneCaptureExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSceneCaptureExtensionWrapper::request_scene_capture(String p_request, SceneCaptureCompleteCallback p_callback, void *p_userdata) {
	if (scene_capture_enabled) {
		ERR_PRINT("Already running scene capture");
		p_callback(XR_ERROR_VALIDATION_FAILURE, p_userdata);
		return false;
	}

	XrAsyncRequestIdFB request_id;

	CharString cstr = p_request.ascii();

	XrSceneCaptureRequestInfoFB request = {
		XR_TYPE_SCENE_CAPTURE_REQUEST_INFO_FB, // type
		nullptr, // next
		(uint32_t)cstr.size(), // requestByteCount
		cstr.ptr(), // request
	};

	XrResult result = xrRequestSceneCaptureFB((XrSession)get_openxr_api()->get_session(), &request, &request_id);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrRequestSceneCaptureFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		p_callback(result, p_userdata);
		return false;
	}

	scene_capture_enabled = true;
	requests[request_id] = RequestInfo(p_callback, p_userdata);
	return true;
}

bool OpenXRFbSceneCaptureExtensionWrapper::_request_scene_capture_bind() {
	return request_scene_capture("", nullptr, nullptr);
}

bool OpenXRFbSceneCaptureExtensionWrapper::is_scene_capture_enabled() {
	return scene_capture_enabled;
}

bool OpenXRFbSceneCaptureExtensionWrapper::initialize_fb_scene_capture_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrRequestSceneCaptureFB);

	return true;
}

bool OpenXRFbSceneCaptureExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB) {
		on_scene_capture_complete((const XrEventDataSceneCaptureCompleteFB *)event);
		return true;
	}
	return false;
}

void OpenXRFbSceneCaptureExtensionWrapper::on_scene_capture_complete(const XrEventDataSceneCaptureCompleteFB *p_event) {
	if (!requests.has(p_event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB");
		return;
	}

	scene_capture_enabled = false;
	emit_signal("scene_capture_completed");

	RequestInfo *request = requests.getptr(p_event->requestId);
	if (request->callback) {
		request->callback(p_event->result, request->userdata);
	}
	requests.erase(p_event->requestId);
}
