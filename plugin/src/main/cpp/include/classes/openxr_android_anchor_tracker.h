/**************************************************************************/
/*  openxr_android_anchor_tracker.h                                       */
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
#include <godot_cpp/classes/xr_positional_tracker.hpp>

#include "classes/openxr_android_trackable_tracker.h"

using namespace godot;

class OpenXRAndroidAnchorTracker : public XRPositionalTracker {
	GDCLASS(OpenXRAndroidAnchorTracker, XRPositionalTracker);

public:
	OpenXRAndroidAnchorTracker();
	virtual ~OpenXRAndroidAnchorTracker() override;

	static Ref<OpenXRAndroidAnchorTracker> create(XrSpace p_xrspace, Ref<OpenXRAndroidTrackableTracker> p_tracker);

	void update();
	XrSpace get_xrspace() const;
	Ref<OpenXRAndroidTrackableTracker> get_tracker() const;

	enum LocationFlags {
		LOCATION_FLAGS_ORIENTATION_VALID = 1,
		LOCATION_FLAGS_POSITION_VALID = 2,
		LOCATION_FLAGS_ORIENTATION_TRACKED = 4,
		LOCATION_FLAGS_POSITION_TRACKED = 8,
	};

	BitField<LocationFlags> get_location_flags(bool p_update = false);
	Transform3D get_location_pose(bool p_update = false);

	static bool get_xranchor_space_location(XrSpace p_xrspace, XrSpaceLocation &o_xrspace_location);
	static XrSpaceLocation create_empty_location();

protected:
	static void _bind_methods();

private:
	void _update_location(bool p_update);

	XrSpace space = XR_NULL_HANDLE;
	XrSpaceLocation location = {};
	Ref<OpenXRAndroidTrackableTracker> tracker;
	BitField<LocationFlags> location_flags = 0;
	Transform3D location_pose;
};

VARIANT_BITFIELD_CAST(OpenXRAndroidAnchorTracker::LocationFlags);
