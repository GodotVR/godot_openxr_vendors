/**************************************************************************/
/*  openxr_ml_marker_understanding_extension_wrapper.cpp                  */
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

#include "extensions/openxr_ml_marker_understanding_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

OpenXRMlMarkerUnderstandingExtensionWrapper *OpenXRMlMarkerUnderstandingExtensionWrapper::singleton = nullptr;

OpenXRMlMarkerUnderstandingExtensionWrapper *OpenXRMlMarkerUnderstandingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMlMarkerUnderstandingExtensionWrapper());
	}
	return singleton;
}

OpenXRMlMarkerUnderstandingExtensionWrapper::OpenXRMlMarkerUnderstandingExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMlMarkerUnderstandingExtensionWrapper singleton already exists.");

	request_extensions[XR_ML_MARKER_UNDERSTANDING_EXTENSION_NAME] = &ml_marker_understanding_ext;

	singleton = this;
}

OpenXRMlMarkerUnderstandingExtensionWrapper::~OpenXRMlMarkerUnderstandingExtensionWrapper() {
	cleanup();
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_marker_understanding_supported"), &OpenXRMlMarkerUnderstandingExtensionWrapper::is_marker_understanding_supported);

	ADD_SIGNAL(MethodInfo("updating_marker_detectors"));
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::cleanup() {
	ml_marker_understanding_ext = false;
}

godot::Dictionary OpenXRMlMarkerUnderstandingExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (ml_marker_understanding_ext) {
		bool result = initialize_ml_marker_understanding_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize ml_marker_understanding extension");
			ml_marker_understanding_ext = false;
		}
	}
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::_on_process() {
	if (!is_enabled()) {
		return;
	}

	// Get the next frame time
	const XrTime display_time = get_openxr_api()->get_predicted_display_time();
	if (display_time == 0) {
		return;
	}

	emit_signal("updating_marker_detectors");
}

XrMarkerDetectorML OpenXRMlMarkerUnderstandingExtensionWrapper::create_marker_detector(const XrMarkerDetectorCreateInfoML *p_create_info) {
	XrMarkerDetectorML marker_detector = XR_NULL_HANDLE;

	XrResult result = xrCreateMarkerDetectorML(SESSION, p_create_info, &marker_detector);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrCreateMarkerDetectorML failed: %s", get_openxr_api()->get_error_string(result)));
		return XR_NULL_HANDLE;
	}

	return marker_detector;
}

bool OpenXRMlMarkerUnderstandingExtensionWrapper::snapshot_marker_detector(XrMarkerDetectorML p_marker_detector) {
	XrMarkerDetectorSnapshotInfoML snapshot_info = {
		XR_TYPE_MARKER_DETECTOR_SNAPSHOT_INFO_ML,
		nullptr,
	};
	XrResult result = xrSnapshotMarkerDetectorML(p_marker_detector, &snapshot_info);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrSnapshotMarkerDetectorML failed: %s", get_openxr_api()->get_error_string(result)));
		return false;
	}
	return true;
}

XrMarkerDetectorStatusML OpenXRMlMarkerUnderstandingExtensionWrapper::get_marker_detector_status(XrMarkerDetectorML p_marker_detector) {
	XrMarkerDetectorStateML state = {
		XR_TYPE_MARKER_DETECTOR_STATE_ML,
		nullptr,
		XR_MARKER_DETECTOR_STATUS_PENDING_ML,
	};
	XrResult result = xrGetMarkerDetectorStateML(p_marker_detector, &state);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkerDetectorStateML failed: %s", get_openxr_api()->get_error_string(result)));
		return XR_MARKER_DETECTOR_STATUS_ERROR_ML;
	}
	return state.state;
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::get_markers(XrMarkerDetectorML p_marker_detector, std::vector<XrMarkerML> &markers) {
	markers.clear();
	uint32_t marker_count = 0;
	XrResult result = xrGetMarkersML(p_marker_detector, 0, &marker_count, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkersML (count query) failed: %s", get_openxr_api()->get_error_string(result)));
	}
	if (marker_count == 0)
		return; // No need to get 0 markers

	markers.resize(marker_count);
	uint32_t output_count = 0;
	result = xrGetMarkersML(p_marker_detector, marker_count, &output_count, &markers.front());
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkersML failed: %s", get_openxr_api()->get_error_string(result)));
	}
	if (marker_count != output_count) {
		UtilityFunctions::printerr(vformat("xrGetMarkersML did not return the number of elements it said it has. Expected: %d Received: %d", marker_count, output_count));
	}
}

float OpenXRMlMarkerUnderstandingExtensionWrapper::get_marker_reprojection_error(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker) {
	float reprojection_error_meters = 0;
	XrResult result = xrGetMarkerReprojectionErrorML(p_marker_detector, p_marker, &reprojection_error_meters);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkerReprojectionErrorML failed: %s", get_openxr_api()->get_error_string(result)));
	}
	return reprojection_error_meters;
}

