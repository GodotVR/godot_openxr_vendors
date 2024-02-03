/**************************************************************************/
/*  openxr_fb_face_tracking_extension_wrapper.h                           */
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

#ifndef OPENXR_FB_FACE_TRACKING_EXTENSION_WRAPPER_H
#define OPENXR_FB_FACE_TRACKING_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook face tracking extension.
class OpenXRFbFaceTrackingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbFaceTrackingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	// Face expressions
	enum Expression {
		EXPRESSION_BROW_LOWERER_L = 0,
		EXPRESSION_BROW_LOWERER_R = 1,
		EXPRESSION_CHEEK_PUFF_L = 2,
		EXPRESSION_CHEEK_PUFF_R = 3,
		EXPRESSION_CHEEK_RAISER_L = 4,
		EXPRESSION_CHEEK_RAISER_R = 5,
		EXPRESSION_CHEEK_SUCK_L = 6,
		EXPRESSION_CHEEK_SUCK_R = 7,
		EXPRESSION_CHIN_RAISER_B = 8,
		EXPRESSION_CHIN_RAISER_T = 9,
		EXPRESSION_DIMPLER_L = 10,
		EXPRESSION_DIMPLER_R = 11,
		EXPRESSION_EYES_CLOSED_L = 12,
		EXPRESSION_EYES_CLOSED_R = 13,
		EXPRESSION_EYES_LOOK_DOWN_L = 14,
		EXPRESSION_EYES_LOOK_DOWN_R = 15,
		EXPRESSION_EYES_LOOK_LEFT_L = 16,
		EXPRESSION_EYES_LOOK_LEFT_R = 17,
		EXPRESSION_EYES_LOOK_RIGHT_L = 18,
		EXPRESSION_EYES_LOOK_RIGHT_R = 19,
		EXPRESSION_EYES_LOOK_UP_L = 20,
		EXPRESSION_EYES_LOOK_UP_R = 21,
		EXPRESSION_INNER_BROW_RAISER_L = 22,
		EXPRESSION_INNER_BROW_RAISER_R = 23,
		EXPRESSION_JAW_DROP = 24,
		EXPRESSION_JAW_SIDEWAYS_LEFT = 25,
		EXPRESSION_JAW_SIDEWAYS_RIGHT = 26,
		EXPRESSION_JAW_THRUST = 27,
		EXPRESSION_LID_TIGHTENER_L = 28,
		EXPRESSION_LID_TIGHTENER_R = 29,
		EXPRESSION_LIP_CORNER_DEPRESSOR_L = 30,
		EXPRESSION_LIP_CORNER_DEPRESSOR_R = 31,
		EXPRESSION_LIP_CORNER_PULLER_L = 32,
		EXPRESSION_LIP_CORNER_PULLER_R = 33,
		EXPRESSION_LIP_FUNNELER_LB = 34,
		EXPRESSION_LIP_FUNNELER_LT = 35,
		EXPRESSION_LIP_FUNNELER_RB = 36,
		EXPRESSION_LIP_FUNNELER_RT = 37,
		EXPRESSION_LIP_PRESSOR_L = 38,
		EXPRESSION_LIP_PRESSOR_R = 39,
		EXPRESSION_LIP_PUCKER_L = 40,
		EXPRESSION_LIP_PUCKER_R = 41,
		EXPRESSION_LIP_STRETCHER_L = 42,
		EXPRESSION_LIP_STRETCHER_R = 43,
		EXPRESSION_LIP_SUCK_LB = 44,
		EXPRESSION_LIP_SUCK_LT = 45,
		EXPRESSION_LIP_SUCK_RB = 46,
		EXPRESSION_LIP_SUCK_RT = 47,
		EXPRESSION_LIP_TIGHTENER_L = 48,
		EXPRESSION_LIP_TIGHTENER_R = 49,
		EXPRESSION_LIPS_TOWARD = 50,
		EXPRESSION_LOWER_LIP_DEPRESSOR_L = 51,
		EXPRESSION_LOWER_LIP_DEPRESSOR_R = 52,
		EXPRESSION_MOUTH_LEFT = 53,
		EXPRESSION_MOUTH_RIGHT = 54,
		EXPRESSION_NOSE_WRINKLER_L = 55,
		EXPRESSION_NOSE_WRINKLER_R = 56,
		EXPRESSION_OUTER_BROW_RAISER_L = 57,
		EXPRESSION_OUTER_BROW_RAISER_R = 58,
		EXPRESSION_UPPER_LID_RAISER_L = 59,
		EXPRESSION_UPPER_LID_RAISER_R = 60,
		EXPRESSION_UPPER_LIP_RAISER_L = 61,
		EXPRESSION_UPPER_LIP_RAISER_R = 62,
		EXPRESSION_COUNT = 63
	};

	// Face Confidences
	enum Confidence {
		CONFIDENCE_LOWER_FACE = 0,
		CONFIDENCE_UPPER_FACE = 1,
		CONFIDENCE_COUNT = 2
	};

	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_session_created(uint64_t instance) override;

	void _on_session_destroyed() override;

	void _on_process() override;

	static OpenXRFbFaceTrackingExtensionWrapper *get_singleton();

	bool is_enabled() const;

	PackedFloat32Array get_weights() const;

	PackedFloat32Array get_confidences() const;

	OpenXRFbFaceTrackingExtensionWrapper();
	~OpenXRFbFaceTrackingExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateFaceTrackerFB,
			(XrSession), session,
			(const XrFaceTrackerCreateInfoFB *), createInfo,
			(XrFaceTrackerFB *), faceTracker);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyFaceTrackerFB,
			(XrFaceTrackerFB), faceTracker);

	EXT_PROTO_XRRESULT_FUNC3(xrGetFaceExpressionWeightsFB,
			(XrFaceTrackerFB), faceTracker,
			(const XrFaceExpressionInfoFB *), expressionInfo,
			(XrFaceExpressionWeightsFB *), expressionWeights);

	bool initialize_fb_face_tracking_extension(const XrInstance instance);

	void cleanup();

	static OpenXRFbFaceTrackingExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool fb_face_tracking_ext = false;
	XrSystemFaceTrackingPropertiesFB system_face_tracking_properties;
	XrFaceTrackerFB face_tracker = XR_NULL_HANDLE;
	float weights[XR_FACE_EXPRESSION_COUNT_FB] = {};
	float confidences[XR_FACE_CONFIDENCE_COUNT_FB] = {};
	XrFaceExpressionWeightsFB face_expression_weights = {};
};

VARIANT_ENUM_CAST(OpenXRFbFaceTrackingExtensionWrapper::Expression);
VARIANT_ENUM_CAST(OpenXRFbFaceTrackingExtensionWrapper::Confidence);

#endif // OPENXR_FB_FACE_TRACKING_EXTENSION_WRAPPER_H
