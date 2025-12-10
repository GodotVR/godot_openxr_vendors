/**************************************************************************/
/*  openxr_android_face_tracking_extension_wrapper.cpp                    */
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

#include "extensions/openxr_android_face_tracking_extension_wrapper.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

// Calculate the average of two floating point values
static inline constexpr float average(float a, float b) {
	return (a + b) * 0.5f;
}

OpenXRAndroidFaceTrackingExtensionWrapper *OpenXRAndroidFaceTrackingExtensionWrapper::singleton = nullptr;

OpenXRAndroidFaceTrackingExtensionWrapper *OpenXRAndroidFaceTrackingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidFaceTrackingExtensionWrapper());
	}
	return singleton;
}

OpenXRAndroidFaceTrackingExtensionWrapper::OpenXRAndroidFaceTrackingExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidFaceTrackingExtensionWrapper singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_FACE_TRACKING_EXTENSION_NAME] = &available;

	on_request_permissions_result_callable = callable_mp(this, &OpenXRAndroidFaceTrackingExtensionWrapper::_on_request_permissions_result);
}

OpenXRAndroidFaceTrackingExtensionWrapper::~OpenXRAndroidFaceTrackingExtensionWrapper() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidFaceTrackingExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRAndroidFaceTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	face_tracking_properties.type = XR_TYPE_SYSTEM_FACE_TRACKING_PROPERTIES_ANDROID;
	face_tracking_properties.next = p_next_pointer;
	face_tracking_properties.supportsFaceTracking = XR_FALSE;
	return reinterpret_cast<uint64_t>(&face_tracking_properties);
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (!_initialize_openxr_android_face_tracking_extension()) {
		UtilityFunctions::print("Failed to initialize face tracking extension");
		available = false;
	}
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_on_session_created(uint64_t instance) {
	if (!available || !face_tracking_properties.supportsFaceTracking) {
		return;
	}

	_try_create_face_tracker();
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_on_request_permissions_result(const String &p_permission, bool p_granted) {
	// On Android XR, if permission was granted during execution, we might want to re-attempt tracker creation.
	if (p_permission != "android.permission.FACE_TRACKING" || !p_granted) {
		return;
	}

	_try_create_face_tracker();
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_try_create_face_tracker() {
	XrFaceTrackerCreateInfoANDROID create_info{
		XR_TYPE_FACE_TRACKER_CREATE_INFO_ANDROID, // type
		nullptr, // next
	};

	XrResult result = xrCreateFaceTrackerANDROID(SESSION, &create_info, &face_tracker);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to create face tracker; ", get_openxr_api()->get_error_string(result));
		face_tracker = XR_NULL_HANDLE;
	}
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_on_state_ready() {
	if (available && face_tracking_properties.supportsFaceTracking && face_tracker == XR_NULL_HANDLE) {
		MainLoop *main_loop = Engine::get_singleton()->get_main_loop();
		if (main_loop && !main_loop->is_connected("on_request_permissions_result", on_request_permissions_result_callable)) {
			main_loop->connect("on_request_permissions_result", on_request_permissions_result_callable);
		}
	}
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_on_session_destroyed() {
	if (face_tracker != XR_NULL_HANDLE) {
		XrResult result = xrDestroyFaceTrackerANDROID(face_tracker);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to delete face tracker: ", get_openxr_api()->get_error_string(result));
		}
		face_tracker = XR_NULL_HANDLE;
	}

	// Unregister the face tracker.
	if (!xr_face_tracker.is_null()) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server) {
			xr_server->remove_tracker(xr_face_tracker);
		}
	}
}

bool OpenXRAndroidFaceTrackingExtensionWrapper::_initialize_openxr_android_face_tracking_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateFaceTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyFaceTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetFaceStateANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetFaceCalibrationStateANDROID);

	return true;
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_face_calibration_state"), &OpenXRAndroidFaceTrackingExtensionWrapper::get_face_calibration_state);

	BIND_ENUM_CONSTANT(CALIBRATION_STATE_UNAVAILABLE);
	BIND_ENUM_CONSTANT(CALIBRATION_STATE_UNCALIBRATED);
	BIND_ENUM_CONSTANT(CALIBRATION_STATE_CALIBRATED);
}

