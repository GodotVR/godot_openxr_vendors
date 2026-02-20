/**************************************************************************/
/*  openxr_android_trackable_tracker.cpp                                  */
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

#include "classes/openxr_android_trackable_tracker.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "extensions/openxr_android_trackables_extension.h"

using namespace godot;

OpenXRAndroidTrackableTracker::OpenXRAndroidTrackableTracker() {}
OpenXRAndroidTrackableTracker::~OpenXRAndroidTrackableTracker() {}

XrTrackableANDROID OpenXRAndroidTrackableTracker::get_trackable() const {
	return trackable;
}

XrTrackableTypeANDROID OpenXRAndroidTrackableTracker::get_trackable_type() const {
	return trackable_type;
}

OpenXRAndroidTrackableTracker::TrackingState OpenXRAndroidTrackableTracker::get_tracking_state() {
	_ensure_updated();
	return tracking_state;
}

const Transform3D &OpenXRAndroidTrackableTracker::get_center_pose() {
	_ensure_updated();
	return center_pose;
}

void OpenXRAndroidTrackableTracker::update() {
	_ensure_updated();
}

void OpenXRAndroidTrackableTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_tracking_state"), &OpenXRAndroidTrackableTracker::get_tracking_state);
	ClassDB::bind_method(D_METHOD("get_center_pose"), &OpenXRAndroidTrackableTracker::get_center_pose);
	ADD_SIGNAL(MethodInfo("updated"));
	BIND_ENUM_CONSTANT(TRACKING_STATE_PAUSED);
	BIND_ENUM_CONSTANT(TRACKING_STATE_STOPPED);
	BIND_ENUM_CONSTANT(TRACKING_STATE_TRACKING);
	BIND_ENUM_CONSTANT(TRACKING_STATE_UNKNOWN);
}

void OpenXRAndroidTrackableTracker::_init(XrTrackableANDROID p_trackable, XrTrackableTypeANDROID p_trackable_type) {
	trackable = p_trackable;
	trackable_type = p_trackable_type;
	set_tracker_type(XRServer::TRACKER_ANCHOR);
	set_tracker_name(String("openxr/androidxr/trackable/") + String::num_uint64(OpenXRAndroidTrackablesExtension::get_next_tracker_id()));
}

void OpenXRAndroidTrackableTracker::_set_tracking_state(XrTrackingStateANDROID p_xr_tracking_state) {
	OpenXRAndroidTrackableTracker::TrackingState new_tracking_state;
	switch (p_xr_tracking_state) {
		case XR_TRACKING_STATE_PAUSED_ANDROID:
			new_tracking_state = TRACKING_STATE_PAUSED;
			break;
		case XR_TRACKING_STATE_STOPPED_ANDROID:
			new_tracking_state = TRACKING_STATE_STOPPED;
			break;
		case XR_TRACKING_STATE_TRACKING_ANDROID:
			new_tracking_state = TRACKING_STATE_TRACKING;
			break;
		case XR_TRACKING_STATE_MAX_ENUM_ANDROID:
			new_tracking_state = TRACKING_STATE_UNKNOWN;
			break;
		default:
			UtilityFunctions::printerr("OpenXR: invalid tracking state: ", p_xr_tracking_state);
			new_tracking_state = TRACKING_STATE_UNKNOWN;
			break;
	}

	tracking_state = new_tracking_state;
}

void OpenXRAndroidTrackableTracker::_set_center_pose(const XrPosef &p_center_pose) {
	center_pose.origin.x = p_center_pose.position.x;
	center_pose.origin.y = p_center_pose.position.y;
	center_pose.origin.z = p_center_pose.position.z;
	center_pose.basis = Basis{ Quaternion{ p_center_pose.orientation.x, p_center_pose.orientation.y, p_center_pose.orientation.z, p_center_pose.orientation.w } };

	XRPose::TrackingConfidence confidence;
	switch (get_tracking_state()) {
		case TRACKING_STATE_TRACKING:
			confidence = XRPose::TrackingConfidence::XR_TRACKING_CONFIDENCE_HIGH;
			break;
		case TRACKING_STATE_PAUSED:
		case TRACKING_STATE_STOPPED:
		case TRACKING_STATE_UNKNOWN:
		default:
			confidence = XRPose::TrackingConfidence::XR_TRACKING_CONFIDENCE_NONE;
			break;
	}
	set_pose(StringName("default"), center_pose, Vector3(), Vector3(), confidence);
}

XrTime OpenXRAndroidTrackableTracker::_set_last_updated_time(XrTime p_last_updated_time) {
	XrTime old_last_update_time = last_updated_time;
	last_updated_time = p_last_updated_time;
	return old_last_update_time;
}
