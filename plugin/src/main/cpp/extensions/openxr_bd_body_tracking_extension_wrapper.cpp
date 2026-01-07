/**************************************************************************/
/*  openxr_bd_body_tracking_extension_wrapper.cpp                         */
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

#include "extensions/openxr_bd_body_tracking_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/templates/local_vector.hpp>

using namespace godot;

/// Joint mapping table entry
struct JointMapEntry {
	/// Joint in Godot XRBodyTracker
	XRBodyTracker::Joint xr_joint;

	/// Joint in OpenXR
	int joint;

	/// Joint rotation
	Quaternion rotation;
};

// PICO axis T-pose orientation is X:Right, Y:Up, Z:Back, so we need to apply some rotations
Quaternion quat_180 = Quaternion(Vector3(0.0, 1.0, 0.0), Math_PI);

/// Joint mapping table
static const JointMapEntry joint_table[] = {
	// Upper body joints
	{ XRBodyTracker::JOINT_HIPS, XR_BODY_JOINT_PELVIS_BD, quat_180 },

	{ XRBodyTracker::JOINT_SPINE, XR_BODY_JOINT_SPINE1_BD, quat_180 },
	{ XRBodyTracker::JOINT_CHEST, XR_BODY_JOINT_SPINE2_BD, quat_180 },
	{ XRBodyTracker::JOINT_UPPER_CHEST, XR_BODY_JOINT_SPINE3_BD, quat_180 },

	{ XRBodyTracker::JOINT_NECK, XR_BODY_JOINT_NECK_BD, quat_180 },
	{ XRBodyTracker::JOINT_HEAD, XR_BODY_JOINT_HEAD_BD, quat_180 },

	{ XRBodyTracker::JOINT_LEFT_SHOULDER, XR_BODY_JOINT_LEFT_COLLAR_BD, Quaternion(-0.5, 0.5, 0.5, 0.5) },
	{ XRBodyTracker::JOINT_LEFT_UPPER_ARM, XR_BODY_JOINT_LEFT_SHOULDER_BD, Quaternion(-0.5, 0.5, -0.5, -0.5) },
	{ XRBodyTracker::JOINT_LEFT_LOWER_ARM, XR_BODY_JOINT_LEFT_ELBOW_BD, Quaternion(0.7071067811865475244, -0.7071067811865475244, 0.0, 0.0) },
	{ XRBodyTracker::JOINT_LEFT_HAND, XR_BODY_JOINT_LEFT_HAND_BD, Quaternion(-0.5, 0.5, -0.5, -0.5) },
	// { XRBodyTracker::JOINT_LEFT_PALM, -1, Quaternion(0.5, -0.5, -0.5, -0.5) },
	// { XRBodyTracker::JOINT_LEFT_WRIST, XR_BODY_JOINT_LEFT_WRIST_BD, Quaternion(0.5, -0.5, -0.5, -0.5) },

	{ XRBodyTracker::JOINT_RIGHT_SHOULDER, XR_BODY_JOINT_RIGHT_COLLAR_BD, Quaternion(0.5, 0.5, 0.5, -0.5) },
	{ XRBodyTracker::JOINT_RIGHT_UPPER_ARM, XR_BODY_JOINT_RIGHT_SHOULDER_BD, Quaternion(0.5, 0.5, -0.5, 0.5) },
	{ XRBodyTracker::JOINT_RIGHT_LOWER_ARM, XR_BODY_JOINT_RIGHT_ELBOW_BD, Quaternion(0.7071067811865475244, 0.7071067811865475244, 0.0, 0.0) },
	{ XRBodyTracker::JOINT_RIGHT_HAND, XR_BODY_JOINT_RIGHT_HAND_BD, Quaternion(0.5, 0.5, -0.5, 0.5) },
	// { XRBodyTracker::JOINT_RIGHT_PALM, -1, Quaternion(0.5, 0.5, -0.5, 0.5) },
	// { XRBodyTracker::JOINT_RIGHT_WRIST, XR_BODY_JOINT_RIGHT_WRIST_BD, Quaternion(0.5, 0.5, -0.5, 0.5) },

	// Lower body joints
	{ XRBodyTracker::JOINT_LEFT_UPPER_LEG, XR_BODY_JOINT_LEFT_HIP_BD, Quaternion(1.0, 0.0, 0.0, 0.0) },
	{ XRBodyTracker::JOINT_LEFT_LOWER_LEG, XR_BODY_JOINT_LEFT_KNEE_BD, Quaternion(0.0, 0.0, 1.0, 0.0) },
	{ XRBodyTracker::JOINT_LEFT_FOOT, XR_BODY_JOINT_LEFT_ANKLE_BD, Quaternion(-0.7071067811865475244, 0.0, 0.0, 0.7071067811865475244) },
	{ XRBodyTracker::JOINT_LEFT_TOES, XR_BODY_JOINT_LEFT_FOOT_BD, Quaternion(0.0, 0.7071067811865475244, -0.7071067811865475244, 0.0) },

	{ XRBodyTracker::JOINT_RIGHT_UPPER_LEG, XR_BODY_JOINT_RIGHT_HIP_BD, Quaternion(1.0, 0.0, 0.0, 0.0) },
	{ XRBodyTracker::JOINT_RIGHT_LOWER_LEG, XR_BODY_JOINT_RIGHT_KNEE_BD, Quaternion(0.0, 0.0, 1.0, 0.0) },
	{ XRBodyTracker::JOINT_RIGHT_FOOT, XR_BODY_JOINT_RIGHT_ANKLE_BD, Quaternion(-0.7071067811865475244, 0.0, 0.0, 0.7071067811865475244) },
	{ XRBodyTracker::JOINT_RIGHT_TOES, XR_BODY_JOINT_RIGHT_FOOT_BD, Quaternion(0.0, 0.7071067811865475244, -0.7071067811865475244, 0.0) },
};

