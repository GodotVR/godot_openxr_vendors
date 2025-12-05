/**************************************************************************/
/*  openxr_fb_face_tracking_extension_wrapper.cpp                         */
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

#include "extensions/openxr_fb_face_tracking_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Calculate the average of two floating point values
static inline constexpr float average(float a, float b) {
	return (a + b) * 0.5f;
}

OpenXRFbFaceTrackingExtensionWrapper *OpenXRFbFaceTrackingExtensionWrapper::singleton = nullptr;

OpenXRFbFaceTrackingExtensionWrapper *OpenXRFbFaceTrackingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbFaceTrackingExtensionWrapper());
	}
	return singleton;
}

OpenXRFbFaceTrackingExtensionWrapper::OpenXRFbFaceTrackingExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbFaceTrackingExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_FACE_TRACKING2_EXTENSION_NAME] = &fb_face_tracking2_ext;

	singleton = this;
}

OpenXRFbFaceTrackingExtensionWrapper::~OpenXRFbFaceTrackingExtensionWrapper() {
	cleanup();
}

void OpenXRFbFaceTrackingExtensionWrapper::_bind_methods() {
}

void OpenXRFbFaceTrackingExtensionWrapper::cleanup() {
	fb_face_tracking2_ext = false;
}

uint64_t OpenXRFbFaceTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *next_pointer) {
	system_face_tracking_properties2.type = XR_TYPE_SYSTEM_FACE_TRACKING_PROPERTIES2_FB;
	system_face_tracking_properties2.next = next_pointer;
	system_face_tracking_properties2.supportsVisualFaceTracking = false;
	system_face_tracking_properties2.supportsAudioFaceTracking = false;
	return reinterpret_cast<uint64_t>(&system_face_tracking_properties2);
}

godot::Dictionary OpenXRFbFaceTrackingExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_face_tracking2_ext) {
		bool result = initialize_fb_face_tracking2_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_face_tracking2 extension");
			fb_face_tracking2_ext = false;
		}
	}
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_session_created(uint64_t instance) {
	// Skip if not enabled
	if (!is_enabled()) {
		return;
	}

	// Create the face-tracker handle
	XrFaceTrackingDataSource2FB dataSources[2] = {
		XR_FACE_TRACKING_DATA_SOURCE2_VISUAL_FB,
		XR_FACE_TRACKING_DATA_SOURCE2_AUDIO_FB
	};
	XrFaceTrackerCreateInfo2FB createInfo2 = {
		XR_TYPE_FACE_TRACKER_CREATE_INFO2_FB, // type
		nullptr, // next
		XR_FACE_EXPRESSION_SET2_DEFAULT_FB, // faceExpressionSet
		2, // requestedDataSourceCount
		dataSources // requestedDataSources
	};
	XrResult result = xrCreateFaceTracker2FB(SESSION, &createInfo2, &face_tracker2);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create face-tracker handle: ", result);
		return;
	}

	// Construct the XRFaceTracker if necessary
	if (xr_face_tracker.is_null()) {
		xr_face_tracker.instantiate();
		xr_face_tracker->set_tracker_name("/user/face_tracker");
	}
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_session_destroyed() {
	// Skip if no face-tracker handle
	if (!face_tracker2) {
		return;
	}

	// Destroy the face-tracker handle
	XrResult result = xrDestroyFaceTracker2FB(face_tracker2);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to destroy face-tracker handle: ", result);
	}
	face_tracker2 = XR_NULL_HANDLE;

	// Unregister the face tracker.
	if (xr_face_tracker_registered) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server && xr_face_tracker.is_valid()) {
			xr_server->remove_tracker(xr_face_tracker);
		}
	}
	xr_face_tracker_registered = false;
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_process() {
	// Skip if not enabled, or no face-tracker handle
	if (!is_enabled() || !face_tracker2) {
		return;
	}

	// Get the next frame time
	const XrTime display_time = get_openxr_api()->get_predicted_display_time();
	if (display_time == 0) {
		return;
	}

	// Construct the expression info struct.
	XrFaceExpressionInfo2FB expression_info2 = {
		XR_TYPE_FACE_EXPRESSION_INFO2_FB, // type
		nullptr, // next
		display_time // time
	};

	// Construct the weights struct.
	float fb_weights[XR_FACE_EXPRESSION2_COUNT_FB] = {};
	float fb_confidences[XR_FACE_CONFIDENCE2_COUNT_FB] = {};
	XrFaceExpressionWeights2FB face_expression_weights2 = {
		XR_TYPE_FACE_EXPRESSION_WEIGHTS2_FB, // type
		nullptr, // next
		XR_FACE_EXPRESSION2_COUNT_FB, // weightCount
		fb_weights, // weights
		XR_FACE_CONFIDENCE2_COUNT_FB, // confidenceCount
		fb_confidences // confidences
	};

	// Read the weights
	XrResult result = xrGetFaceExpressionWeights2FB(face_tracker2, &expression_info2, &face_expression_weights2);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get face expression weights: ", result);
	}

	// Map Meta weights to Godot weights.
	float xr_weights[XRFaceTracker::FT_MAX] = {};

	// Base Shapes
	xr_weights[XRFaceTracker::FT_EYE_LOOK_OUT_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_RIGHT_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_IN_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_LEFT_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_UP_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_UP_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_DOWN_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_DOWN_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_OUT_LEFT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_LEFT_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_IN_LEFT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_RIGHT_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_UP_LEFT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_UP_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_LOOK_DOWN_LEFT] = fb_weights[XR_FACE_EXPRESSION2_EYES_LOOK_DOWN_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_CLOSED_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_EYES_CLOSED_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_CLOSED_LEFT] = fb_weights[XR_FACE_EXPRESSION2_EYES_CLOSED_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_SQUINT_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LID_TIGHTENER_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_SQUINT_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LID_TIGHTENER_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_WIDE_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_UPPER_LID_RAISER_R_FB];
	xr_weights[XRFaceTracker::FT_EYE_WIDE_LEFT] = fb_weights[XR_FACE_EXPRESSION2_UPPER_LID_RAISER_L_FB];
	xr_weights[XRFaceTracker::FT_EYE_DILATION_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_DILATION_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_PINCH_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_PINCH_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_LOWERER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_BROW_LOWERER_R_FB];
	xr_weights[XRFaceTracker::FT_BROW_LOWERER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_BROW_LOWERER_L_FB];
	xr_weights[XRFaceTracker::FT_BROW_INNER_UP_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_INNER_BROW_RAISER_R_FB];
	xr_weights[XRFaceTracker::FT_BROW_INNER_UP_LEFT] = fb_weights[XR_FACE_EXPRESSION2_INNER_BROW_RAISER_L_FB];
	xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_OUTER_BROW_RAISER_R_FB];
	xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_LEFT] = fb_weights[XR_FACE_EXPRESSION2_OUTER_BROW_RAISER_L_FB];
	xr_weights[XRFaceTracker::FT_NOSE_SNEER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_NOSE_WRINKLER_R_FB];
	xr_weights[XRFaceTracker::FT_NOSE_SNEER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_NOSE_WRINKLER_L_FB];
	xr_weights[XRFaceTracker::FT_NASAL_DILATION_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_DILATION_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_CHEEK_RAISER_R_FB];
	xr_weights[XRFaceTracker::FT_CHEEK_SQUINT_LEFT] = fb_weights[XR_FACE_EXPRESSION2_CHEEK_RAISER_L_FB];
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_CHEEK_PUFF_R_FB];
	xr_weights[XRFaceTracker::FT_CHEEK_PUFF_LEFT] = fb_weights[XR_FACE_EXPRESSION2_CHEEK_PUFF_L_FB];
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_CHEEK_SUCK_R_FB];
	xr_weights[XRFaceTracker::FT_CHEEK_SUCK_LEFT] = fb_weights[XR_FACE_EXPRESSION2_CHEEK_SUCK_L_FB];
	xr_weights[XRFaceTracker::FT_JAW_OPEN] = fb_weights[XR_FACE_EXPRESSION2_JAW_DROP_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_CLOSED] = fb_weights[XR_FACE_EXPRESSION2_LIPS_TOWARD_FB];
	xr_weights[XRFaceTracker::FT_JAW_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_JAW_SIDEWAYS_RIGHT_FB];
	xr_weights[XRFaceTracker::FT_JAW_LEFT] = fb_weights[XR_FACE_EXPRESSION2_JAW_SIDEWAYS_LEFT_FB];
	xr_weights[XRFaceTracker::FT_JAW_FORWARD] = fb_weights[XR_FACE_EXPRESSION2_JAW_THRUST_FB];
	xr_weights[XRFaceTracker::FT_JAW_BACKWARD] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_JAW_CLENCH] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_JAW_MANDIBLE_RAISE] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_SUCK_RT_FB];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_UPPER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_SUCK_LT_FB];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_SUCK_RB_FB];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_LOWER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_SUCK_LB_FB];
	xr_weights[XRFaceTracker::FT_LIP_SUCK_CORNER_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_LIP_SUCK_CORNER_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_FUNNELER_RT_FB];
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_UPPER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_FUNNELER_LT_FB];
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_FUNNELER_RB_FB];
	xr_weights[XRFaceTracker::FT_LIP_FUNNEL_LOWER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_FUNNELER_LB_FB];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_PUCKER_R_FB];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_UPPER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_PUCKER_L_FB];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_PUCKER_R_FB];
	xr_weights[XRFaceTracker::FT_LIP_PUCKER_LOWER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_PUCKER_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_UPPER_LIP_RAISER_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_UP_LEFT] = fb_weights[XR_FACE_EXPRESSION2_UPPER_LIP_RAISER_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LOWER_LIP_DEPRESSOR_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_DOWN_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LOWER_LIP_DEPRESSOR_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_DEEPEN_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_DEEPEN_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_UPPER_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_LOWER_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_CORNER_PULLER_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_PULL_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_CORNER_PULLER_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_SLANT_RIGHT] = 0.0f;
	xr_weights[XRFaceTracker::FT_MOUTH_CORNER_SLANT_LEFT] = 0.0f;
	xr_weights[XRFaceTracker::FT_MOUTH_FROWN_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_CORNER_DEPRESSOR_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_FROWN_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_CORNER_DEPRESSOR_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_STRETCHER_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_STRETCHER_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_DIMPLER_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_LEFT] = fb_weights[XR_FACE_EXPRESSION2_DIMPLER_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_RAISER_UPPER] = fb_weights[XR_FACE_EXPRESSION2_CHIN_RAISER_T_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_RAISER_LOWER] = fb_weights[XR_FACE_EXPRESSION2_CHIN_RAISER_B_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_PRESSOR_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_PRESSOR_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_RIGHT] = fb_weights[XR_FACE_EXPRESSION2_LIP_TIGHTENER_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_LEFT] = fb_weights[XR_FACE_EXPRESSION2_LIP_TIGHTENER_L_FB];
	xr_weights[XRFaceTracker::FT_TONGUE_OUT] = fb_weights[XR_FACE_EXPRESSION2_TONGUE_OUT_FB];
	xr_weights[XRFaceTracker::FT_TONGUE_UP] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_DOWN] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_RIGHT] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_LEFT] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_ROLL] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_BLEND_DOWN] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_CURL_UP] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_SQUISH] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_FLAT] = fb_weights[XR_FACE_EXPRESSION2_TONGUE_RETREAT_FB];
	xr_weights[XRFaceTracker::FT_TONGUE_TWIST_RIGHT] = 0.0f;
	xr_weights[XRFaceTracker::FT_TONGUE_TWIST_LEFT] = 0.0f;
	xr_weights[XRFaceTracker::FT_SOFT_PALATE_CLOSE] = 0.0f;
	xr_weights[XRFaceTracker::FT_THROAT_SWALLOW] = 0.0f;
	xr_weights[XRFaceTracker::FT_NECK_FLEX_RIGHT] = 0.0f;
	xr_weights[XRFaceTracker::FT_NECK_FLEX_LEFT] = 0.0f;

	// Blended Shapes
	xr_weights[XRFaceTracker::FT_EYE_CLOSED] = average(xr_weights[XRFaceTracker::FT_EYE_CLOSED_RIGHT], xr_weights[XRFaceTracker::FT_EYE_CLOSED_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_WIDE] = average(xr_weights[XRFaceTracker::FT_EYE_WIDE_RIGHT], xr_weights[XRFaceTracker::FT_EYE_WIDE_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_SQUINT] = average(xr_weights[XRFaceTracker::FT_EYE_SQUINT_RIGHT], xr_weights[XRFaceTracker::FT_EYE_SQUINT_LEFT]);
	xr_weights[XRFaceTracker::FT_EYE_DILATION] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_EYE_CONSTRICT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_BROW_DOWN_RIGHT] = fb_weights[XR_FACE_EXPRESSION_BROW_LOWERER_R_FB];
	xr_weights[XRFaceTracker::FT_BROW_DOWN_LEFT] = fb_weights[XR_FACE_EXPRESSION_BROW_LOWERER_L_FB];
	xr_weights[XRFaceTracker::FT_BROW_DOWN] = average(xr_weights[XRFaceTracker::FT_BROW_DOWN_RIGHT], xr_weights[XRFaceTracker::FT_BROW_DOWN_LEFT]);
	xr_weights[XRFaceTracker::FT_BROW_UP_RIGHT] = average(xr_weights[XRFaceTracker::FT_BROW_INNER_UP_RIGHT], xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_RIGHT]);
	xr_weights[XRFaceTracker::FT_BROW_UP_LEFT] = average(xr_weights[XRFaceTracker::FT_BROW_INNER_UP_LEFT], xr_weights[XRFaceTracker::FT_BROW_OUTER_UP_LEFT]);
	xr_weights[XRFaceTracker::FT_BROW_UP] = average(xr_weights[XRFaceTracker::FT_BROW_UP_RIGHT], xr_weights[XRFaceTracker::FT_BROW_UP_LEFT]);
	xr_weights[XRFaceTracker::FT_NOSE_SNEER] = average(xr_weights[XRFaceTracker::FT_NOSE_SNEER_RIGHT], xr_weights[XRFaceTracker::FT_NOSE_SNEER_LEFT]);
	xr_weights[XRFaceTracker::FT_NASAL_DILATION] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_NASAL_CONSTRICT] = 0.0f; // Not measured by XR_fb_face_tracking
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
	xr_weights[XRFaceTracker::FT_MOUTH_RIGHT] = fb_weights[XR_FACE_EXPRESSION_MOUTH_RIGHT_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_LEFT] = fb_weights[XR_FACE_EXPRESSION_MOUTH_LEFT_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE_RIGHT] = fb_weights[XR_FACE_EXPRESSION_LIP_CORNER_PULLER_R_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE_LEFT] = fb_weights[XR_FACE_EXPRESSION_LIP_CORNER_PULLER_L_FB];
	xr_weights[XRFaceTracker::FT_MOUTH_SMILE] = average(xr_weights[XRFaceTracker::FT_MOUTH_SMILE_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_SMILE_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_SAD_RIGHT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_SAD_LEFT] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_SAD] = 0.0f; // Not measured by XR_fb_face_tracking
	xr_weights[XRFaceTracker::FT_MOUTH_STRETCH] = average(xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_STRETCH_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE] = average(xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_DIMPLE_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER] = average(xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_TIGHTENER_LEFT]);
	xr_weights[XRFaceTracker::FT_MOUTH_PRESS] = average(xr_weights[XRFaceTracker::FT_MOUTH_PRESS_RIGHT], xr_weights[XRFaceTracker::FT_MOUTH_PRESS_LEFT]);

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

bool OpenXRFbFaceTrackingExtensionWrapper::is_enabled() const {
	return fb_face_tracking2_ext && (system_face_tracking_properties2.supportsVisualFaceTracking || system_face_tracking_properties2.supportsAudioFaceTracking);
}

bool OpenXRFbFaceTrackingExtensionWrapper::initialize_fb_face_tracking2_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateFaceTracker2FB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyFaceTracker2FB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetFaceExpressionWeights2FB);

	return true;
}
