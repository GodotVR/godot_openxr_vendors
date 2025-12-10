/**************************************************************************/
/*  openxr_fb_body_tracking_extension_wrapper.cpp                         */
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

#include "extensions/openxr_fb_body_tracking_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/templates/local_vector.hpp>

using namespace godot;

/// Joint mapping table entry
struct JointMapEntry {
	/// Joint in Godot XRBodyTracker
	XRBodyTracker::Joint xr_joint;

	/// Joint in OpenXR XrBodyJointFB or XrFullBodyJointMETA
	int fb_joint;

	/// Joint rotation
	Quaternion rotation;
};

/// Joint mapping table
static const JointMapEntry joint_table[] = {
	// Root joint
	{ XRBodyTracker::JOINT_ROOT, XR_BODY_JOINT_ROOT_FB, Quaternion(0.0, 0.0, 0.0, 1.0) },

	// Upper body joints
	{ XRBodyTracker::JOINT_HIPS, XR_BODY_JOINT_HIPS_FB, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_SPINE, XR_BODY_JOINT_SPINE_LOWER_FB, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_CHEST, XR_BODY_JOINT_SPINE_UPPER_FB, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_UPPER_CHEST, XR_BODY_JOINT_CHEST_FB, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_NECK, XR_BODY_JOINT_NECK_FB, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_HEAD, XR_BODY_JOINT_HEAD_FB, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_LEFT_SHOULDER, XR_BODY_JOINT_LEFT_SHOULDER_FB, Quaternion(0.0, 0.0, 0.7071067811865475244, 0.7071067811865475244) },
	{ XRBodyTracker::JOINT_LEFT_UPPER_ARM, XR_BODY_JOINT_LEFT_ARM_UPPER_FB, Quaternion(-0.7071067811865475244, 0.7071067811865475244, 0.0, 0.0) },
	{ XRBodyTracker::JOINT_LEFT_LOWER_ARM, XR_BODY_JOINT_LEFT_ARM_LOWER_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_RIGHT_SHOULDER, XR_BODY_JOINT_RIGHT_SHOULDER_FB, Quaternion(0.7071067811865475244, 0.7071067811865475244, 0.0, 0.0) },
	{ XRBodyTracker::JOINT_RIGHT_UPPER_ARM, XR_BODY_JOINT_RIGHT_ARM_UPPER_FB, Quaternion(0.0, 0.0, -0.7071067811865475244, 0.7071067811865475244) },
	{ XRBodyTracker::JOINT_RIGHT_LOWER_ARM, XR_BODY_JOINT_RIGHT_ARM_LOWER_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },

	// Left hand joints
	{ XRBodyTracker::JOINT_LEFT_HAND, XR_BODY_JOINT_LEFT_HAND_WRIST_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_PALM, XR_BODY_JOINT_LEFT_HAND_PALM_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_WRIST, XR_BODY_JOINT_LEFT_HAND_WRIST_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_THUMB_METACARPAL, XR_BODY_JOINT_LEFT_HAND_THUMB_METACARPAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_THUMB_PHALANX_PROXIMAL, XR_BODY_JOINT_LEFT_HAND_THUMB_PROXIMAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_THUMB_PHALANX_DISTAL, XR_BODY_JOINT_LEFT_HAND_THUMB_DISTAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_THUMB_TIP, XR_BODY_JOINT_LEFT_HAND_THUMB_TIP_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_INDEX_FINGER_METACARPAL, XR_BODY_JOINT_LEFT_HAND_INDEX_METACARPAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_INDEX_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_LEFT_HAND_INDEX_PROXIMAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_INDEX_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_LEFT_HAND_INDEX_INTERMEDIATE_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_INDEX_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_LEFT_HAND_INDEX_DISTAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_INDEX_FINGER_TIP, XR_BODY_JOINT_LEFT_HAND_INDEX_TIP_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_MIDDLE_FINGER_METACARPAL, XR_BODY_JOINT_LEFT_HAND_MIDDLE_METACARPAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_MIDDLE_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_LEFT_HAND_MIDDLE_PROXIMAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_MIDDLE_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_LEFT_HAND_MIDDLE_INTERMEDIATE_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_MIDDLE_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_LEFT_HAND_MIDDLE_DISTAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_MIDDLE_FINGER_TIP, XR_BODY_JOINT_LEFT_HAND_MIDDLE_TIP_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_RING_FINGER_METACARPAL, XR_BODY_JOINT_LEFT_HAND_RING_METACARPAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_RING_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_LEFT_HAND_RING_PROXIMAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_RING_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_LEFT_HAND_RING_INTERMEDIATE_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_RING_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_LEFT_HAND_RING_DISTAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_RING_FINGER_TIP, XR_BODY_JOINT_LEFT_HAND_RING_TIP_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_PINKY_FINGER_METACARPAL, XR_BODY_JOINT_LEFT_HAND_LITTLE_METACARPAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_PINKY_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_LEFT_HAND_LITTLE_PROXIMAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_PINKY_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_LEFT_HAND_LITTLE_INTERMEDIATE_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_PINKY_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_LEFT_HAND_LITTLE_DISTAL_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_PINKY_FINGER_TIP, XR_BODY_JOINT_LEFT_HAND_LITTLE_TIP_FB, Quaternion(0.5, -0.5, -0.5, -0.5) },

	// Right hand joints
	{ XRBodyTracker::JOINT_RIGHT_HAND, XR_BODY_JOINT_RIGHT_HAND_WRIST_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_PALM, XR_BODY_JOINT_RIGHT_HAND_PALM_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_WRIST, XR_BODY_JOINT_RIGHT_HAND_WRIST_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_THUMB_METACARPAL, XR_BODY_JOINT_RIGHT_HAND_THUMB_METACARPAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_THUMB_PHALANX_PROXIMAL, XR_BODY_JOINT_RIGHT_HAND_THUMB_PROXIMAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_THUMB_PHALANX_DISTAL, XR_BODY_JOINT_RIGHT_HAND_THUMB_DISTAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_THUMB_TIP, XR_BODY_JOINT_RIGHT_HAND_THUMB_TIP_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_INDEX_FINGER_METACARPAL, XR_BODY_JOINT_RIGHT_HAND_INDEX_METACARPAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_INDEX_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_RIGHT_HAND_INDEX_PROXIMAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_INDEX_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_RIGHT_HAND_INDEX_INTERMEDIATE_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_INDEX_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_RIGHT_HAND_INDEX_DISTAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_INDEX_FINGER_TIP, XR_BODY_JOINT_RIGHT_HAND_INDEX_TIP_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_MIDDLE_FINGER_METACARPAL, XR_BODY_JOINT_RIGHT_HAND_MIDDLE_METACARPAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_MIDDLE_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_RIGHT_HAND_MIDDLE_PROXIMAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_MIDDLE_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_RIGHT_HAND_MIDDLE_INTERMEDIATE_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_MIDDLE_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_RIGHT_HAND_MIDDLE_DISTAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_MIDDLE_FINGER_TIP, XR_BODY_JOINT_RIGHT_HAND_MIDDLE_TIP_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_RING_FINGER_METACARPAL, XR_BODY_JOINT_RIGHT_HAND_RING_METACARPAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_RING_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_RIGHT_HAND_RING_PROXIMAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_RING_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_RIGHT_HAND_RING_INTERMEDIATE_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_RING_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_RIGHT_HAND_RING_DISTAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_RING_FINGER_TIP, XR_BODY_JOINT_RIGHT_HAND_RING_TIP_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_PINKY_FINGER_METACARPAL, XR_BODY_JOINT_RIGHT_HAND_LITTLE_METACARPAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_PINKY_FINGER_PHALANX_PROXIMAL, XR_BODY_JOINT_RIGHT_HAND_LITTLE_PROXIMAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_PINKY_FINGER_PHALANX_INTERMEDIATE, XR_BODY_JOINT_RIGHT_HAND_LITTLE_INTERMEDIATE_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_PINKY_FINGER_PHALANX_DISTAL, XR_BODY_JOINT_RIGHT_HAND_LITTLE_DISTAL_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_PINKY_FINGER_TIP, XR_BODY_JOINT_RIGHT_HAND_LITTLE_TIP_FB, Quaternion(0.5, 0.5, -0.5, 0.5) },

	// Lower body joints
	{ XRBodyTracker::JOINT_LEFT_UPPER_LEG, XR_FULL_BODY_JOINT_LEFT_UPPER_LEG_META, Quaternion(0.5, -0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_LEFT_LOWER_LEG, XR_FULL_BODY_JOINT_LEFT_LOWER_LEG_META, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_LEFT_FOOT, XR_FULL_BODY_JOINT_LEFT_FOOT_ANKLE_META, Quaternion(-0.5, -0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_LEFT_TOES, XR_FULL_BODY_JOINT_LEFT_FOOT_BALL_META, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_UPPER_LEG, XR_FULL_BODY_JOINT_RIGHT_UPPER_LEG_META, Quaternion(-0.5, -0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_LOWER_LEG, XR_FULL_BODY_JOINT_RIGHT_LOWER_LEG_META, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_FOOT, XR_FULL_BODY_JOINT_RIGHT_FOOT_ANKLE_META, Quaternion(0.5, -0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_TOES, XR_FULL_BODY_JOINT_RIGHT_FOOT_BALL_META, Quaternion(-0.5, 0.5, 0.5, 0.5) },
};

OpenXRFbBodyTrackingExtensionWrapper *OpenXRFbBodyTrackingExtensionWrapper::singleton = nullptr;

OpenXRFbBodyTrackingExtensionWrapper *OpenXRFbBodyTrackingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbBodyTrackingExtensionWrapper());
	}
	return singleton;
}

OpenXRFbBodyTrackingExtensionWrapper::OpenXRFbBodyTrackingExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbBodyTrackingExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_BODY_TRACKING_EXTENSION_NAME] = &fb_body_tracking_ext;
	request_extensions[XR_META_BODY_TRACKING_FULL_BODY_EXTENSION_NAME] = &meta_body_tracking_full_body_ext;

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	request_extensions[XR_META_BODY_TRACKING_FIDELITY_EXTENSION_NAME] = &meta_body_tracking_fidelity_ext;
	request_extensions[XR_META_BODY_TRACKING_CALIBRATION_EXTENSION_NAME] = &meta_body_tracking_calibration_ext;
