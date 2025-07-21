/**************************************************************************/
/*  openxr_ml_marker_detector.h                                           */
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

#ifndef OPENXR_ML_MARKER_DETECTOR_H
#define OPENXR_ML_MARKER_DETECTOR_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "classes/openxr_ml_marker_detector_profile_settings.h"
#include "classes/openxr_ml_marker_detector_settings.h"
#include "classes/openxr_ml_marker_tracker.h"

namespace godot {
class OpenXRMlMarkerDetector : public Node {
	GDCLASS(OpenXRMlMarkerDetector, Node);

public:
	enum Status {
		STATUS_PENDING,
		STATUS_READY,
		STATUS_ERROR,
		STATUS_MAX
	};

private:
	Status status = Status::STATUS_PENDING;
	TypedArray<OpenXRMlMarkerTracker> marker_trackers;
	XrMarkerDetectorML marker_detector = nullptr;
	bool has_snapshot = false;
	bool detector_settings_dirty = false;
	Ref<OpenXRMlMarkerDetectorSettings> settings;
	Ref<OpenXRMlMarkerDetectorProfileSettings> active_profile_settings;

	void _on_updating_marker_detectors();
	void _on_settings_changed();
	void _create_detector();
	void _destroy_detector();
	void _update_detector();
	void _notification(int p_what);
	Ref<OpenXRMlMarkerTracker> get_or_create_marker_tracker_by_atom(XrMarkerML p_marker);

protected:
	static int next_marker_id;
	static void _bind_methods();

public:
	Status get_status() const;
	TypedArray<OpenXRMlMarkerTracker> get_marker_trackers() const;

	void set_settings(const Ref<OpenXRMlMarkerDetectorSettings> &p_settings);
	Ref<OpenXRMlMarkerDetectorSettings> get_settings() const;
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRMlMarkerDetector::Status);

#endif