OpenXRAndroidFaceTrackingExtensionWrapper::CalibrationState OpenXRAndroidFaceTrackingExtensionWrapper::get_face_calibration_state() const {
	return calibration_state;
}

void OpenXRAndroidFaceTrackingExtensionWrapper::_on_process() {
	if (face_tracker == XR_NULL_HANDLE) {
		calibration_state = CALIBRATION_STATE_UNAVAILABLE;
		return;
	}

	// Construct the XRFaceTracker if necessary
	if (xr_face_tracker.is_null()) {
		XRServer *xr_server = XRServer::get_singleton();
		if (!xr_server) {
			// xr server invalid, no point in trying anything further, bail
			return;
		}
		xr_face_tracker.instantiate();
		xr_face_tracker->set_tracker_name("/user/face_tracker");
		xr_server->add_tracker(xr_face_tracker);
	}

	XrBool32 ret = false;
	XrResult result = xrGetFaceCalibrationStateANDROID(face_tracker, &ret);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: unable to get face calibration state; ", get_openxr_api()->get_error_string(result));
		calibration_state = CALIBRATION_STATE_UNAVAILABLE;
		return;
	}

	calibration_state = ret ? CALIBRATION_STATE_CALIBRATED : CALIBRATION_STATE_UNCALIBRATED;

	if (calibration_state != CALIBRATION_STATE_CALIBRATED) {
		return;
	}

	XrFaceStateGetInfoANDROID get_info{
		XR_TYPE_FACE_STATE_GET_INFO_ANDROID, // type
		nullptr, // next
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
	};

	// The API Defines two constants XR_FACE_PARAMETER_COUNT_ANDROID and XR_FACE_REGION_CONFIDENCE_COUNT_ANDROID
	// that specifies the count of enums at the current version of the API.
	// But future implementations of the extension could add more parameters or regions. The mechanism to query output size is provided for this reason.
	// As the current implementation uses only known constants into XRFaceTracker, we ignore the query mechanism and ignore count output values.

	float parameters[XR_FACE_PARAMETER_COUNT_ANDROID] = {};
	float region_confidences[XR_FACE_REGION_CONFIDENCE_COUNT_ANDROID] = {};

	XrFaceStateANDROID face_state;
	face_state.type = XR_TYPE_FACE_STATE_ANDROID;

	face_state.parametersCapacityInput = XR_FACE_PARAMETER_COUNT_ANDROID;
	face_state.parameters = parameters;

	face_state.regionConfidencesCapacityInput = XR_FACE_REGION_CONFIDENCE_COUNT_ANDROID;
	face_state.regionConfidences = region_confidences;

	XrResult result_face_state = xrGetFaceStateANDROID(face_tracker, &get_info, &face_state);
	if (result_face_state != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: unable to get face state; ", get_openxr_api()->get_error_string(result_face_state));
		return;
	}

	if (!face_state.isValid || face_state.faceTrackingState == XR_FACE_TRACKING_STATE_STOPPED_ANDROID) {
		// If isValid == false or faceTrackingState is stopped, the data within structure is invalid.
		return;
	}

	// We currently ignore the following members of face_state.
	// faceTrackingState
	// sampleTime
	// regionConfidences

	// Rest of this function deals with unpacking `parameters` into Godot XRFaceTracker structure

	// Map Android XR weights to Godot weights.
	float xr_weights[XRFaceTracker::FT_MAX] = {};

	// Base Shapes
	xr_weights[XRFaceTracker::FT_EYE_LOOK_OUT_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_RIGHT_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_IN_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_LEFT_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_UP_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_UP_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_DOWN_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_DOWN_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_OUT_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_LEFT_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_IN_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_RIGHT_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_UP_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_UP_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_DOWN_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_LOOK_DOWN_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_CLOSED_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_CLOSED_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_CLOSED_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_EYES_CLOSED_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_SQUINT_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LID_TIGHTENER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_SQUINT_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LID_TIGHTENER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_WIDE_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_UPPER_LID_RAISER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_WIDE_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_UPPER_LID_RAISER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_EYE_DILATION_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_DILATION_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_PINCH_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_PINCH_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_LOWERER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_BROW_LOWERER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_BROW_LOWERER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_BROW_LOWERER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_BROW_INNER_UP_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_INNER_BROW_RAISER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_BROW_INNER_UP_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_INNER_BROW_RAISER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_OUTER_BROW_RAISER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_OUTER_BROW_RAISER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_NOSE_SNEER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_NOSE_WRINKLER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_NOSE_SNEER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_NOSE_WRINKLER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_NASAL_DILATION_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_DILATION_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_CHEEK_RAISER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_CHEEK_RAISER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_CHEEK_PUFF_R_ANDROID];
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_CHEEK_PUFF_L_ANDROID];
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_CHEEK_SUCK_R_ANDROID];
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_CHEEK_SUCK_L_ANDROID];
	xr_weights[XRFaceTracker::FT_JAW_OPEN] = parameters[XR_FACE_PARAMETER_INDICES_JAW_DROP_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_CLOSED] = parameters[XR_FACE_PARAMETER_INDICES_LIPS_TOWARD_ANDROID];
	xr_weights[XRFaceTracker::FT_JAW_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_JAW_SIDEWAYS_RIGHT_ANDROID];
	xr_weights[XRFaceTracker::FT_JAW_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_JAW_SIDEWAYS_LEFT_ANDROID];
	xr_weights[XRFaceTracker::FT_JAW_FORWARD] = parameters[XR_FACE_PARAMETER_INDICES_JAW_THRUST_ANDROID];
	xr_weights[XRFaceTracker::FT_JAW_BACKWARD] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_JAW_CLENCH] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_JAW_MANDIBLE_RAISE] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_SUCK_RT_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_SUCK_LT_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_SUCK_RB_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_SUCK_LB_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_CORNER_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_CORNER_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_RT_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_LT_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_RB_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_LB_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_PUCKER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_PUCKER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_PUCKER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_PUCKER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_UPPER_LIP_RAISER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_UPPER_LIP_RAISER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LOWER_LIP_DEPRESSOR_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LOWER_LIP_DEPRESSOR_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_DEEPEN_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_DEEPEN_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_CORNER_PULLER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_CORNER_PULLER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_SLANT_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_SLANT_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_FROWN_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_CORNER_DEPRESSOR_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_FROWN_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_CORNER_DEPRESSOR_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_STRETCHER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_STRETCHER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_DIMPLER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_DIMPLER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_RAISER_UPPER] = parameters[XR_FACE_PARAMETER_INDICES_CHIN_RAISER_T_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_RAISER_LOWER] = parameters[XR_FACE_PARAMETER_INDICES_CHIN_RAISER_B_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_PRESSOR_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_PRESSOR_L_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_TIGHTENER_R_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_LIP_TIGHTENER_L_ANDROID];
	xr_weights[XRFaceTracker::FT_TONGUE_OUT] = parameters[XR_FACE_PARAMETER_INDICES_TONGUE_OUT_ANDROID];
	xr_weights[XRFaceTracker::FT_TONGUE_UP] = parameters[XR_FACE_PARAMETER_INDICES_TONGUE_UP_ANDROID];
	xr_weights[XRFaceTracker::FT_TONGUE_DOWN] = parameters[XR_FACE_PARAMETER_INDICES_TONGUE_DOWN_ANDROID];
	xr_weights[XRFaceTracker::FT_TONGUE_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_TONGUE_RIGHT_ANDROID];
	xr_weights[XRFaceTracker::FT_TONGUE_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_TONGUE_LEFT_ANDROID];
	xr_weights[XRFaceTracker::FT_TONGUE_ROLL] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_BLEND_DOWN] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_CURL_UP] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_SQUISH] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_FLAT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_TWIST_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_TWIST_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_SOFT_PALATE_CLOSE] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_THROAT_SWALLOW] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_NECK_FLEX_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_NECK_FLEX_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking

	// Blended Shapes
	xr_weights[XRFaceTracker::FT_EYE_CLOSED] = average(xr_weights[XRFaceTracker::FT_EYE_CLOSED_RIGHT], xr_weights[XRFaceTracker::FT_EYE_CLOSED_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_WIDE] = average(xr_weights[XRFaceTracker::FT_EYE_WIDE_RIGHT], xr_weights[XRFaceTracker::FT_EYE_WIDE_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_SQUINT] = average(xr_weights[XRFaceTracker::FT_EYE_SQUINT_RIGHT], xr_weights[XRFaceTracker::FT_EYE_SQUINT_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_DILATION] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_DOWN_RIGHT] = xr_weights[XRFaceTracker::FT_BROW_LOWERER_RIGHT];
	xr_weights[XRFaceTracker::FT_BROW_DOWN_LEFT] = xr_weights[XRFaceTracker::FT_BROW_LOWERER_LEFT];
	xr_weights[XRFaceTracker::FT_BROW_DOWN] = average(xr_weights[XRFaceTracker::FT_BROW_DOWN_RIGHT], xr_weights[XRFaceTracker::FT_BROW_DOWN_LEFT]);
	xr_weights[XRFaceTracker::FT_BROW_UP_RIGHT] = average(xr_weights[XRFaceTracker::FT_BROW_INNER_UP_RIGHT], xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_RIGHT]);
	xr_weights[XRFaceTracker::FT_BROW_UP_LEFT] = average(xr_weights[XRFaceTracker::FT_BROW_INNER_UP_LEFT], xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_LEFT]);
	xr_weights[XRFaceTracker::FT_BROW_UP] = average(xr_weights[XRFaceTracker::FT_BROW_UP_RIGHT], xr_weights[XRFaceTracker::FT_BROW_UP_LEFT]);
	xr_weights[XRFaceTracker::FT_NOSE_SNEER] = average(xr_weights[XRFaceTracker::FT_NOSE_SNEER_RIGHT], xr_weights[XRFaceTracker::FT_NOSE_SNEER_LEFT]);
	xr_weights[XRFaceTracker::FT_NASAL_DILATION] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF] = average(xr_weights[XRFaceTracker::FT_CHEEK_PUFF_RIGHT], xr_weights[XRFaceTracker::FT_CHEEK_PUFF_LEFT]);
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK] = average(xr_weights[XRFaceTracker::FT_CHEEK_SUCK_RIGHT], xr_weights[XRFaceTracker::FT_CHEEK_SUCK_LEFT]);
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT] = average(xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_RIGHT], xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER] = average(xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_RIGHT], xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER] = average(xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_RIGHT], xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_SUCK] = average(xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER], xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER]);
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER] = average(xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_RIGHT], xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER] = average(xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_RIGHT], xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL] = average(xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER], xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER]);
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER] = average(xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_RIGHT], xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER] = average(xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_RIGHT], xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_LEFT]);
	xr_weights[XRFaceTracker::FT_LIP_PUCKER] = average(xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER], xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER]);
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP] = average(xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN] = average(xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_OPEN] = average(xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP], xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN]);
	xr_weights[XRFaceTracker::FT_MOUTH_RIGHT] = parameters[XR_FACE_PARAMETER_INDICES_MOUTH_RIGHT_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_LEFT] = parameters[XR_FACE_PARAMETER_INDICES_MOUTH_LEFT_ANDROID];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE_RIGHT] = xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_RIGHT];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE_LEFT] = xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_LEFT];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE] = average(xr_weights[XRFaceTracker::FT_MOUTH_SMILE_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_SMILE_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_SAD_RIGHT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_SAD_LEFT] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_SAD] = 0.0f; // Not measured by XR_ANDROID_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH] = average(xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE] = average(xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER] = average(xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS] = average(xr_weights[XRFaceTracker::FT_MOUTH_PRESS_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_PRESS_LEFT]);

	// Populate the XRFaceTracker
	PackedFloat32Array xr_weights_array;
	xr_weights_array.resize(XRFaceTracker::FT_MAX);
	memcpy(xr_weights_array.ptrw(), xr_weights, sizeof(xr_weights));

	xr_face_tracker->set_blend_shapes(xr_weights_array);
}
