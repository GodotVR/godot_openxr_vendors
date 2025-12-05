/**************************************************************************/
/*  openxr_ml_marker_understanding_extension_wrapper.h                    */
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

#ifndef OPENXR_ML_MARKER_UNDERSTANDING_EXTENSION_WRAPPER_H
#define OPENXR_ML_MARKER_UNDERSTANDING_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>
#include <vector>

#include "util.h"

using namespace godot;

// Wrapper for the Magic Leap Marker Understanding extension.
class OpenXRMlMarkerUnderstandingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMlMarkerUnderstandingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_process() override;

	XrMarkerDetectorML create_marker_detector(const XrMarkerDetectorCreateInfoML *p_create_info);

	bool snapshot_marker_detector(XrMarkerDetectorML p_marker_detector);

	void get_markers(XrMarkerDetectorML p_marker_detector, std::vector<XrMarkerML> &markers);

	float get_marker_reprojection_error(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker);

	float get_marker_length(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker);

	uint64_t get_marker_number(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker);

	String get_marker_string(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker);

	XrSpace create_marker_space(XrMarkerDetectorML p_marker_detector, XrMarkerML p_marker);

	void update_tracker_pose(XrSpace p_space, Ref<XRPositionalTracker> &p_positional_tracker);

	void destroy_marker_detector(XrMarkerDetectorML p_marker_detector);

	XrMarkerDetectorStatusML get_marker_detector_status(XrMarkerDetectorML p_marker_detector);

	bool is_marker_understanding_supported() {
		return ml_marker_understanding_ext;
	}

	static OpenXRMlMarkerUnderstandingExtensionWrapper *get_singleton();

	bool is_enabled() const;

	OpenXRMlMarkerUnderstandingExtensionWrapper();
	~OpenXRMlMarkerUnderstandingExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateMarkerDetectorML,
			(XrSession), session,
			(const XrMarkerDetectorCreateInfoML *), createInfo,
			(XrMarkerDetectorML *), markerDetector);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyMarkerDetectorML,
			(XrMarkerDetectorML), markerDetector);

	EXT_PROTO_XRRESULT_FUNC2(xrSnapshotMarkerDetectorML,
			(XrMarkerDetectorML), markerDetector,
			(XrMarkerDetectorSnapshotInfoML *), snapshotInfo);

	EXT_PROTO_XRRESULT_FUNC2(xrGetMarkerDetectorStateML,
			(XrMarkerDetectorML), markerDetector,
			(XrMarkerDetectorStateML *), state);

	EXT_PROTO_XRRESULT_FUNC4(xrGetMarkersML,
			(XrMarkerDetectorML), markerDetector,
			(uint32_t), markerCapacityInput,
			(uint32_t *), markerCountOutput,
			(XrMarkerML *), markers);

	EXT_PROTO_XRRESULT_FUNC3(xrGetMarkerReprojectionErrorML,
			(XrMarkerDetectorML), markerDetector,
			(XrMarkerML), marker,
			(float *), reprojectionErrorMeters);

	EXT_PROTO_XRRESULT_FUNC3(xrGetMarkerLengthML,
			(XrMarkerDetectorML), markerDetector,
			(XrMarkerML), marker,
			(float *), meters);

	EXT_PROTO_XRRESULT_FUNC3(xrGetMarkerNumberML,
			(XrMarkerDetectorML), markerDetector,
			(XrMarkerML), marker,
			(uint64_t *), number);

	EXT_PROTO_XRRESULT_FUNC3(xrCreateMarkerSpaceML,
			(XrSession), session,
			(XrMarkerSpaceCreateInfoML *), createInfo,
			(XrSpace *), space);

	EXT_PROTO_XRRESULT_FUNC5(xrGetMarkerStringML,
			(XrMarkerDetectorML), markerDetector,
			(XrMarkerML), marker,
			(uint32_t), bufferCapacityInput,
			(uint32_t *), bufferCountOutput,
			(char *), buffer);

	EXT_PROTO_XRRESULT_FUNC4(xrLocateSpace,
			(XrSpace), space,
			(XrSpace), baseSpace,
			(XrTime), time,
			(XrSpaceLocation *), location)

	bool initialize_ml_marker_understanding_extension(const XrInstance instance);

	void cleanup();

	static OpenXRMlMarkerUnderstandingExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool ml_marker_understanding_ext = false;
};

#endif
