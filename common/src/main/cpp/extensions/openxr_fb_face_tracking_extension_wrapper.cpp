/**************************************************************************/
/*  openxr_fb_face_tracking_extension_wrapper.cpp                         */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Original contributed implementation:                                   */
/*   Copyright (c) 2024 Malcolm Nixon                                     */
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

#include "include/openxr_fb_face_tracking_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

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

	request_extensions[XR_FB_FACE_TRACKING_EXTENSION_NAME] = &fb_face_tracking_ext;
	singleton = this;
}

OpenXRFbFaceTrackingExtensionWrapper::~OpenXRFbFaceTrackingExtensionWrapper() {
	cleanup();
}

void OpenXRFbFaceTrackingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbFaceTrackingExtensionWrapper::is_enabled);
	ClassDB::bind_method(D_METHOD("get_weights"), &OpenXRFbFaceTrackingExtensionWrapper::get_weights);
	ClassDB::bind_method(D_METHOD("get_confidences"), &OpenXRFbFaceTrackingExtensionWrapper::get_confidences);

	BIND_ENUM_CONSTANT(EXPRESSION_BROW_LOWERER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_BROW_LOWERER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_CHEEK_PUFF_L);
	BIND_ENUM_CONSTANT(EXPRESSION_CHEEK_PUFF_R);
	BIND_ENUM_CONSTANT(EXPRESSION_CHEEK_RAISER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_CHEEK_RAISER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_CHEEK_SUCK_L);
	BIND_ENUM_CONSTANT(EXPRESSION_CHEEK_SUCK_R);
	BIND_ENUM_CONSTANT(EXPRESSION_CHIN_RAISER_B);
	BIND_ENUM_CONSTANT(EXPRESSION_CHIN_RAISER_T);
	BIND_ENUM_CONSTANT(EXPRESSION_DIMPLER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_DIMPLER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_CLOSED_L);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_CLOSED_R);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_DOWN_L);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_DOWN_R);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_LEFT_L);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_LEFT_R);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_RIGHT_L);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_RIGHT_R);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_UP_L);
	BIND_ENUM_CONSTANT(EXPRESSION_EYES_LOOK_UP_R);
	BIND_ENUM_CONSTANT(EXPRESSION_INNER_BROW_RAISER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_INNER_BROW_RAISER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_JAW_DROP);
	BIND_ENUM_CONSTANT(EXPRESSION_JAW_SIDEWAYS_LEFT);
	BIND_ENUM_CONSTANT(EXPRESSION_JAW_SIDEWAYS_RIGHT);
	BIND_ENUM_CONSTANT(EXPRESSION_JAW_THRUST);
	BIND_ENUM_CONSTANT(EXPRESSION_LID_TIGHTENER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LID_TIGHTENER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_CORNER_DEPRESSOR_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_CORNER_DEPRESSOR_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_CORNER_PULLER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_CORNER_PULLER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_FUNNELER_LB);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_FUNNELER_LT);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_FUNNELER_RB);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_FUNNELER_RT);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_PRESSOR_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_PRESSOR_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_PUCKER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_PUCKER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_STRETCHER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_STRETCHER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_SUCK_LB);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_SUCK_LT);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_SUCK_RB);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_SUCK_RT);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_TIGHTENER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LIP_TIGHTENER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_LIPS_TOWARD);
	BIND_ENUM_CONSTANT(EXPRESSION_LOWER_LIP_DEPRESSOR_L);
	BIND_ENUM_CONSTANT(EXPRESSION_LOWER_LIP_DEPRESSOR_R);
	BIND_ENUM_CONSTANT(EXPRESSION_MOUTH_LEFT);
	BIND_ENUM_CONSTANT(EXPRESSION_MOUTH_RIGHT);
	BIND_ENUM_CONSTANT(EXPRESSION_NOSE_WRINKLER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_NOSE_WRINKLER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_OUTER_BROW_RAISER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_OUTER_BROW_RAISER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_UPPER_LID_RAISER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_UPPER_LID_RAISER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_UPPER_LIP_RAISER_L);
	BIND_ENUM_CONSTANT(EXPRESSION_UPPER_LIP_RAISER_R);
	BIND_ENUM_CONSTANT(EXPRESSION_COUNT);
	BIND_ENUM_CONSTANT(CONFIDENCE_LOWER_FACE);
	BIND_ENUM_CONSTANT(CONFIDENCE_UPPER_FACE);
	BIND_ENUM_CONSTANT(CONFIDENCE_COUNT);
}

void OpenXRFbFaceTrackingExtensionWrapper::cleanup() {
	fb_face_tracking_ext = false;
}

