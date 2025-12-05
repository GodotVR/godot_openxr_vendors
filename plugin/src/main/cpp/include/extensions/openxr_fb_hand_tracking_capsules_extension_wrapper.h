/**************************************************************************/
/*  openxr_fb_hand_tracking_capsules_extension_wrapper.h                  */
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

#ifndef OPENXR_FB_HAND_TRACKING_CAPSULES_EXTENSION_WRAPPER_H
#define OPENXR_FB_HAND_TRACKING_CAPSULES_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_hand_tracker.hpp>
#include <map>

using namespace godot;

// Wrapper for the set of Facebook XR hand tracking capsules extension.
class OpenXRFbHandTrackingCapsulesExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbHandTrackingCapsulesExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	using HandJoint = XRHandTracker::HandJoint;

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_destroyed() override;

	uint64_t _set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) override;

	bool is_enabled() {
		return fb_hand_tracking_capsules_ext;
	}

	int get_hand_capsule_count() const { return XR_HAND_TRACKING_CAPSULE_COUNT_FB; }
	Transform3D get_hand_capsule_transform(int p_hand_index, int p_capsule_index) const;
	float get_hand_capsule_height(int p_hand_index, int p_capsule_index) const;
	float get_hand_capsule_radius(int p_hand_index, int p_capsule_index) const;
	HandJoint get_hand_capsule_joint(int p_hand_index, int p_capsule_index) const;

	static OpenXRFbHandTrackingCapsulesExtensionWrapper *get_singleton();

	OpenXRFbHandTrackingCapsulesExtensionWrapper();
	~OpenXRFbHandTrackingCapsulesExtensionWrapper();

protected:
	static void _bind_methods();

private:
	std::map<godot::String, bool *> request_extensions;

	void cleanup();

	static OpenXRFbHandTrackingCapsulesExtensionWrapper *singleton;

	bool fb_hand_tracking_capsules_ext = false;

	static const int HAND_MAX = 2;
	XrHandTrackingCapsulesStateFB capsules_state[HAND_MAX];
};

#endif // OPENXR_FB_HAND_TRACKING_CAPSULES_EXTENSION_WRAPPER_H
