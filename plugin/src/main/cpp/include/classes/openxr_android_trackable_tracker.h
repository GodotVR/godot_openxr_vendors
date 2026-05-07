/**************************************************************************/
/*  openxr_android_trackable_tracker.h                                    */
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

#pragma once

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>

using namespace godot;

class OpenXRAndroidTrackableTracker : public XRPositionalTracker {
	GDCLASS(OpenXRAndroidTrackableTracker, XRPositionalTracker);

public:
	OpenXRAndroidTrackableTracker();
	virtual ~OpenXRAndroidTrackableTracker() override;

	enum TrackingState {
		TRACKING_STATE_PAUSED,
		TRACKING_STATE_STOPPED,
		TRACKING_STATE_TRACKING,
		TRACKING_STATE_UNKNOWN,
	};

	XrTrackableANDROID get_xrtrackable() const;
	XrTrackableTypeANDROID get_xrtrackable_type() const;
	XrTrackableTrackerANDROID get_xrtrackable_tracker() const;
	TrackingState get_tracking_state();
	const Transform3D &get_center_pose();
	void update();
	void deinit();

protected:
	static void _bind_methods();

	void _init(XrTrackableANDROID p_xrtrackable, XrTrackableTrackerANDROID p_xrtrackable_tracker, XrTrackableTypeANDROID p_xrtrackable_type);
	void _set_tracking_state(XrTrackingStateANDROID p_xr_tracking_state);
	void _set_center_pose(const XrPosef &p_center_pose);
	XrTime _set_last_updated_time(XrTime p_last_updated_time);

private:
	virtual void _ensure_updated() = 0;

	TrackingState tracking_state = TRACKING_STATE_UNKNOWN;
	XrTrackableANDROID xrtrackable = XR_NULL_TRACKABLE_ANDROID;
	XrTrackableTypeANDROID xrtrackable_type = XR_TRACKABLE_TYPE_NOT_VALID_ANDROID;
	XrTrackableTrackerANDROID xrtrackable_tracker = XR_NULL_HANDLE;
	Transform3D center_pose;
	XrTime last_updated_time = -1;
};

VARIANT_ENUM_CAST(OpenXRAndroidTrackableTracker::TrackingState);
