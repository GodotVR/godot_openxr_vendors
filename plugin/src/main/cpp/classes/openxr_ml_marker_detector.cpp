/**************************************************************************/
/*  openxr_ml_marker_detector_settings.cpp                                */
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

#include "classes/openxr_ml_marker_detector.h"
#include "classes/openxr_ml_marker_detector_april_tag_settings.h"
#include "classes/openxr_ml_marker_detector_aruco_settings.h"
#include "classes/openxr_ml_marker_detector_profile_settings.h"
#include "classes/openxr_ml_marker_detector_qr_settings.h"
#include "classes/openxr_ml_marker_detector_settings.h"
#include "extensions/openxr_ml_marker_understanding_extension.h"
#include "godot_cpp/core/error_macros.hpp"
#include "openxr/openxr.h"

#include <algorithm>
#include <vector>

using namespace godot;

int OpenXRMlMarkerDetector::next_marker_id = 1;

void OpenXRMlMarkerDetector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_status"), &OpenXRMlMarkerDetector::get_status);
	ClassDB::bind_method(D_METHOD("get_marker_trackers"), &OpenXRMlMarkerDetector::get_marker_trackers);

	ClassDB::bind_method(D_METHOD("set_settings", "settings"), &OpenXRMlMarkerDetector::set_settings);
	ClassDB::bind_method(D_METHOD("get_settings"), &OpenXRMlMarkerDetector::get_settings);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "settings", PROPERTY_HINT_RESOURCE_TYPE, "OpenXRMlMarkerDetectorSettings"), "set_settings", "get_settings");

	BIND_ENUM_CONSTANT(STATUS_PENDING);
	BIND_ENUM_CONSTANT(STATUS_READY);
	BIND_ENUM_CONSTANT(STATUS_ERROR);
	BIND_ENUM_CONSTANT(STATUS_MAX);
}

OpenXRMlMarkerDetector::Status OpenXRMlMarkerDetector::get_status() const {
	return status;
}

TypedArray<OpenXRMlMarkerTracker> OpenXRMlMarkerDetector::get_marker_trackers() const {
	return marker_trackers;
}

void OpenXRMlMarkerDetector::set_settings(const Ref<OpenXRMlMarkerDetectorSettings> &p_settings) {
	if (settings == p_settings)
		return;

	_destroy_detector();
	detector_settings_dirty = true;
	settings = p_settings;
}

Ref<OpenXRMlMarkerDetectorSettings> OpenXRMlMarkerDetector::get_settings() const {
	return settings;
}

void OpenXRMlMarkerDetector::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			OpenXRMlMarkerUnderstandingExtension *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtension::get_singleton();
			if (marker_understanding_extension) {
				marker_understanding_extension->connect("updating_marker_detectors", callable_mp(this, &OpenXRMlMarkerDetector::_on_updating_marker_detectors));
			}

			_create_detector();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_destroy_detector();

			OpenXRMlMarkerUnderstandingExtension *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtension::get_singleton();
			if (marker_understanding_extension) {
				marker_understanding_extension->disconnect("updating_marker_detectors", callable_mp(this, &OpenXRMlMarkerDetector::_on_updating_marker_detectors));
			}
		} break;
	}
}

void OpenXRMlMarkerDetector::_on_updating_marker_detectors() {
	if (detector_settings_dirty) {
		_destroy_detector(); // Just for safety. The detector should already be destroyed
		_create_detector();
	}
	_update_detector();
}

