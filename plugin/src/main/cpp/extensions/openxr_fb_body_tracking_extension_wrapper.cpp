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
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

/// Joint mapping table entry
struct JointMapEntry {
	/// Joint in Godot XRBodyTracker
	XRBodyTracker::Joint xr_joint;

	/// Joint in OpenXR XrBodyJointFB
	XrBodyJointFB fb_joint;

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

	singleton = this;
}

OpenXRFbBodyTrackingExtensionWrapper::~OpenXRFbBodyTrackingExtensionWrapper() {
	cleanup();
}

void OpenXRFbBodyTrackingExtensionWrapper::_bind_methods() {
}

void OpenXRFbBodyTrackingExtensionWrapper::cleanup() {
	fb_body_tracking_ext = false;
}

uint64_t OpenXRFbBodyTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *next_pointer) {
	system_body_tracking_properties.type = XR_TYPE_SYSTEM_BODY_TRACKING_PROPERTIES_FB;
	system_body_tracking_properties.next = next_pointer;
	system_body_tracking_properties.supportsBodyTracking = false;
	return reinterpret_cast<uint64_t>(&system_body_tracking_properties);
}

godot::Dictionary OpenXRFbBodyTrackingExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbBodyTrackingExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_body_tracking_ext) {
		bool result = initialize_fb_body_tracking_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_body_tracking extension");
			fb_body_tracking_ext = false;
		}
	}
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
	XrBodyTrackerCreateInfoFB createInfo = {
		XR_TYPE_BODY_TRACKER_CREATE_INFO_FB, // type
		nullptr, // next
		XR_BODY_JOINT_SET_DEFAULT_FB // bodyJointSet
	};
	XrResult result = xrCreateBodyTrackerFB(SESSION, &createInfo, &body_tracker);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create body-tracker handle: ", result);
		return;
	}

	// Construct the XRBodyTracker if necessary
	if (xr_body_tracker.is_null()) {
		xr_body_tracker.instantiate();
		xr_body_tracker->set_tracker_name("/user/body_tracker");
		xr_body_tracker->set_body_flags(XRBodyTracker::BODY_FLAG_UPPER_BODY_SUPPORTED | XRBodyTracker::BODY_FLAG_HANDS_SUPPORTED);
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
		UtilityFunctions::print("Failed to destroy body-tracker handle: ", result);
	}
	body_tracker = XR_NULL_HANDLE;

	// Unregister the body tracker.
	if (xr_body_tracker_registered) {
		XRServer *xr_server = XRServer::get_singleton();
		if (xr_server) {
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

	// Construct the locations struct.
	XrBodyJointLocationFB fb_locations[XR_BODY_JOINT_COUNT_FB] = {};
	XrBodyJointLocationsFB locations = {
		XR_TYPE_BODY_JOINT_LOCATIONS_FB, // type
		nullptr, // next
		XR_FALSE, // isActive
		0.0f, // confidence
		XR_BODY_JOINT_COUNT_FB, // jointCount
		fb_locations // jointLocations
	};

	// Read the weights
	XrResult result = xrLocateBodyJointsFB(body_tracker, &locate_info, &locations);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get body joint locations: ", result);
	}

	// Set the tracking active flag
	xr_body_tracker->set_has_tracking_data(locations.isActive);

	// Process all joints
	for (const JointMapEntry &entry : joint_table) {
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
	// - The Root joint carries no data, so instead place it on the ground
	//   under the hips pointing forwards.
	//
	// - Adjusting the left and right shoulder back so they are aligned
	//   with how models are designed, rather than the Meta positions of
	//   the tips of the clavicles.
	if (locations.isActive) {
		// Get the hips transform
		Transform3D hips = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_HIPS);

		// Construct the root under the hips pointing forwards
		Vector3 root_y = Vector3(0.0, 1.0, 0.0);
		Vector3 root_z = -hips.basis[Vector3::AXIS_X].cross(root_y);
		Vector3 root_x = root_y.cross(root_z);
		Vector3 root_o = hips.origin.slide(Vector3(0.0, 1.0, 0.0));
		Transform3D root = Transform3D(root_x, root_y, root_z, root_o).orthonormalized();
		xr_body_tracker->set_joint_transform(XRBodyTracker::JOINT_ROOT, root);
		xr_body_tracker->set_pose("default", root, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);

		// Distance in meters to push the shoulder joints back from the
		// clavicle-position to be in-line with the upper arm joints as
		// required for T-Pose designed models.
		constexpr float shoulder_z_offset = -0.07;

		// Deduce the shoulder offset from the upper chest transform
		Transform3D upper_chest = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_UPPER_CHEST);
		Vector3 shoulder_offset = upper_chest.basis[2] * shoulder_z_offset;

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
