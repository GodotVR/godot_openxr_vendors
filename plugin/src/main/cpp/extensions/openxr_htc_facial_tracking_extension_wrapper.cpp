/**************************************************************************/
/*  openxr_htc_facial_tracking_extension_wrapper.cpp                      */
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

#include "extensions/openxr_htc_facial_tracking_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Calculate the average of two floating point values
static inline constexpr float average(float a, float b) {
	return (a + b) * 0.5f;
}

OpenXRHtcFacialTrackingExtensionWrapper *OpenXRHtcFacialTrackingExtensionWrapper::singleton = nullptr;

OpenXRHtcFacialTrackingExtensionWrapper *OpenXRHtcFacialTrackingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRHtcFacialTrackingExtensionWrapper());
	}
	return singleton;
}

OpenXRHtcFacialTrackingExtensionWrapper::OpenXRHtcFacialTrackingExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRHtcFacialTrackingExtensionWrapper singleton already exists.");

	request_extensions[XR_HTC_FACIAL_TRACKING_EXTENSION_NAME] = &htc_facial_tracking_ext;

	singleton = this;
}

OpenXRHtcFacialTrackingExtensionWrapper::~OpenXRHtcFacialTrackingExtensionWrapper() {
	cleanup();
}

void OpenXRHtcFacialTrackingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRHtcFacialTrackingExtensionWrapper::is_enabled);
}

void OpenXRHtcFacialTrackingExtensionWrapper::cleanup() {
	htc_facial_tracking_ext = false;
}

uint64_t OpenXRHtcFacialTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *next_pointer) {
	system_facial_tracking_properties.type = XR_TYPE_SYSTEM_FACIAL_TRACKING_PROPERTIES_HTC;
	system_facial_tracking_properties.next = next_pointer;
	system_facial_tracking_properties.supportEyeFacialTracking = false;
	system_facial_tracking_properties.supportLipFacialTracking = false;
	return reinterpret_cast<uint64_t>(&system_facial_tracking_properties);
}

godot::Dictionary OpenXRHtcFacialTrackingExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRHtcFacialTrackingExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (htc_facial_tracking_ext) {
		bool result = initialize_htc_facial_tracking_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize htc_facial_tracking extension");
			htc_facial_tracking_ext = false;
		}
	}
}

void OpenXRHtcFacialTrackingExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRHtcFacialTrackingExtensionWrapper::_on_session_created(uint64_t instance) {
	// Skip if not enabled
	if (!is_enabled()) {
		return;
	}

	// Create eye-tracking if supported
	if (system_facial_tracking_properties.supportEyeFacialTracking) {
		// Create the face-tracker handle
		XrFacialTrackerCreateInfoHTC createInfoEye = {
			XR_TYPE_FACIAL_TRACKER_CREATE_INFO_HTC, // type
			nullptr, // next
			XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC, // facialTrackingType
		};
		XrResult result = xrCreateFacialTrackerHTC(SESSION, &createInfoEye, &facial_tracking_eye);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create facial-tracker handle for eye-tracking: ", result);
			return;
		}
	}

	// Create lip-tracking if supported
	if (system_facial_tracking_properties.supportLipFacialTracking) {
		// Create the face-tracker handle
		XrFacialTrackerCreateInfoHTC createInfoLip = {
			XR_TYPE_FACIAL_TRACKER_CREATE_INFO_HTC, // type
			nullptr, // next
			XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC, // facialTrackingType
		};
		XrResult result = xrCreateFacialTrackerHTC(SESSION, &createInfoLip, &facial_tracking_lip);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create facial-tracker handle for lip-tracking: ", result);
			return;
		}
	}

	// Construct the XRFaceTracker if necessary
	if (xr_face_tracker.is_null()) {
		xr_face_tracker.instantiate();
		xr_face_tracker->set_tracker_name("/user/face_tracker");
	}
}

void OpenXRHtcFacialTrackingExtensionWrapper::_on_session_destroyed() {
	// Release eye-tracking if active
	if (facial_tracking_eye) {
		// Destroy the face-tracker handle
		XrResult result = xrDestroyFacialTrackerHTC(facial_tracking_eye);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to destroy facial-tracker handle for eye-tracking: ", result);
		}
		facial_tracking_eye = XR_NULL_HANDLE;
	}

	// Release lip-tracking if active
	if (facial_tracking_lip) {
		// Destroy the face-tracker handle
		XrResult result = xrDestroyFacialTrackerHTC(facial_tracking_lip);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to destroy facial-tracker handle for lip-tracking: ", result);
		}
		facial_tracking_lip = XR_NULL_HANDLE;
	}

	// Unregister the face tracker.
	if (xr_face_tracker_registered) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server && xr_face_tracker.is_valid()) {
			xr_server->remove_tracker(xr_face_tracker);
		}
		xr_face_tracker_registered = false;
	}
}

