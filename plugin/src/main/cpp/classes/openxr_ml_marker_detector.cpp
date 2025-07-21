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
#include "classes/openxr_ml_marker_data.h"
#include "classes/openxr_ml_marker_detector_april_tag_settings.h"
#include "classes/openxr_ml_marker_detector_aruco_settings.h"
#include "classes/openxr_ml_marker_detector_qr_settings.h"
#include "classes/openxr_ml_marker_detector_settings.h"
#include "extensions/openxr_ml_marker_understanding_extension_wrapper.h"

#include <algorithm>
#include <vector>

using namespace godot;

void OpenXRMlMarkerDetector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_status"), &OpenXRMlMarkerDetector::get_status);
	ClassDB::bind_method(D_METHOD("get_marker_data"), &OpenXRMlMarkerDetector::get_marker_data);

	ClassDB::bind_static_method("OpenXRMlMarkerDetector", D_METHOD("create_detector", "marker_detector_settings"), &OpenXRMlMarkerDetector::create_detector);

	BIND_ENUM_CONSTANT(STATUS_PENDING);
	BIND_ENUM_CONSTANT(STATUS_READY);
	BIND_ENUM_CONSTANT(STATUS_ERROR);
	BIND_ENUM_CONSTANT(STATUS_MAX);
}

Ref<OpenXRMlMarkerDetector> OpenXRMlMarkerDetector::create_detector(const Ref<OpenXRMlMarkerDetectorSettings> &p_marker_detector_settings) {
	return Ref<OpenXRMlMarkerDetector>(memnew(OpenXRMlMarkerDetector(p_marker_detector_settings)));
}

OpenXRMlMarkerDetector::Status OpenXRMlMarkerDetector::get_status() const {
	return status;
}

TypedArray<OpenXRMlMarkerData> OpenXRMlMarkerDetector::get_marker_data() const {
	return marker_data;
}

void OpenXRMlMarkerDetector::_on_updating_marker_detectors() {
	OpenXRMlMarkerUnderstandingExtensionWrapper *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtensionWrapper::get_singleton();

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
		Ref<OpenXRMlMarkerData> marker = get_or_create_marker_data_by_atom(marker_handle);
		marker->update_marker();
	}

	// Remove obsolete markers
	for (int i = marker_data.size() - 1; i >= 0; --i) {
		Ref<OpenXRMlMarkerData> marker = marker_data.get(i);
		XrMarkerML handle = marker->get_marker_atom();

		bool still_exists = std::find(marker_atoms.begin(), marker_atoms.end(), handle) != marker_atoms.end();
		if (!still_exists) {
			marker_data.remove_at(i);
		}
	}
}

Ref<OpenXRMlMarkerData> OpenXRMlMarkerDetector::get_or_create_marker_data_by_atom(XrMarkerML p_marker_atom) {
	for (int i = 0; i < marker_data.size(); i++) {
		Ref<OpenXRMlMarkerData> marker = marker_data.get(i);
		if (marker->get_marker_atom() == p_marker_atom) {
			return marker;
		}
	}
	OpenXRMlMarkerData *marker = memnew(OpenXRMlMarkerData(marker_detector, p_marker_atom, marker_type));
	marker_data.push_back(marker);
	return marker;
}

void OpenXRMlMarkerDetector::destroy() {
	ERR_FAIL_COND_MSG(marker_detector == XR_NULL_HANDLE, "Underlying marker detector doesn't exist (yet) or has been destroyed.");
	OpenXRMlMarkerUnderstandingExtensionWrapper *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtensionWrapper::get_singleton();
	if (marker_understanding_extension) {
		marker_understanding_extension->disconnect("updating_marker_detectors", callable_mp(this, &OpenXRMlMarkerDetector::_on_updating_marker_detectors));
		marker_understanding_extension->destroy_marker_detector(marker_detector);
		marker_detector = XR_NULL_HANDLE;
	}
}

