/**************************************************************************/
/*  openxr_fb_hand_tracking_capsules_extension_wrapper.cpp                */
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

#include "extensions/openxr_fb_hand_tracking_capsules_extension_wrapper.h"

using namespace godot;

OpenXRFbHandTrackingCapsulesExtensionWrapper *OpenXRFbHandTrackingCapsulesExtensionWrapper::singleton = nullptr;

OpenXRFbHandTrackingCapsulesExtensionWrapper *OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbHandTrackingCapsulesExtensionWrapper());
	}
	return singleton;
}

OpenXRFbHandTrackingCapsulesExtensionWrapper::OpenXRFbHandTrackingCapsulesExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbHandTrackingCapsulesExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_HAND_TRACKING_CAPSULES_EXTENSION_NAME] = &fb_hand_tracking_capsules_ext;
	singleton = this;
}

OpenXRFbHandTrackingCapsulesExtensionWrapper::~OpenXRFbHandTrackingCapsulesExtensionWrapper() {
	cleanup();
}

void OpenXRFbHandTrackingCapsulesExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbHandTrackingCapsulesExtensionWrapper::is_enabled);
}

void OpenXRFbHandTrackingCapsulesExtensionWrapper::cleanup() {
	fb_hand_tracking_capsules_ext = false;
}

godot::Dictionary OpenXRFbHandTrackingCapsulesExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbHandTrackingCapsulesExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

uint64_t OpenXRFbHandTrackingCapsulesExtensionWrapper::_set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) {
	if (!fb_hand_tracking_capsules_ext) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	capsules_state[p_hand_index].type = XR_TYPE_HAND_TRACKING_CAPSULES_STATE_FB;
	capsules_state[p_hand_index].next = p_next_pointer;

	return reinterpret_cast<uint64_t>(&capsules_state[p_hand_index]);
}

XrHandCapsuleFB *OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsules(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_enabled(), nullptr, "OpenXR extension XR_FB_hand_tracking_capsules is not available");
	return capsules_state[p_hand_index].capsules;
}