void OpenXRMlMarkerDetector::_update_detector() {
	if (marker_detector == XR_NULL_HANDLE)
		return;

	OpenXRMlMarkerUnderstandingExtension *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtension::get_singleton();

	// Snapshot the detector
	if (!has_snapshot) {
		if (marker_understanding_extension->snapshot_marker_detector(marker_detector)) {
			has_snapshot = true;
		}
	}

	// Wait for the snapshot to be ready
	status = (OpenXRMlMarkerDetector::Status)marker_understanding_extension->get_marker_detector_status(marker_detector);
	if (status != OpenXRMlMarkerDetector::Status::STATUS_READY) {
		return;
	}

	// Get the marker handles
	has_snapshot = false;
	std::vector<XrMarkerML> marker_atoms;
	marker_understanding_extension->get_markers(marker_detector, marker_atoms);

	// Update the markers we received and create the ones that are new
	for (const XrMarkerML marker_handle : marker_atoms) {
		Ref<OpenXRMlMarkerTracker> marker = get_or_create_marker_tracker_by_atom(marker_handle);
		marker->update_marker();
	}

	// Remove obsolete markers
	XRServer *xr_server = XRServer::get_singleton();
	for (int i = marker_trackers.size() - 1; i >= 0; --i) {
		Ref<OpenXRMlMarkerTracker> tracker = marker_trackers.get(i);
		XrMarkerML handle = tracker->get_marker_atom();

		bool still_exists = std::find(marker_atoms.begin(), marker_atoms.end(), handle) != marker_atoms.end();
		if (!still_exists) {
			marker_trackers.remove_at(i);
			if (xr_server) {
				xr_server->remove_tracker(tracker);
			}
		}
	}
}

Ref<OpenXRMlMarkerTracker> OpenXRMlMarkerDetector::get_or_create_marker_tracker_by_atom(XrMarkerML p_marker_atom) {
	for (int i = 0; i < marker_trackers.size(); i++) {
		Ref<OpenXRMlMarkerTracker> marker = marker_trackers.get(i);
		if (marker->get_marker_atom() == p_marker_atom) {
			return marker;
		}
	}
	OpenXRMlMarkerTracker *tracker = memnew(OpenXRMlMarkerTracker(marker_detector, p_marker_atom, settings->get_marker_type()));
	tracker->set_tracker_name("/ml_marker/" + String::num_int64(next_marker_id));
	++next_marker_id;
	marker_trackers.push_back(tracker);
	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server) {
		XRServer::get_singleton()->add_tracker(tracker);
	}
	return tracker;
}

void OpenXRMlMarkerDetector::_on_settings_changed() {
	_destroy_detector();
	detector_settings_dirty = true;
}

void OpenXRMlMarkerDetector::_destroy_detector() {
	if (marker_detector == XR_NULL_HANDLE)
		return;

	settings->disconnect("changed", callable_mp(this, &OpenXRMlMarkerDetector::_on_settings_changed));
	if (active_profile_settings.is_valid()) {
		active_profile_settings->disconnect("changed", callable_mp(this, &OpenXRMlMarkerDetector::_on_settings_changed));
	}

	OpenXRMlMarkerUnderstandingExtension *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtension::get_singleton();
	if (marker_understanding_extension) {
		marker_understanding_extension->destroy_marker_detector(marker_detector);
	}
	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server) {
		for (int i = 0; i < marker_trackers.size(); i++) {
			Ref<OpenXRMlMarkerTracker> marker = marker_trackers.get(i);
			XRServer::get_singleton()->remove_tracker(marker);
		}
	}
	marker_trackers.clear();
	marker_detector = XR_NULL_HANDLE;
}

