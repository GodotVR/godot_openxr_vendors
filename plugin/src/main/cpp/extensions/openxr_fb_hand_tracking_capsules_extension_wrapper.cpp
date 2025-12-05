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

#include <godot_cpp/classes/project_settings.hpp>

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
	ClassDB::bind_method(D_METHOD("get_hand_capsule_count"), &OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_count);
	ClassDB::bind_method(D_METHOD("get_hand_capsule_transform", "hand_index", "capsule_index"), &OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_transform);
	ClassDB::bind_method(D_METHOD("get_hand_capsule_height", "hand_index", "capsule_index"), &OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_height);
	ClassDB::bind_method(D_METHOD("get_hand_capsule_radius", "hand_index", "capsule_index"), &OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_radius);
	ClassDB::bind_method(D_METHOD("get_hand_capsule_joint", "hand_index", "capsule_index"), &OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_joint);
}

void OpenXRFbHandTrackingCapsulesExtensionWrapper::cleanup() {
	fb_hand_tracking_capsules_ext = false;
}

godot::Dictionary OpenXRFbHandTrackingCapsulesExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
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

	capsules_state[p_hand_index] = {
		XR_TYPE_HAND_TRACKING_CAPSULES_STATE_FB, // type
		p_next_pointer, // next
	};

	return reinterpret_cast<uint64_t>(&capsules_state[p_hand_index]);
}

Transform3D OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_transform(int p_hand_index, int p_capsule_index) const {
	ERR_FAIL_INDEX_V_MSG(p_hand_index, HAND_MAX, Transform3D(), vformat("Invalid hand index %d", p_hand_index));
	ERR_FAIL_INDEX_V_MSG(p_capsule_index, XR_FB_HAND_TRACKING_CAPSULE_COUNT, Transform3D(), vformat("Invalid capsule index %d", p_capsule_index));

	if (!fb_hand_tracking_capsules_ext) {
		return Transform3D();
	}

	XrHandCapsuleFB capsule = capsules_state[p_hand_index].capsules[p_capsule_index];

	XrVector3f xr_p1 = capsule.points[0];
	XrVector3f xr_p2 = capsule.points[1];
	Vector3 p1 = Vector3(xr_p1.x, xr_p1.y, xr_p1.z);
	Vector3 p2 = Vector3(xr_p2.x, xr_p2.y, xr_p2.z);

	Vector3 up_dir = Vector3(0, 1, 0);
	Vector3 right_dir = Vector3(1, 0, 0);

	Vector3 y_dir = (p2 - p1).normalized();
	Vector3 x_dir = (y_dir.is_equal_approx(up_dir)) ? y_dir.cross(right_dir).normalized() : y_dir.cross(up_dir).normalized();
	Vector3 z_dir = y_dir.cross(x_dir).normalized();
	Basis basis = Basis(x_dir, y_dir, z_dir);
	Vector3 center = (p1 + p2) * 0.5;

	float height = p1.distance_to(p2) + (capsule.radius * 2.0);
	Transform3D transform = Transform3D(basis, center);

	return Transform3D(basis, center);
}

float OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_height(int p_hand_index, int p_capsule_index) const {
	ERR_FAIL_INDEX_V_MSG(p_hand_index, HAND_MAX, 0.0, vformat("Invalid hand index %d", p_hand_index));
	ERR_FAIL_INDEX_V_MSG(p_capsule_index, XR_FB_HAND_TRACKING_CAPSULE_COUNT, 0.0, vformat("Invalid capsule index %d", p_capsule_index));

	if (!fb_hand_tracking_capsules_ext) {
		return 0.0;
	}

	XrHandCapsuleFB capsule = capsules_state[p_hand_index].capsules[p_capsule_index];

	XrVector3f xr_p1 = capsule.points[0];
	XrVector3f xr_p2 = capsule.points[1];
	Vector3 p1 = Vector3(xr_p1.x, xr_p1.y, xr_p1.z);
	Vector3 p2 = Vector3(xr_p2.x, xr_p2.y, xr_p2.z);

	return p1.distance_to(p2) + (capsule.radius * 2.0);
}

float OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_radius(int p_hand_index, int p_capsule_index) const {
	ERR_FAIL_INDEX_V_MSG(p_hand_index, HAND_MAX, 0.0, vformat("Invalid hand index %d", p_hand_index));
	ERR_FAIL_INDEX_V_MSG(p_capsule_index, XR_FB_HAND_TRACKING_CAPSULE_COUNT, 0.0, vformat("Invalid capsule index %d", p_capsule_index));

	if (!fb_hand_tracking_capsules_ext) {
		return 0.0;
	}

	XrHandCapsuleFB capsule = capsules_state[p_hand_index].capsules[p_capsule_index];
	return capsule.radius;
}

XRHandTracker::HandJoint OpenXRFbHandTrackingCapsulesExtensionWrapper::get_hand_capsule_joint(int p_hand_index, int p_capsule_index) const {
	ERR_FAIL_INDEX_V_MSG(p_hand_index, HAND_MAX, HandJoint(0), vformat("Invalid hand index %d", p_hand_index));
	ERR_FAIL_INDEX_V_MSG(p_capsule_index, XR_FB_HAND_TRACKING_CAPSULE_COUNT, HandJoint(0), vformat("Invalid capsule index %d", p_capsule_index));

	if (!fb_hand_tracking_capsules_ext) {
		return HandJoint(0);
	}

	XrHandCapsuleFB capsule = capsules_state[p_hand_index].capsules[p_capsule_index];
	return HandJoint(capsule.joint);
}
