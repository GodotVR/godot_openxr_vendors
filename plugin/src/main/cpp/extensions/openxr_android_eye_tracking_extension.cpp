/**************************************************************************/
/*  openxr_android_eye_tracking_extension_wrapper.cpp                     */
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

#include "extensions/openxr_android_eye_tracking_extension.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include "util.h"

using namespace godot;

OpenXRAndroidEyeTrackingExtension *OpenXRAndroidEyeTrackingExtension::singleton = nullptr;

OpenXRAndroidEyeTrackingExtension *OpenXRAndroidEyeTrackingExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidEyeTrackingExtension());
	}
	return singleton;
}

OpenXRAndroidEyeTrackingExtension::OpenXRAndroidEyeTrackingExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidEyeTrackingExtension singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_EYE_TRACKING_EXTENSION_NAME] = &available;

	on_request_permissions_result_callable = callable_mp(this, &OpenXRAndroidEyeTrackingExtension::_on_request_permissions_result);
}

OpenXRAndroidEyeTrackingExtension::~OpenXRAndroidEyeTrackingExtension() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidEyeTrackingExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRAndroidEyeTrackingExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (available) {
		eye_tracking_properties.type = XR_TYPE_SYSTEM_EYE_TRACKING_PROPERTIES_ANDROID;
		eye_tracking_properties.next = p_next_pointer;
		eye_tracking_properties.supportsEyeTracking = XR_FALSE;
		return reinterpret_cast<uint64_t>(&eye_tracking_properties);
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRAndroidEyeTrackingExtension::_on_instance_created(uint64_t p_instance) {
	if (available) {
		if (!_initialize_openxr_android_eye_tracking_extension()) {
			UtilityFunctions::print("Failed to initialize eye tracking extension");
			available = false;
		}
	}
}

void OpenXRAndroidEyeTrackingExtension::_on_session_created(uint64_t instance) {
	if (!available || !eye_tracking_properties.supportsEyeTracking) {
		return;
	}

	_try_create_eye_tracker();
}

void OpenXRAndroidEyeTrackingExtension::_on_request_permissions_result(const String &p_permission, bool p_granted) {
	if (!available || p_permission != "android.permission.EYE_TRACKING_COARSE" || !p_granted) {
		return;
	}

	_try_create_eye_tracker();
}

void OpenXRAndroidEyeTrackingExtension::_try_create_eye_tracker() {
	if (eye_tracker != XR_NULL_HANDLE) {
		return;
	}

	XrEyeTrackerCreateInfoANDROID create_info{
		XR_TYPE_EYE_TRACKER_CREATE_INFO_ANDROID, // type
		nullptr, // next
	};

	XrResult result = xrCreateEyeTrackerANDROID(SESSION, &create_info, &eye_tracker);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to create eye tracker; ", get_openxr_api()->get_error_string(result));
		eye_tracker = XR_NULL_HANDLE;
	}
}

void OpenXRAndroidEyeTrackingExtension::_on_session_destroyed() {
	if (eye_tracker != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEyeTrackerANDROID(eye_tracker);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to delete eye tracker: ", get_openxr_api()->get_error_string(result));
		}
		eye_tracker = XR_NULL_HANDLE;
	}
}

bool OpenXRAndroidEyeTrackingExtension::_initialize_openxr_android_eye_tracking_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEyeTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEyeTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetCoarseTrackingEyesInfoANDROID);

	return true;
}

void OpenXRAndroidEyeTrackingExtension::_bind_methods() {
}

void OpenXRAndroidEyeTrackingExtension::_on_state_ready() {
	if (available && eye_tracking_properties.supportsEyeTracking) {
		MainLoop *main_loop = Engine::get_singleton()->get_main_loop();
		if (main_loop && !main_loop->is_connected("on_request_permissions_result", on_request_permissions_result_callable)) {
			main_loop->connect("on_request_permissions_result", on_request_permissions_result_callable);
		}
	}
}

void OpenXRAndroidEyeTrackingExtension::_on_process() {
	if (eye_tracker == XR_NULL_HANDLE) {
		return;
	}

	// Construct the eye trackers if necessary
	for (int i = 0; i < 2; i++) {
		if (xr_eye_tracker[i].is_null()) {
			XRServer *xr_server = XRServer::get_singleton();
			if (!xr_server) {
				// xr server invalid, no point in trying anything further, bail
				return;
			}
			xr_eye_tracker[i].instantiate();
			xr_eye_tracker[i]->set_tracker_name(i == 0 ? "/user/eye_tracker_android/left" : "/user/eye_tracker_android/right");
			xr_eye_tracker[i]->set_tracker_hand(i == 0 ? XRPositionalTracker::TrackerHand::TRACKER_HAND_LEFT : XRPositionalTracker::TrackerHand::TRACKER_HAND_RIGHT);
			xr_server->add_tracker(xr_eye_tracker[i]);
		}
	}

	XrEyesANDROID eyes = {
		XR_TYPE_EYES_ANDROID, // type
		nullptr, // next
		{}, // XrEyeANDROID[]
		{}, // XrEyeTrackingModeANDROID
	};

	XrEyesGetInfoANDROID get_info = {
		XR_TYPE_EYES_GET_INFO_ANDROID, // type
		nullptr, // next
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
	};

	XrResult result = xrGetCoarseTrackingEyesInfoANDROID(eye_tracker, &get_info, &eyes);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: querying eye tracking info failed: ", get_openxr_api()->get_error_string(result));
	}

	bool tracking_left_eye = (result == XR_SUCCESS) && (eyes.mode == XR_EYE_TRACKING_MODE_BOTH_ANDROID || eyes.mode == XR_EYE_TRACKING_MODE_LEFT_ANDROID);
	bool tracking_right_eye = (result == XR_SUCCESS) && (eyes.mode == XR_EYE_TRACKING_MODE_BOTH_ANDROID || eyes.mode == XR_EYE_TRACKING_MODE_RIGHT_ANDROID);

	_populate_eye_tracker(xr_eye_tracker[0].ptr(), tracking_left_eye, eyes.eyes[0]);
	_populate_eye_tracker(xr_eye_tracker[1].ptr(), tracking_right_eye, eyes.eyes[1]);
}

void OpenXRAndroidEyeTrackingExtension::_populate_eye_tracker(XRControllerTracker *tracker, bool tracking_status, const XrEyeANDROID &xr_eye) {
	// handle lost scenario where eye was not tracked
	if (!tracking_status) {
		tracker->invalidate_pose("default");
		return;
	}

	// set state
	switch (xr_eye.eyeState) {
		case XR_EYE_STATE_SHUT_ANDROID: {
			tracker->set_pose("default", OpenXRUtilities::xrPosef_to_godot_transform3d(xr_eye.eyePose), Vector3(), Vector3(), godot::XRPose::XR_TRACKING_CONFIDENCE_HIGH);
			tracker->set_input("blink", true);
			break;
		}
		case XR_EYE_STATE_GAZING_ANDROID: {
			tracker->set_pose("default", OpenXRUtilities::xrPosef_to_godot_transform3d(xr_eye.eyePose), Vector3(), Vector3(), godot::XRPose::XR_TRACKING_CONFIDENCE_HIGH);
			tracker->set_input("blink", false);
			break;
		}
		case XR_EYE_STATE_INVALID_ANDROID:
		default: {
			tracker->invalidate_pose("default");
			break;
		}
	}
}