#endif // META_HEADERS_ENABLED

	singleton = this;
}

OpenXRFbBodyTrackingExtensionWrapper::~OpenXRFbBodyTrackingExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

void OpenXRFbBodyTrackingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_full_body_tracking_supported"), &OpenXRFbBodyTrackingExtensionWrapper::is_full_body_tracking_supported);

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	ClassDB::bind_method(D_METHOD("is_body_tracking_fidelity_supported"), &OpenXRFbBodyTrackingExtensionWrapper::is_body_tracking_fidelity_supported);
	ClassDB::bind_method(D_METHOD("request_body_tracking_fidelity", "fidelity"), &OpenXRFbBodyTrackingExtensionWrapper::request_body_tracking_fidelity);
	ClassDB::bind_method(D_METHOD("get_body_tracking_fidelity_status"), &OpenXRFbBodyTrackingExtensionWrapper::get_body_tracking_fidelity_status);

	ClassDB::bind_method(D_METHOD("is_body_tracking_height_override_supported"), &OpenXRFbBodyTrackingExtensionWrapper::is_body_tracking_height_override_supported);
	ClassDB::bind_method(D_METHOD("suggest_body_tracking_height_override", "body_height"), &OpenXRFbBodyTrackingExtensionWrapper::suggest_body_tracking_height_override);
	ClassDB::bind_method(D_METHOD("get_body_tracking_calibration_state"), &OpenXRFbBodyTrackingExtensionWrapper::get_body_tracking_calibration_state);
	ClassDB::bind_method(D_METHOD("reset_body_tracking_calibration"), &OpenXRFbBodyTrackingExtensionWrapper::reset_body_tracking_calibration);

	BIND_ENUM_CONSTANT(BODY_TRACKING_FIDELITY_UNKNOWN);
	BIND_ENUM_CONSTANT(BODY_TRACKING_FIDELITY_LOW);
	BIND_ENUM_CONSTANT(BODY_TRACKING_FIDELITY_HIGH);

	BIND_ENUM_CONSTANT(BODY_TRACKING_CALIBRATION_STATE_VALID);
	BIND_ENUM_CONSTANT(BODY_TRACKING_CALIBRATION_STATE_CALIBRATING);
	BIND_ENUM_CONSTANT(BODY_TRACKING_CALIBRATION_STATE_INVALID);
