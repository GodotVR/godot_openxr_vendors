/**************************************************************************/
/*  openxr_android_eye_tracking_extension_wrapper.h                       */
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

#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/classes/xr_controller_tracker.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

class OpenXRAndroidEyeTrackingExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidEyeTrackingExtension, OpenXRExtensionWrapper);

public:
	static OpenXRAndroidEyeTrackingExtension *get_singleton();

	OpenXRAndroidEyeTrackingExtension();
	virtual ~OpenXRAndroidEyeTrackingExtension() override;

	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	virtual void _on_instance_created(uint64_t p_instance) override;

	void _on_session_created(uint64_t instance) override;
	virtual void _on_session_destroyed() override;

	void _on_state_ready() override;

	void _on_process() override;

	void _on_request_permissions_result(const String &p_permission, bool p_granted);

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateEyeTrackerANDROID,
			(XrSession), session,
			(const XrEyeTrackerCreateInfoANDROID *), createInfo,
			(XrEyeTrackerANDROID *), eyeTracker);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyEyeTrackerANDROID,
			(XrEyeTrackerANDROID), eyeTracker);

	EXT_PROTO_XRRESULT_FUNC3(xrGetCoarseTrackingEyesInfoANDROID,
			(XrEyeTrackerANDROID), eyeTracker,
			(const XrEyesGetInfoANDROID *), getInfo,
			(XrEyesANDROID *), eyesOutput);

	bool _initialize_openxr_android_eye_tracking_extension();
	void _try_create_eye_tracker();

	void _populate_eye_tracker(XRControllerTracker *tracker, bool tracking_status, const XrEyeANDROID &xr_eye);

	static OpenXRAndroidEyeTrackingExtension *singleton;

	Callable on_request_permissions_result_callable;

	HashMap<String, bool *> request_extensions;
	bool available = false;

	XrSystemEyeTrackingPropertiesANDROID eye_tracking_properties;
	XrEyeTrackerANDROID eye_tracker = XR_NULL_HANDLE;

	Ref<XRControllerTracker> xr_eye_tracker[2];
};