void OpenXRHtcFacialTrackingExtensionWrapper::_on_process() {
	// Skip if not enabled
	if (!is_enabled()) {
		return;
	}

	// Get the next frame time
	const XrTime display_time = get_openxr_api()->get_predicted_display_time();
	if (display_time == 0) {
		return;
	}

	float eyeWeights[XR_FACIAL_EXPRESSION_EYE_COUNT_HTC] = {};
	float lipWeights[XR_FACIAL_EXPRESSION_LIP_COUNT_HTC] = {};

	// Read the eye weights if supported
	if (facial_tracking_eye) {
		// Construct the expression struct.
		XrFacialExpressionsHTC expression_eye = {
			XR_TYPE_FACIAL_EXPRESSIONS_HTC, // type
			nullptr, // next
			XR_FALSE, // isActive
			display_time, // sampleTime
			XR_FACIAL_EXPRESSION_EYE_COUNT_HTC,
			eyeWeights
		};

		// Read the weights
		XrResult result = xrGetFacialExpressionsHTC(facial_tracking_eye, &expression_eye);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to get facial expression eye weights: ", result);
		}
	}

	// Read the lip weights if supported
	if (facial_tracking_lip) {
		// Construct the expression struct.
		XrFacialExpressionsHTC expression_lip = {
			XR_TYPE_FACIAL_EXPRESSIONS_HTC, // type
			nullptr, // next
			XR_FALSE, // isActive
			display_time, // sampleTime
			XR_FACIAL_EXPRESSION_LIP_COUNT_HTC,
			lipWeights
		};

		// Read the weights
		XrResult result = xrGetFacialExpressionsHTC(facial_tracking_lip, &expression_lip);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to get facial expression lip weights: ", result);
		}
	}

	// Map HTC weights to Godot weights.
	float xr_weights[XRFaceTracker::FT_MAX] = {};

	// Base Shapes
	xr_weights[XRFaceTracker::FT_EYE_LOOK_OUT_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_OUT_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_IN_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_IN_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_UP_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_UP_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_DOWN_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_DOWN_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_OUT_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_OUT_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_IN_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_IN_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_UP_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_UP_HTC];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_DOWN_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_DOWN_HTC];
	xr_weights[XRFaceTracker::FT_EYE_CLOSED_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_BLINK_HTC];
	xr_weights[XRFaceTracker::FT_EYE_CLOSED_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_BLINK_HTC];
	xr_weights[XRFaceTracker::FT_EYE_SQUINT_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_SQUEEZE_HTC];
	xr_weights[XRFaceTracker::FT_EYE_SQUINT_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_SQUEEZE_HTC];
	xr_weights[XRFaceTracker::FT_EYE_WIDE_RIGHT] = eyeWeights[XR_EYE_EXPRESSION_RIGHT_WIDE_HTC];
	xr_weights[XRFaceTracker::FT_EYE_WIDE_LEFT] = eyeWeights[XR_EYE_EXPRESSION_LEFT_WIDE_HTC];
	xr_weights[XRFaceTracker::FT_EYE_DILATION_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_EYE_DILATION_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_PINCH_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_PINCH_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_LOWERER_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_LOWERER_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_INNER_UP_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_INNER_UP_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NOSE_SNEER_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NOSE_SNEER_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NASAL_DILATION_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NASAL_DILATION_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF_RIGHT] = lipWeights[XR_LIP_EXPRESSION_CHEEK_PUFF_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF_LEFT] = lipWeights[XR_LIP_EXPRESSION_CHEEK_PUFF_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK_RIGHT] = lipWeights[XR_LIP_EXPRESSION_CHEEK_SUCK_HTC];
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK_LEFT] = lipWeights[XR_LIP_EXPRESSION_CHEEK_SUCK_HTC];
	xr_weights[XRFaceTracker::FT_JAW_OPEN] = lipWeights[XR_LIP_EXPRESSION_JAW_OPEN_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_CLOSED] = lipWeights[XR_LIP_EXPRESSION_MOUTH_APE_SHAPE_HTC];
	xr_weights[XRFaceTracker::FT_JAW_RIGHT] = lipWeights[XR_LIP_EXPRESSION_JAW_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_JAW_LEFT] = lipWeights[XR_LIP_EXPRESSION_JAW_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_JAW_FORWARD] = lipWeights[XR_LIP_EXPRESSION_JAW_FORWARD_HTC];
	xr_weights[XRFaceTracker::FT_JAW_BACKWARD] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_JAW_CLENCH] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_JAW_MANDIBLE_RAISE] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_INSIDE_HTC];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_INSIDE_HTC];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_INSIDE_HTC];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_INSIDE_HTC];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_CORNER_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_CORNER_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_UPRIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_UPLEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_DOWNRIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_DOWNLEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_DEEPEN_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_DEEPEN_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SMILE_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SMILE_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_SLANT_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_SLANT_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_FROWN_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SAD_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_FROWN_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SAD_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_RAISER_UPPER] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_RAISER_LOWER] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_OUT] = lipWeights[XR_LIP_EXPRESSION_TONGUE_LONGSTEP2_HTC];
	xr_weights[XRFaceTracker::FT_TONGUE_UP] = lipWeights[XR_LIP_EXPRESSION_TONGUE_UP_HTC];
	xr_weights[XRFaceTracker::FT_TONGUE_DOWN] = lipWeights[XR_LIP_EXPRESSION_TONGUE_DOWN_HTC];
	xr_weights[XRFaceTracker::FT_TONGUE_RIGHT] = lipWeights[XR_LIP_EXPRESSION_TONGUE_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_TONGUE_LEFT] = lipWeights[XR_LIP_EXPRESSION_TONGUE_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_TONGUE_ROLL] = lipWeights[XR_LIP_EXPRESSION_TONGUE_ROLL_HTC];
	xr_weights[XRFaceTracker::FT_TONGUE_BLEND_DOWN] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_CURL_UP] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_SQUISH] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_FLAT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_TWIST_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_TONGUE_TWIST_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_SOFT_PALATE_CLOSE] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_THROAT_SWALLOW] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NECK_FLEX_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NECK_FLEX_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking

	// Blended Shapes
	xr_weights[XRFaceTracker::FT_EYE_CLOSED] = average(xr_weights[XRFaceTracker::FT_EYE_CLOSED_RIGHT], xr_weights[XRFaceTracker::FT_EYE_CLOSED_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_WIDE] = average(xr_weights[XRFaceTracker::FT_EYE_WIDE_RIGHT], xr_weights[XRFaceTracker::FT_EYE_WIDE_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_SQUINT] = average(xr_weights[XRFaceTracker::FT_EYE_SQUINT_RIGHT], xr_weights[XRFaceTracker::FT_EYE_SQUINT_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_DILATION] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_DOWN_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_DOWN_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_DOWN] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_UP_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_UP_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_BROW_UP] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NOSE_SNEER] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NASAL_DILATION] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF] = average(xr_weights[XRFaceTracker::FT_CHEEK_PUFF_RIGHT], xr_weights[XRFaceTracker::FT_CHEEK_PUFF_LEFT]);
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK] = lipWeights[XR_LIP_EXPRESSION_CHEEK_SUCK_HTC];
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER] = lipWeights[XR_LIP_EXPRESSION_MOUTH_UPPER_INSIDE_HTC];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER] = lipWeights[XR_LIP_EXPRESSION_MOUTH_LOWER_INSIDE_HTC];
	xr_weights[XRFaceTracker::FT_LIP_SUCK] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER] = lipWeights[XR_LIP_EXPRESSION_MOUTH_POUT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP] = average(xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN] = average(xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_OPEN] = average(xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP], xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN]);
	xr_weights[XRFaceTracker::FT_MOUTH_RIGHT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_LEFT] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SMILE_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SMILE_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE] = average(xr_weights[XRFaceTracker::FT_MOUTH_SMILE_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_SMILE_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_SAD_RIGHT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SAD_RIGHT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_SAD_LEFT] = lipWeights[XR_LIP_EXPRESSION_MOUTH_SAD_LEFT_HTC];
	xr_weights[XRFaceTracker::FT_MOUTH_SAD] = average(xr_weights[XRFaceTracker::FT_MOUTH_SAD_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_SAD_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER] = 0.0f; // Not measured by XR_htc_facial_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS] = 0.0f; // Not measured by XR_htc_facial_tracking

	// Populate the XRFaceTracker
	PackedFloat32Array xr_weights_array;
	xr_weights_array.resize(XRFaceTracker::FT_MAX);
	memcpy(xr_weights_array.ptrw(), xr_weights, sizeof(xr_weights));
	xr_face_tracker->set_blend_shapes(xr_weights_array);

	// Register the XRFaceTracker if necessary
	if (!xr_face_tracker_registered) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server) {
			xr_server->add_tracker(xr_face_tracker);
			xr_face_tracker_registered = true;
		}
	}
}

bool OpenXRHtcFacialTrackingExtensionWrapper::is_enabled() const {
	return htc_facial_tracking_ext && (system_facial_tracking_properties.supportEyeFacialTracking || system_facial_tracking_properties.supportLipFacialTracking);
}

bool OpenXRHtcFacialTrackingExtensionWrapper::initialize_htc_facial_tracking_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateFacialTrackerHTC);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyFacialTrackerHTC);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetFacialExpressionsHTC);

	return true;
}