#endif // META_HEADERS_ENABLED
}

void OpenXRFbBodyTrackingExtensionWrapper::cleanup() {
	fb_body_tracking_ext = false;
	meta_body_tracking_full_body_ext = false;

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	meta_body_tracking_fidelity_ext = false;
	meta_body_tracking_calibration_ext = false;
#endif // META_HEADERS_ENABLED
}

uint64_t OpenXRFbBodyTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (fb_body_tracking_ext) {
		system_body_tracking_properties.next = p_next_pointer;
		p_next_pointer = &system_body_tracking_properties;
	}
	if (meta_body_tracking_full_body_ext) {
		system_body_tracking_full_body_properties.next = p_next_pointer;
		p_next_pointer = &system_body_tracking_full_body_properties;
	}

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	if (meta_body_tracking_fidelity_ext) {
		system_body_tracking_fidelity_properties.next = p_next_pointer;
		p_next_pointer = &system_body_tracking_fidelity_properties;
	}
	if (meta_body_tracking_calibration_ext) {
		system_body_tracking_calibration_properties.next = p_next_pointer;
		p_next_pointer = &system_body_tracking_calibration_properties;
	}
#endif // META_HEADERS_ENABLED

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

godot::Dictionary OpenXRFbBodyTrackingExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbBodyTrackingExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (fb_body_tracking_ext) {
		bool result = initialize_fb_body_tracking_extension((XrInstance)p_instance);
		if (!result) {
			ERR_PRINT("Failed to initialize fb_body_tracking extension");
			fb_body_tracking_ext = false;
		}
	}

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	if (meta_body_tracking_fidelity_ext) {
		bool result = initialize_meta_body_tracking_fidelity_extension((XrInstance)p_instance);
		if (!result) {
			ERR_PRINT("Failed to initialize meta_body_tracking_fidelity extension");
			meta_body_tracking_fidelity_ext = false;
		}
	}

	if (meta_body_tracking_calibration_ext) {
		bool result = initialize_meta_body_tracking_calibration_extension((XrInstance)p_instance);
		if (!result) {
			ERR_PRINT("Failed to initialize meta_body_tracking_calibration extension");
			meta_body_tracking_calibration_ext = false;
		}
	}
