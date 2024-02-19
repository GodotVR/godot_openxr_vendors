/**************************************************************************/
/*  openxr_fb_face_tracking_extension_wrapper.h                           */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Original contributed implementation:                                   */
/*   Copyright (c) 2024 Malcolm Nixon                                     */
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

#ifndef OPENXR_FB_FACE_TRACKING_EXTENSION_WRAPPER_H
#define OPENXR_FB_FACE_TRACKING_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_face_tracker.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook face tracking extension.
class OpenXRFbFaceTrackingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbFaceTrackingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_session_created(uint64_t instance) override;

	void _on_session_destroyed() override;

	void _on_process() override;

	static OpenXRFbFaceTrackingExtensionWrapper *get_singleton();

	bool is_enabled() const;

	OpenXRFbFaceTrackingExtensionWrapper();
	~OpenXRFbFaceTrackingExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateFaceTracker2FB,
			(XrSession), session,
			(const XrFaceTrackerCreateInfo2FB *), createInfo,
			(XrFaceTracker2FB *), faceTracker);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyFaceTracker2FB,
			(XrFaceTracker2FB), faceTracker);

	EXT_PROTO_XRRESULT_FUNC3(xrGetFaceExpressionWeights2FB,
			(XrFaceTracker2FB), faceTracker,
			(const XrFaceExpressionInfo2FB *), expressionInfo,
			(XrFaceExpressionWeights2FB *), expressionWeights);

	bool initialize_fb_face_tracking2_extension(const XrInstance instance);

	void cleanup();

	static OpenXRFbFaceTrackingExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool fb_face_tracking2_ext = false;

	bool xr_face_tracker_registered = false;

	// OpenXR system properties struct for XR_FB_face_tracking2.
	XrSystemFaceTrackingProperties2FB system_face_tracking_properties2;

	// XR_FB_face_tracking handle.
	XrFaceTracker2FB face_tracker2 = XR_NULL_HANDLE;

	// Godot XRFaceTracker instance.
	Ref<XRFaceTracker> xr_face_tracker;
};

#endif // OPENXR_FB_FACE_TRACKING_EXTENSION_WRAPPER_H