float OpenXRMlMarkerUnderstandingExtensionWrapper::get_marker_length(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker) {
	float length_meters = 0;
	XrResult result = xrGetMarkerLengthML(p_marker_detector, p_marker, &length_meters);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkerLengthML failed: %s", get_openxr_api()->get_error_string(result)));
	}
	return length_meters;
}

uint64_t OpenXRMlMarkerUnderstandingExtensionWrapper::get_marker_number(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker) {
	uint64_t marker_number = 0;
	XrResult result = xrGetMarkerNumberML(p_marker_detector, p_marker, &marker_number);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkerNumberML failed: %s", get_openxr_api()->get_error_string(result)));
	}
	return marker_number;
}

String OpenXRMlMarkerUnderstandingExtensionWrapper::get_marker_string(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker) {
	uint32_t string_length = 0; // xrGetMarkerStringML will include the NULL terminator in the string_length
	XrResult result = xrGetMarkerStringML(p_marker_detector, p_marker, 0, &string_length, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkerStringML (count query) failed: %s", get_openxr_api()->get_error_string(result)));
		return "";
	}
	if (string_length == 0)
		return ""; // No need to get 0 length string

	CharString characters;
	characters.resize(string_length);
	uint32_t output_count = 0;
	result = xrGetMarkerStringML(p_marker_detector, p_marker, string_length, &output_count, characters.ptrw()); // xrGetMarkerStringML will include the NULL terminator in the output_count and in the buffer
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetMarkerStringML failed: %s", get_openxr_api()->get_error_string(result)));
		return "";
	}
	if (string_length != output_count) {
		UtilityFunctions::printerr(vformat("xrGetMarkerStringML did not return the number of characters it said it has. Expected: %d Received: %d", string_length, output_count));
		return "";
	}
	return String::utf8(characters, output_count - 1); // 1 byte less because of the NULL terminator being included in the output_count
}

XrSpace OpenXRMlMarkerUnderstandingExtensionWrapper::create_marker_space(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker) {
	XrMarkerSpaceCreateInfoML create_info = {
		XR_TYPE_MARKER_SPACE_CREATE_INFO_ML,
		nullptr,
		p_marker_detector,
		p_marker,
		{
				{ 0.0, 0.0, 0.0, 1.0 }, // orientation
				{ 0.0, 0.0, 0.0 } // position
		} // pose
	};
	XrSpace space = XR_NULL_HANDLE;
	XrResult result = xrCreateMarkerSpaceML(SESSION, &create_info, &space);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrCreateMarkerSpaceML failed: %s", get_openxr_api()->get_error_string(result)));
	}
	return space;
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::update_tracker_pose(XrSpace p_space, Ref<XRPositionalTracker> &p_positional_tracker) {
	XrSpaceLocation space_location = {
		XR_TYPE_SPACE_LOCATION,
		nullptr,
		0, // locationFlags
		{
				{ 0.0, 0.0, 0.0, 0.0 }, // orientation
				{ 0.0, 0.0, 0.0 } // position
		} // pose
	};
	XrResult result = xrLocateSpace(p_space, (XrSpace)get_openxr_api()->get_play_space(), get_openxr_api()->get_predicted_display_time(), &space_location);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrLocateSpace failed: %s", get_openxr_api()->get_error_string(result)));
		return;
	}
	if ((space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) && (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
		Transform3D transform(
				Basis(Quaternion(space_location.pose.orientation.x, space_location.pose.orientation.y, space_location.pose.orientation.z, space_location.pose.orientation.w)),
				Vector3(space_location.pose.position.x, space_location.pose.position.y, space_location.pose.position.z));

		p_positional_tracker->set_pose("default", transform, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);
	} else {
		Ref<XRPose> default_pose = p_positional_tracker->get_pose("default");
		if (default_pose.is_valid()) {
			// Set the tracking confidence to none, while maintaining the existing transform.
			default_pose->set_tracking_confidence(XRPose::XR_TRACKING_CONFIDENCE_NONE);
		} else {
			p_positional_tracker->set_pose("default", Transform3D(), Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_NONE);
		}
	}
}

void OpenXRMlMarkerUnderstandingExtensionWrapper::destroy_marker_detector(XrMarkerDetectorML p_marker_detector) {
	XrResult result = xrDestroyMarkerDetectorML(p_marker_detector);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrDestroyMarkerDetectorML failed: %s", get_openxr_api()->get_error_string(result)));
	}
}

bool OpenXRMlMarkerUnderstandingExtensionWrapper::is_enabled() const {
	return ml_marker_understanding_ext;
}

bool OpenXRMlMarkerUnderstandingExtensionWrapper::initialize_ml_marker_understanding_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateMarkerDetectorML);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyMarkerDetectorML);
	GDEXTENSION_INIT_XR_FUNC_V(xrSnapshotMarkerDetectorML);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetMarkerDetectorStateML);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetMarkersML);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetMarkerReprojectionErrorML);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetMarkerLengthML);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetMarkerNumberML);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateMarkerSpaceML);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetMarkerStringML);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateSpace);

	return true;
}