#endif // META_HEADERS_ENABLED
}

void OpenXRFbBodyTrackingExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbBodyTrackingExtensionWrapper::_on_session_created(uint64_t instance) {
	// Skip if not enabled
	if (!is_enabled()) {
		return;
	}

	// Create the body-tracker handle
	XrBodyJointSetFB body_joint_set = XR_BODY_JOINT_SET_DEFAULT_FB;
	if (meta_body_tracking_full_body_ext && is_full_body_tracking_supported()) {
		body_joint_set = XR_BODY_JOINT_SET_FULL_BODY_META;
	}

	XrBodyTrackerCreateInfoFB createInfo = {
		XR_TYPE_BODY_TRACKER_CREATE_INFO_FB, // type
		nullptr, // next
		body_joint_set, // bodyJointSet
	};
	XrResult result = xrCreateBodyTrackerFB(SESSION, &createInfo, &body_tracker);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to create body-tracker handle: ", get_openxr_api()->get_error_string(result)));

	// Construct the XRBodyTracker if necessary
	if (xr_body_tracker.is_null()) {
		xr_body_tracker.instantiate();
		xr_body_tracker->set_tracker_name("/user/body_tracker");

		BitField<XRBodyTracker::BodyFlags> body_flags = XRBodyTracker::BODY_FLAG_UPPER_BODY_SUPPORTED | XRBodyTracker::BODY_FLAG_HANDS_SUPPORTED;
		if (meta_body_tracking_full_body_ext && is_full_body_tracking_supported()) {
			body_flags.set_flag(XRBodyTracker::BODY_FLAG_LOWER_BODY_SUPPORTED);
		}
		xr_body_tracker->set_body_flags(body_flags);
	}
}

