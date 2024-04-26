/**************************************************************************/
/*  openxr_fb_body_tracking_extension_wrapper.h                           */
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

#ifndef OPENXR_FB_BODY_TRACKING_EXTENSION_WRAPPER_H
#define OPENXR_FB_BODY_TRACKING_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_body_tracker.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook body tracking extension.
class OpenXRFbBodyTrackingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbBodyTrackingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_session_created(uint64_t instance) override;

	void _on_session_destroyed() override;

	void _on_process() override;

	static OpenXRFbBodyTrackingExtensionWrapper *get_singleton();

	bool is_enabled() const;

	OpenXRFbBodyTrackingExtensionWrapper();
	~OpenXRFbBodyTrackingExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateBodyTrackerFB,
			(XrSession), session,
			(const XrBodyTrackerCreateInfoFB *), createInfo,
			(XrBodyTrackerFB *), bodyTracker);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyBodyTrackerFB,
			(XrBodyTrackerFB), bodyTracker);

	EXT_PROTO_XRRESULT_FUNC3(xrLocateBodyJointsFB,
			(XrBodyTrackerFB), bodyTracker,
			(const XrBodyJointsLocateInfoFB *), locateInfo,
			(XrBodyJointLocationsFB *), locations);

	bool initialize_fb_body_tracking_extension(const XrInstance instance);

	void cleanup();

	static OpenXRFbBodyTrackingExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool fb_body_tracking_ext = false;

	bool xr_body_tracker_registered = false;

	// OpenXR system properties struct for XR_FB_body_tracking.
	XrSystemBodyTrackingPropertiesFB system_body_tracking_properties;

	// XR_FB_body_tracking handle.
	XrBodyTrackerFB body_tracker = XR_NULL_HANDLE;

	// Godot XRBodyTracker instance.
	Ref<XRBodyTracker> xr_body_tracker;
};

#endif // OPENXR_FB_BODY_TRACKING_EXTENSION_WRAPPER_H
