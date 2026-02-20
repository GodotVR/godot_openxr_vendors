/**************************************************************************/
/*  openxr_android_anchor_tracker.cpp                                     */
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

#include "classes/openxr_android_anchor_tracker.h"
#include <godot_cpp/classes/open_xrapi_extension.hpp>

#include "extensions/openxr_android_trackables_extension.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/object.hpp"
#include "util.h"

using namespace godot;

OpenXRAndroidAnchorTracker::OpenXRAndroidAnchorTracker() {}
OpenXRAndroidAnchorTracker::~OpenXRAndroidAnchorTracker() {}

Ref<OpenXRAndroidAnchorTracker> OpenXRAndroidAnchorTracker::create(XrSpace p_xrspace, Ref<OpenXRAndroidTrackableTracker> p_tracker) {
	Ref<OpenXRAndroidAnchorTracker> ret;
	ret.instantiate();
	ret->space = p_xrspace;
	ret->tracker = p_tracker;
	ret->location = create_empty_location();
	ret->set_tracker_type(XRServer::TRACKER_ANCHOR);
	ret->set_tracker_name(String("openxr/androidxr/anchor_tracker/") + String::num_uint64(OpenXRAndroidTrackablesExtension::get_next_tracker_id()));

	// Ensure its cached state is sync'd with the xr runtime, just so the caller doesn't have to
	// explicitly remember to call "get_location/etc" with
	// "p_update" true.
	ret->update();

	return ret;
}

void OpenXRAndroidAnchorTracker::update() {
	_update_location(true);
}

XrSpace OpenXRAndroidAnchorTracker::get_xrspace() const {
	return space;
}

Ref<OpenXRAndroidTrackableTracker> OpenXRAndroidAnchorTracker::get_tracker() const {
	return tracker;
}

BitField<OpenXRAndroidAnchorTracker::LocationFlags> OpenXRAndroidAnchorTracker::get_location_flags(bool p_update) {
	_update_location(p_update);
	return location_flags;
}

Transform3D OpenXRAndroidAnchorTracker::get_location_pose(bool p_update) {
	_update_location(p_update);
	return location_pose;
}

void OpenXRAndroidAnchorTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_tracker"), &OpenXRAndroidAnchorTracker::get_tracker);
	ADD_SIGNAL(MethodInfo("location_flags_changed"));

	ClassDB::bind_method(D_METHOD("get_location_flags", "update"), &OpenXRAndroidAnchorTracker::get_location_flags, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_location_pose", "update"), &OpenXRAndroidAnchorTracker::get_location_pose, DEFVAL(false));
	BIND_BITFIELD_FLAG(LOCATION_FLAGS_ORIENTATION_VALID);
	BIND_BITFIELD_FLAG(LOCATION_FLAGS_POSITION_VALID);
	BIND_BITFIELD_FLAG(LOCATION_FLAGS_ORIENTATION_TRACKED);
	BIND_BITFIELD_FLAG(LOCATION_FLAGS_POSITION_TRACKED);
}

bool OpenXRAndroidAnchorTracker::get_xranchor_space_location(XrSpace p_xrspace, XrSpaceLocation &o_xrspace_location) {
	if (p_xrspace == XR_NULL_HANDLE) {
		UtilityFunctions::printerr("OpenXR: null anchor space");
		return false;
	}

	o_xrspace_location = create_empty_location();
	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	XrResult result = wrapper->xrLocateSpace(p_xrspace, (XrSpace)wrapper->get_openxr_api()->get_play_space(), (XrTime)wrapper->get_openxr_api()->get_predicted_display_time(), &o_xrspace_location);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to locate anchor space; ", wrapper->get_openxr_api()->get_error_string(result));
		return false;
	}

	return true;
}

XrSpaceLocation OpenXRAndroidAnchorTracker::create_empty_location() {
	return XrSpaceLocation{
		XR_TYPE_SPACE_LOCATION, // type
		nullptr, // next
		0, // locationFlags
		{}, // pose
	};
}

void OpenXRAndroidAnchorTracker::_update_location(bool p_update) {
	if (!p_update) {
		return;
	}

	XrSpaceLocation new_location;
	if (!get_xranchor_space_location(space, new_location)) {
		UtilityFunctions::printerr("OpenXR: unable to get XrSpaceLocation");
		return;
	}

	if (location.locationFlags != new_location.locationFlags) {
		location.locationFlags = new_location.locationFlags;

		location_flags = 0;
		if (0 != (XR_SPACE_LOCATION_ORIENTATION_VALID_BIT & new_location.locationFlags)) {
			location_flags.set_flag(LOCATION_FLAGS_ORIENTATION_VALID);
		}

		if (0 != (XR_SPACE_LOCATION_POSITION_VALID_BIT & new_location.locationFlags)) {
			location_flags.set_flag(LOCATION_FLAGS_POSITION_VALID);
		}

		if (0 != (XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT & new_location.locationFlags)) {
			location_flags.set_flag(LOCATION_FLAGS_ORIENTATION_TRACKED);
		}

		if (0 != (XR_SPACE_LOCATION_POSITION_TRACKED_BIT & new_location.locationFlags)) {
			location_flags.set_flag(LOCATION_FLAGS_POSITION_TRACKED);
		}

		emit_signal("location_flags_changed");
	}

	if (0 == (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) || 0 == (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)) {
		return;
	}

	// try to exit early since set_pose() does a lot of work, and
	// the pose doesn't change most of the time
	if (location.pose.position.x == new_location.pose.position.x && location.pose.position.y == new_location.pose.position.y && location.pose.position.z == new_location.pose.position.z && location.pose.orientation.x == new_location.pose.orientation.x && location.pose.orientation.y == new_location.pose.orientation.y && location.pose.orientation.z == new_location.pose.orientation.z && location.pose.orientation.w == new_location.pose.orientation.w) {
		return;
	}
	location.pose = new_location.pose;

	location_pose.origin.x = location.pose.position.x;
	location_pose.origin.y = location.pose.position.y;
	location_pose.origin.z = location.pose.position.z;
	location_pose.basis = Basis{ Quaternion{ location.pose.orientation.x, location.pose.orientation.y, location.pose.orientation.z, location.pose.orientation.w } };

	XRPose::TrackingConfidence confidence;
	if (0 == (location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) || 0 == (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
		confidence = XRPose::XR_TRACKING_CONFIDENCE_NONE;
	} else {
		confidence = XRPose::XR_TRACKING_CONFIDENCE_HIGH;
	}

	set_pose(StringName("default"), location_pose, Vector3(), Vector3(), confidence);
}