void OpenXRFbBodyTrackingExtensionWrapper::_on_session_destroyed() {
	// Skip if no body-tracker handle
	if (!body_tracker) {
		return;
	}

	// Destroy the body-tracker handle
	XrResult result = xrDestroyBodyTrackerFB(body_tracker);
	if (XR_FAILED(result)) {
		ERR_PRINT(vformat("Failed to destroy body-tracker handle: ", get_openxr_api()->get_error_string(result)));
	}
	body_tracker = XR_NULL_HANDLE;

	// Unregister the body tracker.
	if (xr_body_tracker_registered) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server && xr_body_tracker.is_valid()) {
			xr_server->remove_tracker(xr_body_tracker);
		}
	}
	xr_body_tracker_registered = false;
}

void OpenXRFbBodyTrackingExtensionWrapper::_on_process() {
	// Skip if not enabled, or no body-tracker handle
	if (!is_enabled() || !body_tracker) {
		return;
	}

	// Get the next frame time
	const XrTime display_time = get_openxr_api()->get_predicted_display_time();
	if (display_time == 0) {
		return;
	}

	// Construct the expression info struct.
	XrBodyJointsLocateInfoFB locate_info = {
		XR_TYPE_BODY_JOINTS_LOCATE_INFO_FB, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
		display_time // time
	};

	// Construct locations struct next chain.
	void *next_pointer = nullptr;
// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	if (meta_body_tracking_fidelity_ext && is_body_tracking_fidelity_supported()) {
		body_tracking_fidelity_status.next = next_pointer;
		next_pointer = &body_tracking_fidelity_status;
	}

	if (meta_body_tracking_calibration_ext) {
		body_tracking_calibration_status.next = next_pointer;
		next_pointer = &body_tracking_calibration_status;
	}
#endif // META_HEADERS_ENABLED

	// Construct the locations struct.
	uint32_t fb_joint_count = XR_BODY_JOINT_COUNT_FB;
	bool is_full_body_supported = is_full_body_tracking_supported();
	if (meta_body_tracking_full_body_ext && is_full_body_supported) {
		fb_joint_count = XR_FULL_BODY_JOINT_COUNT_META;
	}

	XrBodyJointLocationFB fb_locations[XR_FULL_BODY_JOINT_COUNT_META];
	XrBodyJointLocationsFB locations = {
		XR_TYPE_BODY_JOINT_LOCATIONS_FB, // type
		next_pointer, // next
		XR_FALSE, // isActive
		0.0f, // confidence
		fb_joint_count, // jointCount
		fb_locations // jointLocations
	};

	// Read the weights
	XrResult result = xrLocateBodyJointsFB(body_tracker, &locate_info, &locations);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to get body joint locations: ", get_openxr_api()->get_error_string(result)));

	// Set the tracking active flag
	xr_body_tracker->set_has_tracking_data(locations.isActive);

	// Process all joints
	for (const JointMapEntry &entry : joint_table) {
		// Skip full body joints if extension is not supported.
		if (!is_full_body_supported && entry.fb_joint >= XR_BODY_JOINT_COUNT_FB) {
			break;
		}

		// Process the joint pose
		const XrBodyJointLocationFB &location = fb_locations[entry.fb_joint];
		const XrPosef &pose = location.pose;
		Transform3D transform;
		BitField<XRBodyTracker::JointFlags> flags(0);

		// Analyze the available joint data
		if (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) {
			flags.set_flag(XRBodyTracker::JOINT_FLAG_ORIENTATION_VALID);
			transform.basis = Basis(Quaternion(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w) * entry.rotation);
		}
		if (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT) {
			flags.set_flag(XRBodyTracker::JOINT_FLAG_ORIENTATION_TRACKED);
		}
		if (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
			flags.set_flag(XRBodyTracker::JOINT_FLAG_POSITION_VALID);
			transform.origin = Vector3(pose.position.x, pose.position.y, pose.position.z);
		}
		if (location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) {
			flags.set_flag(XRBodyTracker::JOINT_FLAG_POSITION_TRACKED);
		}

		// Set the joint information
		xr_body_tracker->set_joint_flags(entry.xr_joint, flags);
		xr_body_tracker->set_joint_transform(entry.xr_joint, transform);
	}

	// If the location data is good then we need to apply some corrections
	// before handing the data back to Godot. These include:
	//
	// - The Root joint carries no useful orientation data, so instead align it
	//   under the hips pointing forwards.
	//
	// - Adjusting the left and right shoulder back so they are aligned
	//   with how models are designed, rather than the Meta positions of
	//   the tips of the clavicles.
	if (locations.isActive) {
		// Align root under the hips pointing 'forwards'
		// IE, +z aligns with hips & user's real-world [upper-body] forward.
		// (Remaining, however, parallel to the XRorigin / Global XZ plane; root's basis rotated around Y to fit)

		// Get the hips transform
		Transform3D hips = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_HIPS);
		Vector3 root_y = Vector3(0.0, 1.0, 0.0);
		Vector3 hips_left = hips.basis.get_column(Vector3::AXIS_X);
		Vector3 root_x = (hips_left.slide(Vector3(0.0, 1.0, 0.0))).normalized();
		Vector3 root_z = root_x.cross(root_y);
		Vector3 root_o = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_ROOT).origin;
		Transform3D root = Transform3D(root_x, root_y, root_z, root_o).orthonormalized();
		xr_body_tracker->set_joint_transform(XRBodyTracker::JOINT_ROOT, root);
		// Set tracker pose, velocities, confidence.
		xr_body_tracker->set_pose("default", root, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);

		// Distance in meters to push the shoulder joints back from the
		// clavicle-position to be in-line with the upper arm joints as
		// required for T-Pose designed models.
		constexpr float shoulder_z_offset = -0.07;

		// Deduce the shoulder offset from the upper chest transform
		Transform3D upper_chest = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_UPPER_CHEST);
		Vector3 shoulder_offset = upper_chest.basis.get_column(Vector3::AXIS_Z) * shoulder_z_offset;

		// Correct the left shoulder
		Transform3D left_shoulder = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_LEFT_SHOULDER);
		left_shoulder.origin += shoulder_offset;
		xr_body_tracker->set_joint_transform(XRBodyTracker::JOINT_LEFT_SHOULDER, left_shoulder);

		// Correct the right shoulder
		Transform3D right_shoulder = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_RIGHT_SHOULDER);
		right_shoulder.origin += shoulder_offset;
		xr_body_tracker->set_joint_transform(XRBodyTracker::JOINT_RIGHT_SHOULDER, right_shoulder);
	}

	// Register the XRBodyTracker if necessary
	if (!xr_body_tracker_registered) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server) {
			xr_server->add_tracker(xr_body_tracker);
			xr_body_tracker_registered = true;
		}
	}
}

