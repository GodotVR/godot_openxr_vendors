/**************************************************************************/
/*  openxr_fb_hand_tracking_aim_extension_wrapper.h                       */
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

#ifndef OPENXR_FB_HAND_TRACKING_AIM_EXTENSION_WRAPPER_H
#define OPENXR_FB_HAND_TRACKING_AIM_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>

#include <map>

using namespace godot;

// Wrapper for the set of Facebook XR hand tracking aim extension.
class OpenXRFbHandTrackingAimExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbHandTrackingAimExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	using Hand = OpenXRInterface::Hand;

	godot::Dictionary _get_requested_extensions() override;

	PackedStringArray _get_suggested_tracker_names() override;

	void _on_state_ready() override;

	void _on_instance_destroyed() override;

	uint64_t _set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) override;

	bool is_enabled() {
		return fb_hand_tracking_aim_ext;
	}

	void _on_process() override;

	void add_project_setting();

	static OpenXRFbHandTrackingAimExtensionWrapper *get_singleton();

	OpenXRFbHandTrackingAimExtensionWrapper();
	~OpenXRFbHandTrackingAimExtensionWrapper();

protected:
	static void _bind_methods();

private:
	std::map<godot::String, bool *> request_extensions;

	void cleanup();

	static OpenXRFbHandTrackingAimExtensionWrapper *singleton;

	const String TRACKER_NAME_LEFT = "/user/fbhandaim/left";
	const String TRACKER_NAME_RIGHT = "/user/fbhandaim/right";

	bool fb_hand_tracking_aim_ext = false;
	bool is_project_setting_enabled = false;

	Ref<XRPositionalTracker> trackers[Hand::HAND_MAX];

	XrHandTrackingAimStateFB aim_state[Hand::HAND_MAX];
};

#endif // OPENXR_FB_HAND_TRACKING_AIM_EXTENSION_WRAPPER_H
