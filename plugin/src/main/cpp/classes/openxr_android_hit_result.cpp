/**************************************************************************/
/*  openxr_android_hit_result.cpp                                         */
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

#include "classes/openxr_android_hit_result.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include "extensions/openxr_android_trackables_extension.h"

using namespace godot;

OpenXRAndroidHitResult::OpenXRAndroidHitResult() {}
OpenXRAndroidHitResult::~OpenXRAndroidHitResult() {}

bool OpenXRAndroidHitResult::set_hit_result(const XrRaycastHitResultANDROID &p_hit_result) {
	switch ((int)p_hit_result.type) {
		case XR_TRACKABLE_TYPE_PLANE_ANDROID: {
			OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
			if (wrapper == nullptr) {
				UtilityFunctions::printerr("OpenXR: expected non-null OpenXRAndroidTrackablesExtension; returning null plane trackable");
				trackable = Ref<OpenXRAndroidTrackableTracker>();
			} else {
				// NOTE: don't bother updating the tracker (if it already exists) since perhaps this
				// tracker's data is ultimately ignored by the caller.  Like any other OpenXRAndroidTrackableTracker,
				// calling a getter function will ensure latest state if it's stale.
				trackable = wrapper->get_or_create_tracker_and_update(p_hit_result.trackable, XR_TRACKABLE_TYPE_PLANE_ANDROID, false);
			}
			break;
		}

		case XR_TRACKABLE_TYPE_DEPTH_ANDROID:
			// DEPTH hit results should not return a trackable
			if (p_hit_result.trackable != XR_NULL_TRACKABLE_ANDROID) {
				UtilityFunctions::print_verbose("OpenXR: unexpected trackable received for DEPTH hit result");
			}

			trackable = Ref<OpenXRAndroidTrackableTracker>();
			break;

		case XR_TRACKABLE_TYPE_NOT_VALID_ANDROID:
		case XR_TRACKABLE_TYPE_OBJECT_ANDROID:
		case XR_TRACKABLE_TYPE_MAX_ENUM_ANDROID:
		default:
			UtilityFunctions::printerr("OpenXR: unsupported hit trackable type: ", p_hit_result.type);
			return false;
	}

	pose.origin.x = p_hit_result.pose.position.x;
	pose.origin.y = p_hit_result.pose.position.y;
	pose.origin.z = p_hit_result.pose.position.z;
	pose.basis = Basis{ Quaternion{ p_hit_result.pose.orientation.x, p_hit_result.pose.orientation.y, p_hit_result.pose.orientation.z, p_hit_result.pose.orientation.w } };

	return true;
}

Ref<OpenXRAndroidTrackableTracker> OpenXRAndroidHitResult::get_tracker() const {
	return trackable;
}

const Transform3D &OpenXRAndroidHitResult::get_pose() const {
	return pose;
}

void OpenXRAndroidHitResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_tracker"), &OpenXRAndroidHitResult::get_tracker);
	ClassDB::bind_method(D_METHOD("get_pose"), &OpenXRAndroidHitResult::get_pose);
}