OpenXRBdBodyTrackingExtensionWrapper *OpenXRBdBodyTrackingExtensionWrapper::singleton = nullptr;

OpenXRBdBodyTrackingExtensionWrapper *OpenXRBdBodyTrackingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRBdBodyTrackingExtensionWrapper());
	}
	return singleton;
}

OpenXRBdBodyTrackingExtensionWrapper::OpenXRBdBodyTrackingExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRBdBodyTrackingExtensionWrapper singleton already exists.");

	request_extensions[XR_BD_BODY_TRACKING_EXTENSION_NAME] = &bd_body_tracking_ext;

	singleton = this;
}

OpenXRBdBodyTrackingExtensionWrapper::~OpenXRBdBodyTrackingExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

void OpenXRBdBodyTrackingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRBdBodyTrackingExtensionWrapper::is_enabled);
}

void OpenXRBdBodyTrackingExtensionWrapper::cleanup() {
	bd_body_tracking_ext = false;
}

uint64_t OpenXRBdBodyTrackingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (bd_body_tracking_ext) {
		system_body_tracking_properties.next = p_next_pointer;
		p_next_pointer = &system_body_tracking_properties;
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

godot::Dictionary OpenXRBdBodyTrackingExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;

		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRBdBodyTrackingExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (bd_body_tracking_ext) {
		bool result = initialize_bd_body_tracking_extension((XrInstance)p_instance);
		if (!result) {
			ERR_PRINT("Failed to initialize bd_body_tracking extension");
			bd_body_tracking_ext = false;
		}
	}
}

void OpenXRBdBodyTrackingExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRBdBodyTrackingExtensionWrapper::_on_session_created(uint64_t instance) {
	// Skip if not enabled
	if (!is_enabled()) {
		return;
	}

	// Create the body-tracker handle
	XrBodyTrackerCreateInfoBD createInfo = {
		XR_TYPE_BODY_TRACKER_CREATE_INFO_BD, // type
		nullptr, // next
		body_joint_set, // bodyJointSet
	};
	XrResult result = xrCreateBodyTrackerBD(SESSION, &createInfo, &body_tracker);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to create body-tracker handle: ", get_openxr_api()->get_error_string(result)));

	// Construct the XRBodyTracker if necessary
	if (xr_body_tracker.is_null()) {
		xr_body_tracker.instantiate();
		xr_body_tracker->set_tracker_name("/user/body_tracker");

		BitField<XRBodyTracker::BodyFlags> body_flags = XRBodyTracker::BODY_FLAG_UPPER_BODY_SUPPORTED | XRBodyTracker::BODY_FLAG_LOWER_BODY_SUPPORTED;
		xr_body_tracker->set_body_flags(body_flags);
	}
}

