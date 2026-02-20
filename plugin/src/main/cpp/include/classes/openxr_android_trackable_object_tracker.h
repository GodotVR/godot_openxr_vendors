/**************************************************************************/
/*  openxr_android_trackable_object_tracker.h                             */
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

#include "classes/openxr_android_trackable_tracker.h"

using namespace godot;

class OpenXRAndroidTrackableObjectTracker : public OpenXRAndroidTrackableTracker {
	GDCLASS(OpenXRAndroidTrackableObjectTracker, OpenXRAndroidTrackableTracker);

public:
	OpenXRAndroidTrackableObjectTracker();
	virtual ~OpenXRAndroidTrackableObjectTracker() override;

	enum ObjectLabel {
		OBJECT_LABEL_UNKNOWN,
		OBJECT_LABEL_KEYBOARD,
		OBJECT_LABEL_MOUSE,
		OBJECT_LABEL_LAPTOP,
	};

	static Ref<OpenXRAndroidTrackableObjectTracker> create(XrTrackableANDROID p_trackable);

	Vector3 get_extents();
	ObjectLabel get_object_label();

protected:
	static void _bind_methods();

private:
	void _ensure_updated() override;
	bool _get_xrtrackable_info(XrTrackableObjectANDROID &p_xr_trackable_object);

	uint64_t last_frame = -1;

	Vector3 extents;
	ObjectLabel object_label = OBJECT_LABEL_UNKNOWN;
};

VARIANT_ENUM_CAST(OpenXRAndroidTrackableObjectTracker::ObjectLabel);