uint64_t OpenXRFbFaceTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *next_pointer) {
	system_face_tracking_properties.type = XR_TYPE_SYSTEM_FACE_TRACKING_PROPERTIES_FB;
	system_face_tracking_properties.next = next_pointer;
	system_face_tracking_properties.supportsFaceTracking = false;
	return reinterpret_cast<uint64_t>(&system_face_tracking_properties);
}

godot::Dictionary OpenXRFbFaceTrackingExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_face_tracking_ext) {
		bool result = initialize_fb_face_tracking_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_face_tracking extension");
			fb_face_tracking_ext = false;
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

	// Configure the weights struct
	face_expression_weights.type = XR_TYPE_FACE_EXPRESSION_WEIGHTS_FB;
	face_expression_weights.weightCount = XR_FACE_EXPRESSION_COUNT_FB;
	face_expression_weights.weights = weights;
	face_expression_weights.confidenceCount  = XR_FACE_CONFIDENCE_COUNT_FB;
	face_expression_weights.confidences = confidences;

	// Create the face-tracker handle
	XrFaceTrackerCreateInfoFB createInfo = {XR_TYPE_FACE_TRACKER_CREATE_INFO_FB};
	createInfo.faceExpressionSet = XR_FACE_EXPRESSION_SET_DEFAULT_FB;
	XrResult result = xrCreateFaceTrackerFB(SESSION, &createInfo, &face_tracker);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create face-tracker handle: ", result);
	}
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_session_destroyed() {
	// Skip if no face-tracker handle
	if (!face_tracker) {
		return;
	}

	// Destroy the face-tracker handle
	XrResult result = xrDestroyFaceTrackerFB(face_tracker);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to destroy face-tracker handle: ", result);
	}
	face_tracker = XR_NULL_HANDLE;

	// Clear the weights
	face_expression_weights.status.isValid = XR_FALSE;
}

void OpenXRFbFaceTrackingExtensionWrapper::_on_process() {
	// Skip if not enabled, or no face-tracker handle
	if (!is_enabled() || !face_tracker) {
		return;
	}

	// Get the next frame time
	const XrTime next_frame_time = get_openxr_api()->get_next_frame_time();
	if (next_frame_time == 0) {
		return;
	}

	// Read the expression weights
	XrFaceExpressionInfoFB expression_info = {XR_TYPE_FACE_EXPRESSION_INFO_FB};
	expression_info.time = next_frame_time;
	XrResult result = xrGetFaceExpressionWeightsFB(face_tracker, &expression_info, &face_expression_weights);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get face expression weights: ", result);
	}
}

bool OpenXRFbFaceTrackingExtensionWrapper::is_enabled() const {
	return fb_face_tracking_ext && system_face_tracking_properties.supportsFaceTracking;
}