bool OpenXRFbBodyTrackingExtensionWrapper::is_enabled() const {
	return fb_body_tracking_ext && system_body_tracking_properties.supportsBodyTracking;
}

bool OpenXRFbBodyTrackingExtensionWrapper::initialize_fb_body_tracking_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateBodyTrackerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyBodyTrackerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateBodyJointsFB);

	return true;
}

// META_body_tracking_full_body extension.

bool OpenXRFbBodyTrackingExtensionWrapper::is_full_body_tracking_supported() {
	return system_body_tracking_full_body_properties.supportsFullBodyTracking;
}

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
// META_body_tracking_fidelity extension.

bool OpenXRFbBodyTrackingExtensionWrapper::is_body_tracking_fidelity_supported() {
	return system_body_tracking_fidelity_properties.supportsBodyTrackingFidelity;
}

void OpenXRFbBodyTrackingExtensionWrapper::request_body_tracking_fidelity(BodyTrackingFidelity p_fidelity) {
	ERR_FAIL_COND_MSG(!fb_body_tracking_ext || !meta_body_tracking_fidelity_ext, "XR_META_body_tracking_fidelity extension is not enabled");
	ERR_FAIL_COND_MSG(p_fidelity == BODY_TRACKING_FIDELITY_UNKNOWN, "Cannot request body tracking fidelity update: invalid fidelity type");
	ERR_FAIL_COND_MSG(body_tracker == XR_NULL_HANDLE, "Cannot request body tracking fidelity update: body tracker handle is null");

	const XrBodyTrackingFidelityMETA fidelity = XrBodyTrackingFidelityMETA(p_fidelity);
	XrResult result = xrRequestBodyTrackingFidelityMETA(body_tracker, fidelity);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to request body tracking fidelity update: ", get_openxr_api()->get_error_string(result)));
}