OpenXRMlMarkerDetector::OpenXRMlMarkerDetector(const Ref<OpenXRMlMarkerDetectorSettings> &p_marker_detector_settings) {
	ERR_FAIL_NULL(p_marker_detector_settings);
	marker_type = p_marker_detector_settings->get_marker_type();

	OpenXRMlMarkerUnderstandingExtensionWrapper *marker_understanding_extension = OpenXRMlMarkerUnderstandingExtensionWrapper::get_singleton();
	ERR_FAIL_NULL(marker_understanding_extension);

	XrMarkerDetectorCreateInfoML info = {
		XR_TYPE_MARKER_DETECTOR_CREATE_INFO_ML, // type
		nullptr, // next
		(XrMarkerDetectorProfileML)p_marker_detector_settings->get_marker_detector_profile(), // profile
		(XrMarkerTypeML)p_marker_detector_settings->get_marker_type(), // marker type
	};
	const void **chain = &info.next;

	XrMarkerDetectorCustomProfileInfoML profile_info;
	XrMarkerDetectorSizeInfoML size_info;
	XrMarkerDetectorArucoInfoML aruco_info;
	XrMarkerDetectorAprilTagInfoML april_tag_info;

	if (p_marker_detector_settings->get_marker_detector_profile() == OpenXRMlMarkerDetectorSettings::MarkerDetectorProfile::MARKER_DETECTOR_PROFILE_CUSTOM && p_marker_detector_settings->get_marker_detector_profile_settings() != nullptr) {
		profile_info = {
			XR_TYPE_MARKER_DETECTOR_CUSTOM_PROFILE_INFO_ML,
			nullptr,
			(XrMarkerDetectorFpsML)p_marker_detector_settings->get_marker_detector_profile_settings()->get_fps_hint(),
			(XrMarkerDetectorResolutionML)p_marker_detector_settings->get_marker_detector_profile_settings()->get_resolution(),
			(XrMarkerDetectorCameraML)p_marker_detector_settings->get_marker_detector_profile_settings()->get_camera(),
			(XrMarkerDetectorCornerRefineMethodML)p_marker_detector_settings->get_marker_detector_profile_settings()->get_corner_refine_method(),
			(XrBool32)p_marker_detector_settings->get_marker_detector_profile_settings()->get_use_edge_refinement(),
			(XrMarkerDetectorFullAnalysisIntervalML)p_marker_detector_settings->get_marker_detector_profile_settings()->get_full_analysis_interval(),
		};
		*chain = &profile_info;
		chain = &profile_info.next;
	}

	if (p_marker_detector_settings->get_marker_type() == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_ARUCO) {
		OpenXRMlMarkerDetectorArucoSettings *aruco_settings = Object::cast_to<OpenXRMlMarkerDetectorArucoSettings>(p_marker_detector_settings.ptr());
		ERR_FAIL_NULL(aruco_settings);

		aruco_info = {
			XR_TYPE_MARKER_DETECTOR_ARUCO_INFO_ML,
			nullptr,
			(XrMarkerArucoDictML)aruco_settings->get_aruco_dictionary()
		};
		*chain = &aruco_info;
		chain = &aruco_info.next;

		if (aruco_settings->get_estimate_aruco_length()) {
			size_info = {
				XR_TYPE_MARKER_DETECTOR_SIZE_INFO_ML,
				nullptr,
				aruco_settings->get_aruco_length()
			};
			*chain = &size_info;
			chain = &size_info.next;
		}
	}

	if (p_marker_detector_settings->get_marker_type() == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_APRIL_TAG) {
		OpenXRMlMarkerDetectorAprilTagSettings *april_tag_settings = Object::cast_to<OpenXRMlMarkerDetectorAprilTagSettings>(p_marker_detector_settings.ptr());
		ERR_FAIL_NULL(april_tag_settings);

		april_tag_info = {
			XR_TYPE_MARKER_DETECTOR_APRIL_TAG_INFO_ML,
			nullptr,
			(XrMarkerAprilTagDictML)april_tag_settings->get_april_tag_dictionary()
		};
		*chain = &april_tag_info;
		chain = &april_tag_info.next;

		if (april_tag_settings->get_estimate_april_tag_length()) {
			size_info = {
				XR_TYPE_MARKER_DETECTOR_SIZE_INFO_ML,
				nullptr,
				april_tag_settings->get_april_tag_length()
			};
			*chain = &size_info;
			chain = &size_info.next;
		}
	}

	if (p_marker_detector_settings->get_marker_type() == OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_QR) {
		OpenXRMlMarkerDetectorQrSettings *qr_settings = Object::cast_to<OpenXRMlMarkerDetectorQrSettings>(p_marker_detector_settings.ptr());
		ERR_FAIL_NULL(qr_settings);

		if (qr_settings->get_estimate_qr_length()) {
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
	if (marker_detector != nullptr) {
		marker_understanding_extension->connect("updating_marker_detectors", callable_mp(this, &OpenXRMlMarkerDetector::_on_updating_marker_detectors));
	}
}