PackedFloat32Array OpenXRFbFaceTrackingExtensionWrapper::get_weights() const {
	PackedFloat32Array ret;

	// If valid, populate the array with weights
	if (face_expression_weights.status.isValid) {
		// This might be done faster with a memcpy, but this approach ensures
		// the size and order of the Godot array matches the Godot enum and
		// should be immune from changes or additional weights in the OpenXR data.
		ret.resize(EXPRESSION_COUNT);
		ret[EXPRESSION_BROW_LOWERER_L] = weights[XR_FACE_EXPRESSION_BROW_LOWERER_L_FB];
		ret[EXPRESSION_BROW_LOWERER_R] = weights[XR_FACE_EXPRESSION_BROW_LOWERER_R_FB];
		ret[EXPRESSION_CHEEK_PUFF_L] = weights[XR_FACE_EXPRESSION_CHEEK_PUFF_L_FB];
		ret[EXPRESSION_CHEEK_PUFF_R] = weights[XR_FACE_EXPRESSION_CHEEK_PUFF_R_FB];
		ret[EXPRESSION_CHEEK_RAISER_L] = weights[XR_FACE_EXPRESSION_CHEEK_RAISER_L_FB];
		ret[EXPRESSION_CHEEK_RAISER_R] = weights[XR_FACE_EXPRESSION_CHEEK_RAISER_R_FB];
		ret[EXPRESSION_CHEEK_SUCK_L] = weights[XR_FACE_EXPRESSION_CHEEK_SUCK_L_FB];
		ret[EXPRESSION_CHEEK_SUCK_R] = weights[XR_FACE_EXPRESSION_CHEEK_SUCK_R_FB];
		ret[EXPRESSION_CHIN_RAISER_B] = weights[XR_FACE_EXPRESSION_CHIN_RAISER_B_FB];
		ret[EXPRESSION_CHIN_RAISER_T] = weights[XR_FACE_EXPRESSION_CHIN_RAISER_T_FB];
		ret[EXPRESSION_DIMPLER_L] = weights[XR_FACE_EXPRESSION_DIMPLER_L_FB];
		ret[EXPRESSION_DIMPLER_R] = weights[XR_FACE_EXPRESSION_DIMPLER_R_FB];
		ret[EXPRESSION_EYES_CLOSED_L] = weights[XR_FACE_EXPRESSION_EYES_CLOSED_L_FB];
		ret[EXPRESSION_EYES_CLOSED_R] = weights[XR_FACE_EXPRESSION_EYES_CLOSED_R_FB];
		ret[EXPRESSION_EYES_LOOK_DOWN_L] = weights[XR_FACE_EXPRESSION_EYES_LOOK_DOWN_L_FB];
		ret[EXPRESSION_EYES_LOOK_DOWN_R] = weights[XR_FACE_EXPRESSION_EYES_LOOK_DOWN_R_FB];
		ret[EXPRESSION_EYES_LOOK_LEFT_L] = weights[XR_FACE_EXPRESSION_EYES_LOOK_LEFT_L_FB];
		ret[EXPRESSION_EYES_LOOK_LEFT_R] = weights[XR_FACE_EXPRESSION_EYES_LOOK_LEFT_R_FB];
		ret[EXPRESSION_EYES_LOOK_RIGHT_L] = weights[XR_FACE_EXPRESSION_EYES_LOOK_RIGHT_L_FB];
		ret[EXPRESSION_EYES_LOOK_RIGHT_R] = weights[XR_FACE_EXPRESSION_EYES_LOOK_RIGHT_R_FB];
		ret[EXPRESSION_EYES_LOOK_UP_L] = weights[XR_FACE_EXPRESSION_EYES_LOOK_UP_L_FB];
		ret[EXPRESSION_EYES_LOOK_UP_R] = weights[XR_FACE_EXPRESSION_EYES_LOOK_UP_R_FB];
		ret[EXPRESSION_INNER_BROW_RAISER_L] = weights[XR_FACE_EXPRESSION_INNER_BROW_RAISER_L_FB];
		ret[EXPRESSION_INNER_BROW_RAISER_R] = weights[XR_FACE_EXPRESSION_INNER_BROW_RAISER_R_FB];
		ret[EXPRESSION_JAW_DROP] = weights[XR_FACE_EXPRESSION_JAW_DROP_FB];
		ret[EXPRESSION_JAW_SIDEWAYS_LEFT] = weights[XR_FACE_EXPRESSION_JAW_SIDEWAYS_LEFT_FB];
		ret[EXPRESSION_JAW_SIDEWAYS_RIGHT] = weights[XR_FACE_EXPRESSION_JAW_SIDEWAYS_RIGHT_FB];
		ret[EXPRESSION_JAW_THRUST] = weights[XR_FACE_EXPRESSION_JAW_THRUST_FB];
		ret[EXPRESSION_LID_TIGHTENER_L] = weights[XR_FACE_EXPRESSION_LID_TIGHTENER_L_FB];
		ret[EXPRESSION_LID_TIGHTENER_R] = weights[XR_FACE_EXPRESSION_LID_TIGHTENER_R_FB];
		ret[EXPRESSION_LIP_CORNER_DEPRESSOR_L] = weights[XR_FACE_EXPRESSION_LIP_CORNER_DEPRESSOR_L_FB];
		ret[EXPRESSION_LIP_CORNER_DEPRESSOR_R] = weights[XR_FACE_EXPRESSION_LIP_CORNER_DEPRESSOR_R_FB];
		ret[EXPRESSION_LIP_CORNER_PULLER_L] = weights[XR_FACE_EXPRESSION_LIP_CORNER_PULLER_L_FB];
		ret[EXPRESSION_LIP_CORNER_PULLER_R] = weights[XR_FACE_EXPRESSION_LIP_CORNER_PULLER_R_FB];
		ret[EXPRESSION_LIP_FUNNELER_LB] = weights[XR_FACE_EXPRESSION_LIP_FUNNELER_LB_FB];
		ret[EXPRESSION_LIP_FUNNELER_LT] = weights[XR_FACE_EXPRESSION_LIP_FUNNELER_LT_FB];
		ret[EXPRESSION_LIP_FUNNELER_RB] = weights[XR_FACE_EXPRESSION_LIP_FUNNELER_RB_FB];
		ret[EXPRESSION_LIP_FUNNELER_RT] = weights[XR_FACE_EXPRESSION_LIP_FUNNELER_RT_FB];
		ret[EXPRESSION_LIP_PRESSOR_L] = weights[XR_FACE_EXPRESSION_LIP_PRESSOR_L_FB];
		ret[EXPRESSION_LIP_PRESSOR_R] = weights[XR_FACE_EXPRESSION_LIP_PRESSOR_R_FB];
		ret[EXPRESSION_LIP_PUCKER_L] = weights[XR_FACE_EXPRESSION_LIP_PUCKER_L_FB];
		ret[EXPRESSION_LIP_PUCKER_R] = weights[XR_FACE_EXPRESSION_LIP_PUCKER_R_FB];
		ret[EXPRESSION_LIP_STRETCHER_L] = weights[XR_FACE_EXPRESSION_LIP_STRETCHER_L_FB];
		ret[EXPRESSION_LIP_STRETCHER_R] = weights[XR_FACE_EXPRESSION_LIP_STRETCHER_R_FB];
		ret[EXPRESSION_LIP_SUCK_LB] = weights[XR_FACE_EXPRESSION_LIP_SUCK_LB_FB];
		ret[EXPRESSION_LIP_SUCK_LT] = weights[XR_FACE_EXPRESSION_LIP_SUCK_LT_FB];
		ret[EXPRESSION_LIP_SUCK_RB] = weights[XR_FACE_EXPRESSION_LIP_SUCK_RB_FB];
		ret[EXPRESSION_LIP_SUCK_RT] = weights[XR_FACE_EXPRESSION_LIP_SUCK_RT_FB];
		ret[EXPRESSION_LIP_TIGHTENER_L] = weights[XR_FACE_EXPRESSION_LIP_TIGHTENER_L_FB];
		ret[EXPRESSION_LIP_TIGHTENER_R] = weights[XR_FACE_EXPRESSION_LIP_TIGHTENER_R_FB];
		ret[EXPRESSION_LIPS_TOWARD] = weights[XR_FACE_EXPRESSION_LIPS_TOWARD_FB];
		ret[EXPRESSION_LOWER_LIP_DEPRESSOR_L] = weights[XR_FACE_EXPRESSION_LOWER_LIP_DEPRESSOR_L_FB];
		ret[EXPRESSION_LOWER_LIP_DEPRESSOR_R] = weights[XR_FACE_EXPRESSION_LOWER_LIP_DEPRESSOR_R_FB];
		ret[EXPRESSION_MOUTH_LEFT] = weights[XR_FACE_EXPRESSION_MOUTH_LEFT_FB];
		ret[EXPRESSION_MOUTH_RIGHT] = weights[XR_FACE_EXPRESSION_MOUTH_RIGHT_FB];
		ret[EXPRESSION_NOSE_WRINKLER_L] = weights[XR_FACE_EXPRESSION_NOSE_WRINKLER_L_FB];
		ret[EXPRESSION_NOSE_WRINKLER_R] = weights[XR_FACE_EXPRESSION_NOSE_WRINKLER_R_FB];
		ret[EXPRESSION_OUTER_BROW_RAISER_L] = weights[XR_FACE_EXPRESSION_OUTER_BROW_RAISER_L_FB];
		ret[EXPRESSION_OUTER_BROW_RAISER_R] = weights[XR_FACE_EXPRESSION_OUTER_BROW_RAISER_R_FB];
		ret[EXPRESSION_UPPER_LID_RAISER_L] = weights[XR_FACE_EXPRESSION_UPPER_LID_RAISER_L_FB];
		ret[EXPRESSION_UPPER_LID_RAISER_R] = weights[XR_FACE_EXPRESSION_UPPER_LID_RAISER_R_FB];
		ret[EXPRESSION_UPPER_LIP_RAISER_L] = weights[XR_FACE_EXPRESSION_UPPER_LIP_RAISER_L_FB];
		ret[EXPRESSION_UPPER_LIP_RAISER_R] = weights[XR_FACE_EXPRESSION_UPPER_LIP_RAISER_R_FB];
	}

	return ret;
}

PackedFloat32Array OpenXRFbFaceTrackingExtensionWrapper::get_confidences() const {
	PackedFloat32Array ret;

	// If valid, populate the array with confidences
	if (face_expression_weights.status.isValid) {
		// This might be done faster with a memcpy, but this approach ensures
		// the size and order of the Godot array matches the Godot enum and
		// should be immune from changes or additional confidences in the OpenXR
		// data.
		ret.resize(CONFIDENCE_COUNT);
		ret[CONFIDENCE_LOWER_FACE] = confidences[XR_FACE_CONFIDENCE_LOWER_FACE_FB];
		ret[CONFIDENCE_UPPER_FACE] = confidences[XR_FACE_CONFIDENCE_UPPER_FACE_FB];
	}

	return ret;
}

bool OpenXRFbFaceTrackingExtensionWrapper::initialize_fb_face_tracking_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateFaceTrackerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyFaceTrackerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetFaceExpressionWeightsFB);

	return true;
}