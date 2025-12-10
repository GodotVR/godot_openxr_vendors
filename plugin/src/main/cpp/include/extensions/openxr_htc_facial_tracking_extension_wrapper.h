/**************************************************************************/
/*  openxr_htc_facial_tracking_extension_wrapper.h                        */
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

#ifndef OPENXR_HTC_FACIAL_TRACKING_EXTENSION_WRAPPER_H
#define OPENXR_HTC_FACIAL_TRACKING_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_face_tracker.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the HTC facial tracking extension.
class OpenXRHtcFacialTrackingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRHtcFacialTrackingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_session_created(uint64_t instance) override;

	void _on_session_destroyed() override;

	void _on_process() override;

	static OpenXRHtcFacialTrackingExtensionWrapper *get_singleton();

	bool is_enabled() const;

	OpenXRHtcFacialTrackingExtensionWrapper();
	~OpenXRHtcFacialTrackingExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateFacialTrackerHTC,
			(XrSession), session,
			(const XrFacialTrackerCreateInfoHTC *), createInfo,
			(XrFacialTrackerHTC *), facialTracker);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyFacialTrackerHTC,
			(XrFacialTrackerHTC), facialTracker);

	EXT_PROTO_XRRESULT_FUNC2(xrGetFacialExpressionsHTC,
			(XrFacialTrackerHTC), facialTracker,
			(XrFacialExpressionsHTC *), facialExpressions);

	bool initialize_htc_facial_tracking_extension(const XrInstance instance);

	void cleanup();

	static OpenXRHtcFacialTrackingExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool htc_facial_tracking_ext = false;

	bool xr_face_tracker_registered = false;

	// OpenXR system properties struct for XR_HTC_facial_tracking.
	XrSystemFacialTrackingPropertiesHTC system_facial_tracking_properties;

	// XR_HTC_facial_tracking handle for eye-tracking
	XrFacialTrackerHTC facial_tracking_eye = XR_NULL_HANDLE;

	// XR_HTC_facial_tracking handle for lip-tracking
	XrFacialTrackerHTC facial_tracking_lip = XR_NULL_HANDLE;

	// Godot XRFaceTracker instance.
	Ref<XRFaceTracker> xr_face_tracker;
};

#endif // OPENXR_HTC_FACIAL_TRACKING_EXTENSION_WRAPPER_H