void OpenXRBdBodyTrackingExtensionWrapper::_on_session_destroyed() {
	// Skip if no body-tracker handle
	if (!body_tracker) {
		return;
	}

	// Destroy the body-tracker handle
	XrResult result = xrDestroyBodyTrackerBD(body_tracker);
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

void OpenXRBdBodyTrackingExtensionWrapper::_on_process() {
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
	XrBodyJointsLocateInfoBD locate_info = {
		XR_TYPE_BODY_JOINTS_LOCATE_INFO_BD, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
		display_time // time
	};

	// Construct the locations struct.
	XrBodyJointLocationBD bd_locations[XR_BODY_JOINT_COUNT_BD];
	XrBodyJointLocationsBD locations = {
		XR_TYPE_BODY_JOINT_LOCATIONS_BD, // type
		nullptr, // next
		XR_FALSE, // allJointPosesTracked
		XR_BODY_JOINT_COUNT_BD, // jointLocationCount
		bd_locations // jointLocations
	};

	// Read the weights
	XrResult result = xrLocateBodyJointsBD(body_tracker, &locate_info, &locations);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to get body joint locations: ", get_openxr_api()->get_error_string(result)));

	// Set the tracking active flag
	xr_body_tracker->set_has_tracking_data(locations.allJointPosesTracked); // Should we use allJointPosesTracked here?

	// Process all joints
	for (const JointMapEntry &entry : joint_table) {
		// Process the joint pose
		const XrBodyJointLocationBD &location = bd_locations[entry.joint];
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
	// - PICO doesn't have a root node, so we create one on the ground under the hip joint.
	//
	// - ??? Adjusting the left and right shoulder back so they are aligned
	//   with how models are designed, rather than the PICO positions of
	//   the tips of the clavicles.
	if (locations.allJointPosesTracked) { // should we use allJointPosesTracked here?
		// Align root under the hips pointing 'forwards'
		// IE, +z aligns with hips & user's real-world [upper-body] forward.
		// (Remaining, however, parallel to the XRorigin / Global XZ plane; root's basis rotated around Y to fit)

		Transform3D hips = xr_body_tracker->get_joint_transform(XRBodyTracker::JOINT_HIPS);
		Vector3 root_y = Vector3(0.0, 1.0, 0.0);
		Vector3 hips_left = hips.basis.get_column(Vector3::AXIS_X);
		Vector3 root_x = (hips_left.slide(Vector3(0.0, 1.0, 0.0))).normalized();
		Vector3 root_z = root_x.cross(root_y);
		Vector3 root_o = Vector3(hips.origin.x, 0.0, hips.origin.z);
		Transform3D root = Transform3D(root_x, root_y, root_z, root_o).orthonormalized();
		BitField<XRBodyTracker::JointFlags> flags(0);
		flags.set_flag(XRBodyTracker::JOINT_FLAG_ORIENTATION_VALID);
		flags.set_flag(XRBodyTracker::JOINT_FLAG_POSITION_VALID);
		xr_body_tracker->set_joint_flags(XRBodyTracker::JOINT_ROOT, flags);
		xr_body_tracker->set_joint_transform(XRBodyTracker::JOINT_ROOT, root);

		// Set tracker pose, velocities, confidence.
		xr_body_tracker->set_pose("default", root, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);

		/*
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
		*/
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

bool OpenXRBdBodyTrackingExtensionWrapper::is_enabled() const {
	return bd_body_tracking_ext && system_body_tracking_properties.supportsBodyTracking;
}

bool OpenXRBdBodyTrackingExtensionWrapper::initialize_bd_body_tracking_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateBodyTrackerBD);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyBodyTrackerBD);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateBodyJointsBD);

	return true;
}