OpenXRFbBodyTrackingExtensionWrapper::BodyTrackingFidelity OpenXRFbBodyTrackingExtensionWrapper::get_body_tracking_fidelity_status() {
	ERR_FAIL_COND_V_MSG(!fb_body_tracking_ext || !meta_body_tracking_fidelity_ext, BODY_TRACKING_FIDELITY_UNKNOWN, "XR_META_body_tracking_fidelity extension is not enabled");

	switch (body_tracking_fidelity_status.fidelity) {
		case XR_BODY_TRACKING_FIDELITY_LOW_META:
			return BODY_TRACKING_FIDELITY_LOW;
		case XR_BODY_TRACKING_FIDELITY_HIGH_META:
			return BODY_TRACKING_FIDELITY_HIGH;
		default:
			return BODY_TRACKING_FIDELITY_UNKNOWN;
	}
}

bool OpenXRFbBodyTrackingExtensionWrapper::initialize_meta_body_tracking_fidelity_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrRequestBodyTrackingFidelityMETA);

	return true;
}

// META_body_tracking_calibration extension.

bool OpenXRFbBodyTrackingExtensionWrapper::is_body_tracking_height_override_supported() {
	return system_body_tracking_calibration_properties.supportsHeightOverride;
}

OpenXRFbBodyTrackingExtensionWrapper::BodyTrackingCalibrationState OpenXRFbBodyTrackingExtensionWrapper::get_body_tracking_calibration_state() {
	ERR_FAIL_COND_V_MSG(!fb_body_tracking_ext || !meta_body_tracking_calibration_ext, BODY_TRACKING_CALIBRATION_STATE_INVALID, "XR_META_body_tracking_calibration extension is not enabled");

	switch (body_tracking_calibration_status.status) {
		case XR_BODY_TRACKING_CALIBRATION_STATE_VALID_META:
			return BODY_TRACKING_CALIBRATION_STATE_VALID;
		case XR_BODY_TRACKING_CALIBRATION_STATE_CALIBRATING_META:
			return BODY_TRACKING_CALIBRATION_STATE_CALIBRATING;
		case XR_BODY_TRACKING_CALIBRATION_STATE_INVALID_META:
			return BODY_TRACKING_CALIBRATION_STATE_INVALID;
		default:
			return BODY_TRACKING_CALIBRATION_STATE_INVALID;
	}
}

void OpenXRFbBodyTrackingExtensionWrapper::suggest_body_tracking_height_override(float p_body_height) {
	ERR_FAIL_COND_MSG(!fb_body_tracking_ext || !meta_body_tracking_calibration_ext, "XR_META_body_tracking_calibration extension is not enabled");
	ERR_FAIL_COND_MSG(p_body_height < 0.5f || p_body_height > 3.0f, "Cannot request body tracking height override: height must be within range of 0.5 and 3.0 meters");
	ERR_FAIL_COND_MSG(body_tracker == XR_NULL_HANDLE, "Cannot request body tracking height override: body tracker handle is null");

	const XrBodyTrackingCalibrationInfoMETA body_tracking_calibration_info = {
		XR_TYPE_BODY_TRACKING_CALIBRATION_INFO_META, // type
		nullptr, // next
		p_body_height, // bodyHeight
	};

	XrResult result = xrSuggestBodyTrackingCalibrationOverrideMETA(body_tracker, &body_tracking_calibration_info);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to suggest body tracking calibration override: ", get_openxr_api()->get_error_string(result)));
}

void OpenXRFbBodyTrackingExtensionWrapper::reset_body_tracking_calibration() {
	ERR_FAIL_COND_MSG(!fb_body_tracking_ext || !meta_body_tracking_calibration_ext, "XR_META_body_tracking_calibration extension is not enabled");
	ERR_FAIL_COND_MSG(body_tracker == XR_NULL_HANDLE, "Cannot reset body tracking calibration: body tracker handle is null");

	XrResult result = xrResetBodyTrackingCalibrationMETA(body_tracker);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to reset body tracking calibration: ", get_openxr_api()->get_error_string(result)));
}

bool OpenXRFbBodyTrackingExtensionWrapper::initialize_meta_body_tracking_calibration_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrSuggestBodyTrackingCalibrationOverrideMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrResetBodyTrackingCalibrationMETA);

	return true;
}
#endif // META_HEADERS_ENABLED
