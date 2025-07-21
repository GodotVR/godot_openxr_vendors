/**************************************************************************/
/*  openxr_ml_marker_detector_april_tag_settings.cpp                      */
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

#include "classes/openxr_ml_marker_tracker.h"
#include "extensions/openxr_ml_marker_understanding_extension_wrapper.h"

using namespace godot;

void OpenXRMlMarkerTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_marker_type", "marker_type"), &OpenXRMlMarkerTracker::set_marker_type);
	ClassDB::bind_method(D_METHOD("get_marker_type"), &OpenXRMlMarkerTracker::get_marker_type);

	ClassDB::bind_method(D_METHOD("set_reprojection_error_meters", "reprojection_error_meters"), &OpenXRMlMarkerTracker::set_reprojection_error_meters);
	ClassDB::bind_method(D_METHOD("get_reprojection_error_meters"), &OpenXRMlMarkerTracker::get_reprojection_error_meters);

	ClassDB::bind_method(D_METHOD("set_marker_length", "marker_length"), &OpenXRMlMarkerTracker::set_marker_length);
	ClassDB::bind_method(D_METHOD("get_marker_length"), &OpenXRMlMarkerTracker::get_marker_length);

	ClassDB::bind_method(D_METHOD("set_marker_number", "marker_number"), &OpenXRMlMarkerTracker::set_marker_number);
	ClassDB::bind_method(D_METHOD("get_marker_number"), &OpenXRMlMarkerTracker::get_marker_number);

	ClassDB::bind_method(D_METHOD("set_marker_string", "marker_string"), &OpenXRMlMarkerTracker::set_marker_string);
	ClassDB::bind_method(D_METHOD("get_marker_string"), &OpenXRMlMarkerTracker::get_marker_string);

	ADD_SIGNAL(MethodInfo("openxr_ml_marker_updated"));
}

void OpenXRMlMarkerTracker::set_marker_type(OpenXRMlMarkerDetectorSettings::MarkerType p_marker_type) {
	marker_type = p_marker_type;
}

OpenXRMlMarkerDetectorSettings::MarkerType OpenXRMlMarkerTracker::get_marker_type() const {
	return marker_type;
}

void OpenXRMlMarkerTracker::set_reprojection_error_meters(float p_reprojection_error_meters) {
	reprojection_error_meters = p_reprojection_error_meters;
}

float OpenXRMlMarkerTracker::get_reprojection_error_meters() const {
	return reprojection_error_meters;
}

void OpenXRMlMarkerTracker::set_marker_length(float p_marker_length) {
	marker_length = p_marker_length;
}

float OpenXRMlMarkerTracker::get_marker_length() const {
	return marker_length;
}

void OpenXRMlMarkerTracker::set_marker_number(uint64_t p_marker_number) {
	marker_number = p_marker_number;
}

uint64_t OpenXRMlMarkerTracker::get_marker_number() const {
	return marker_number;
}

void OpenXRMlMarkerTracker::set_marker_string(const String &p_marker_string) {
	marker_string = p_marker_string;
}

String OpenXRMlMarkerTracker::get_marker_string() const {
	return marker_string;
}

void OpenXRMlMarkerTracker::update_marker() {
	if (marker_detector == XR_NULL_HANDLE)
		return;

	OpenXRMlMarkerUnderstandingExtensionWrapper *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtensionWrapper::get_singleton();
	marker_length = marker_understanding_extension->get_marker_length(marker_detector, marker_atom);
	if (marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_QR || marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_CODE_128 || marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_EAN_13 || marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_UPC_A) {
		// Qr, Code128, Ean13, and UPCA markers contain a string
		marker_string = marker_understanding_extension->get_marker_string(marker_detector, marker_atom);
	} else {
		// Aruco, and April Tag markers have marker number and reprojection errors
		reprojection_error_meters = marker_understanding_extension->get_marker_reprojection_error(marker_detector, marker_atom);
		marker_number = marker_understanding_extension->get_marker_number(marker_detector, marker_atom);
	}

	// Aruco, Qr, and April Tag markers can be position tracked
	if (marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_ARUCO || marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_QR || marker_type == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_APRIL_TAG) {
		if (marker_space_handle == XR_NULL_HANDLE) {
			marker_space_handle = marker_understanding_extension->create_marker_space(marker_detector, marker_atom); // Try to create marker space
		}

		if (marker_space_handle != XR_NULL_HANDLE) {
			Ref<XRPositionalTracker> self = Ref(this);
			marker_understanding_extension->update_tracker_pose(marker_space_handle, self);
		}
	}
	emit_signal("openxr_ml_marker_updated");
}

OpenXRMlMarkerTracker::OpenXRMlMarkerTracker(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker_atom, OpenXRMlMarkerDetectorSettings::MarkerType p_marker_type) {
	set_tracker_type(XRServer::TrackerType::TRACKER_ANCHOR);
	marker_detector = p_marker_detector;
	marker_atom = p_marker_atom;
	marker_type = p_marker_type;
}
