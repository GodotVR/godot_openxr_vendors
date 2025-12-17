/**************************************************************************/
/*  openxr_ml_marker_tracker.h                                            */
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

#pragma once

#include "classes/openxr_ml_marker_detector_settings.h"
#include <openxr/openxr.h>
#include <godot_cpp/classes/xr_pose.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/core/binder_common.hpp>

namespace godot {
class OpenXRMlMarkerTracker : public XRPositionalTracker {
	GDCLASS(OpenXRMlMarkerTracker, XRPositionalTracker);

public:
private:
	float reprojection_error_meters = 0;
	float marker_length = 0;
	uint64_t marker_number = 0;
	String marker_string;
	OpenXRMlMarkerDetectorSettings::MarkerType marker_type;
	XrMarkerDetectorML marker_detector = XR_NULL_HANDLE;
	XrMarkerML marker_atom = 0;
	XrSpace marker_space_handle = XR_NULL_HANDLE;

protected:
	static void _bind_methods();

public:
	_FORCE_INLINE_ XrMarkerML get_marker_atom() { return marker_atom; }

	void set_marker_type(OpenXRMlMarkerDetectorSettings::MarkerType p_marker_type);
	OpenXRMlMarkerDetectorSettings::MarkerType get_marker_type() const;

	void set_reprojection_error_meters(float p_reprojection_error_meters);
	float get_reprojection_error_meters() const;

	void set_marker_length(float p_marker_length);
	float get_marker_length() const;

	void set_marker_number(uint64_t p_marker_number);
	uint64_t get_marker_number() const;

	void set_marker_string(const String &p_marker_string);
	String get_marker_string() const;

	void update_marker();

	OpenXRMlMarkerTracker() = default;
	OpenXRMlMarkerTracker(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker_atom, OpenXRMlMarkerDetectorSettings::MarkerType p_marker_type);
};
} // namespace godot
