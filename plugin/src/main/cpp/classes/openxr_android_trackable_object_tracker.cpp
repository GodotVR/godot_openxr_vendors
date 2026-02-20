/**************************************************************************/
/*  openxr_android_trackable_object_tracker.cpp                           */
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

#include "classes/openxr_android_trackable_object_tracker.h"

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "extensions/openxr_android_trackables_extension.h"

using namespace godot;

OpenXRAndroidTrackableObjectTracker::OpenXRAndroidTrackableObjectTracker() {}
OpenXRAndroidTrackableObjectTracker::~OpenXRAndroidTrackableObjectTracker() {}

Ref<OpenXRAndroidTrackableObjectTracker> OpenXRAndroidTrackableObjectTracker::create(XrTrackableANDROID p_trackable) {
	Ref<OpenXRAndroidTrackableObjectTracker> ret{};
	if (p_trackable == XR_NULL_TRACKABLE_ANDROID) {
		return ret;
	}

	ret.instantiate();
	ret->_init(p_trackable, XR_TRACKABLE_TYPE_OBJECT_ANDROID);
	return ret;
}

Vector3 OpenXRAndroidTrackableObjectTracker::get_extents() {
	_ensure_updated();
	return extents;
}

OpenXRAndroidTrackableObjectTracker::ObjectLabel OpenXRAndroidTrackableObjectTracker::get_object_label() {
	_ensure_updated();
	return object_label;
}

void OpenXRAndroidTrackableObjectTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_extents"), &OpenXRAndroidTrackableObjectTracker::get_extents);
	ClassDB::bind_method(D_METHOD("get_object_label"), &OpenXRAndroidTrackableObjectTracker::get_object_label);

	BIND_ENUM_CONSTANT(OBJECT_LABEL_UNKNOWN);
	BIND_ENUM_CONSTANT(OBJECT_LABEL_KEYBOARD);
	BIND_ENUM_CONSTANT(OBJECT_LABEL_MOUSE);
	BIND_ENUM_CONSTANT(OBJECT_LABEL_LAPTOP);
}

void OpenXRAndroidTrackableObjectTracker::_ensure_updated() {
	// This function can be called multiple times per frame, however it will only do work once per
	// frame
	uint64_t frame = Engine::get_singleton()->get_process_frames();
	if (last_frame == frame) {
		return;
	}
	last_frame = frame;

	XrTrackableObjectANDROID xr_trackable_object;
	if (!_get_xrtrackable_info(xr_trackable_object)) {
		_set_tracking_state(XR_TRACKING_STATE_STOPPED_ANDROID);
		return;
	}

	// always set pose, even when old last updated time == xr_trackable_plane.lastUpdatedTime, since
	// it could have changed due to XR changing the reference space
	extents.x = xr_trackable_object.extents.width;
	extents.y = xr_trackable_object.extents.height;
	extents.z = xr_trackable_object.extents.depth;
	_set_center_pose(xr_trackable_object.centerPose);

	if (_set_last_updated_time(xr_trackable_object.lastUpdatedTime) == xr_trackable_object.lastUpdatedTime) {
		return;
	}

	switch (xr_trackable_object.objectLabel) {
		case XR_OBJECT_LABEL_UNKNOWN_ANDROID:
			object_label = OBJECT_LABEL_UNKNOWN;
			break;
		case XR_OBJECT_LABEL_KEYBOARD_ANDROID:
			object_label = OBJECT_LABEL_KEYBOARD;
			break;
		case XR_OBJECT_LABEL_MOUSE_ANDROID:
			object_label = OBJECT_LABEL_MOUSE;
			break;
		case XR_OBJECT_LABEL_LAPTOP_ANDROID:
			object_label = OBJECT_LABEL_LAPTOP;
			break;
		case XR_OBJECT_LABEL_MAX_ENUM_ANDROID:
		default:
			UtilityFunctions::printerr("OpenXR: invalid object label: ", xr_trackable_object.objectLabel);
			_set_tracking_state(XR_TRACKING_STATE_STOPPED_ANDROID);
			return;
	}

	_set_tracking_state(xr_trackable_object.trackingState);

	// Any recursive calls this 'updated' signal brings will early-return.
	// It should never infinitely recurse.
	emit_signal(StringName("updated"));
}

bool OpenXRAndroidTrackableObjectTracker::_get_xrtrackable_info(XrTrackableObjectANDROID &p_xr_trackable_object) {
	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	XrTrackableTrackerANDROID trackable_tracker = wrapper->get_or_create_xrtrackable_tracker(XR_TRACKABLE_TYPE_OBJECT_ANDROID);
	if (trackable_tracker == XR_NULL_HANDLE) {
		return false;
	}

	XrTrackableGetInfoANDROID get_info = {
		XR_TYPE_TRACKABLE_GET_INFO_ANDROID, // type
		nullptr, // next
		get_trackable(), // trackable
		(XrSpace)wrapper->get_openxr_api()->get_play_space(), // baseSpace
		(XrTime)wrapper->get_openxr_api()->get_predicted_display_time(), // time
	};

	p_xr_trackable_object = XrTrackableObjectANDROID{
		XR_TYPE_TRACKABLE_OBJECT_ANDROID, // type
		nullptr, // next
		XR_TRACKING_STATE_PAUSED_ANDROID, // trackingState
		{}, // centerPose
		{}, // extents
		XR_OBJECT_LABEL_UNKNOWN_ANDROID, // objectLabel
		0, // lastUpdatedTime
	};
	XrResult result = wrapper->xrGetTrackableObjectANDROID(trackable_tracker, &get_info, &p_xr_trackable_object);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get trackable object; ", wrapper->get_openxr_api()->get_error_string(result));
		return false;
	}

	return true;
}