void OpenXRMlMarkerDetector::_create_detector() {
	detector_settings_dirty = false;
	if (!settings.is_valid())
		return;

	OpenXRMlMarkerUnderstandingExtension *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtension::get_singleton();
	if (!marker_understanding_extension)
		return;

	if (!marker_understanding_extension->is_marker_understanding_supported())
		return;

	XrMarkerDetectorCreateInfoML info = {
		XR_TYPE_MARKER_DETECTOR_CREATE_INFO_ML, // type
		nullptr, // next
		(XrMarkerDetectorProfileML)settings->get_marker_detector_profile(), // profile
		(XrMarkerTypeML)settings->get_marker_type(), // marker type
	};
	const void **chain = &info.next;

	XrMarkerDetectorCustomProfileInfoML profile_info;
	XrMarkerDetectorSizeInfoML size_info;
	XrMarkerDetectorArucoInfoML aruco_info;
	XrMarkerDetectorAprilTagInfoML april_tag_info;

	if (settings->get_marker_detector_profile() == OpenXRMlMarkerDetectorSettings::MarkerDetectorProfile::MARKER_DETECTOR_PROFILE_CUSTOM && settings->get_marker_detector_profile_settings().is_valid()) {
		active_profile_settings = settings->get_marker_detector_profile_settings(); // Store the active profile settings so that we can call disconnect("changed", ...) on the same instance
		profile_info = {
			XR_TYPE_MARKER_DETECTOR_CUSTOM_PROFILE_INFO_ML,
			nullptr,
			(XrMarkerDetectorFpsML)active_profile_settings->get_fps_hint(),
			(XrMarkerDetectorResolutionML)active_profile_settings->get_resolution(),
			(XrMarkerDetectorCameraML)active_profile_settings->get_camera(),
			(XrMarkerDetectorCornerRefineMethodML)active_profile_settings->get_corner_refine_method(),
			(XrBool32)active_profile_settings->get_use_edge_refinement(),
			(XrMarkerDetectorFullAnalysisIntervalML)active_profile_settings->get_full_analysis_interval(),
		};
		*chain = &profile_info;
		chain = &profile_info.next;
	} else {
		active_profile_settings = Ref<OpenXRMlMarkerDetectorProfileSettings>();
	}

	if (settings->get_marker_type() == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_ARUCO) {
		OpenXRMlMarkerDetectorArucoSettings *aruco_settings = Object::cast_to<OpenXRMlMarkerDetectorArucoSettings>(settings.ptr());
		ERR_FAIL_NULL(aruco_settings);

		aruco_info = {
			XR_TYPE_MARKER_DETECTOR_ARUCO_INFO_ML,
			nullptr,
			(XrMarkerArucoDictML)aruco_settings->get_aruco_dictionary()
		};
		*chain = &aruco_info;
		chain = &aruco_info.next;

		if (!aruco_settings->get_estimate_aruco_length()) {
			size_info = {
				XR_TYPE_MARKER_DETECTOR_SIZE_INFO_ML,
				nullptr,
				aruco_settings->get_aruco_length()
			};
			*chain = &size_info;
			chain = &size_info.next;
		}
	}

	if (settings->get_marker_type() == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_APRIL_TAG) {
		OpenXRMlMarkerDetectorAprilTagSettings *april_tag_settings = Object::cast_to<OpenXRMlMarkerDetectorAprilTagSettings>(settings.ptr());
		ERR_FAIL_NULL(april_tag_settings);

		april_tag_info = {
			XR_TYPE_MARKER_DETECTOR_APRIL_TAG_INFO_ML,
			nullptr,
			(XrMarkerAprilTagDictML)april_tag_settings->get_april_tag_dictionary()
		};
		*chain = &april_tag_info;
		chain = &april_tag_info.next;

		if (!april_tag_settings->get_estimate_april_tag_length()) {
			size_info = {
				XR_TYPE_MARKER_DETECTOR_SIZE_INFO_ML,
				nullptr,
				april_tag_settings->get_april_tag_length()
			};
			*chain = &size_info;
			chain = &size_info.next;
		}
	}

	if (settings->get_marker_type() == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_QR) {
		OpenXRMlMarkerDetectorQrSettings *qr_settings = Object::cast_to<OpenXRMlMarkerDetectorQrSettings>(settings.ptr());
		ERR_FAIL_NULL(qr_settings);

		if (!qr_settings->get_estimate_qr_length()) {
			size_info = {
				XR_TYPE_MARKER_DETECTOR_SIZE_INFO_ML,
				nullptr,
				qr_settings->get_qr_length()
			};
			*chain = &size_info;
			chain = &size_info.next;
		}
	}

	marker_detector = marker_understanding_extension->create_marker_detector(&info);
	has_snapshot = false;
	if (marker_detector != XR_NULL_HANDLE) {
		settings->connect("changed", callable_mp(this, &OpenXRMlMarkerDetector::_on_settings_changed));
		if (active_profile_settings.is_valid()) {
			active_profile_settings->connect("changed", callable_mp(this, &OpenXRMlMarkerDetector::_on_settings_changed));
		}
	}
}
