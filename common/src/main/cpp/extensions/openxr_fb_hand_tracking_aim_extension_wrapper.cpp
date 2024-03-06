/**************************************************************************/
/*  openxr_fb_hand_tracking_aim_extension_wrapper.cpp                     */
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

#include "extensions/openxr_fb_hand_tracking_aim_extension_wrapper.h"

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_pose.hpp>

using namespace godot;

OpenXRFbHandTrackingAimExtensionWrapper *OpenXRFbHandTrackingAimExtensionWrapper::singleton = nullptr;

OpenXRFbHandTrackingAimExtensionWrapper *OpenXRFbHandTrackingAimExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbHandTrackingAimExtensionWrapper());
	}
	return singleton;
}

OpenXRFbHandTrackingAimExtensionWrapper::OpenXRFbHandTrackingAimExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbHandTrackingAimExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME] = &fb_hand_tracking_aim_ext;
	singleton = this;
}

OpenXRFbHandTrackingAimExtensionWrapper::~OpenXRFbHandTrackingAimExtensionWrapper() {
	cleanup();
}

void OpenXRFbHandTrackingAimExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbHandTrackingAimExtensionWrapper::is_enabled);
}

void OpenXRFbHandTrackingAimExtensionWrapper::cleanup() {
	XRServer *xr_server = XRServer::get_singleton();

	for (int i = 0; i < Hand::HAND_MAX; i++) {
		if (xr_server != nullptr) {
			xr_server->remove_tracker(trackers[i]);
		}
		trackers[i].unref();
	}

	fb_hand_tracking_aim_ext = false;
}

godot::Dictionary OpenXRFbHandTrackingAimExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

PackedStringArray OpenXRFbHandTrackingAimExtensionWrapper::_get_suggested_tracker_names() {
	PackedStringArray arr = PackedStringArray();
	arr.push_back(TRACKER_NAME_LEFT);
	arr.push_back(TRACKER_NAME_RIGHT);
	return arr;
}

void OpenXRFbHandTrackingAimExtensionWrapper::_on_state_ready() {
	is_project_setting_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking_aim");
	if (!is_project_setting_enabled) {
		return;
	}

	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server == nullptr) {
		return;
	}

	trackers[Hand::HAND_LEFT].instantiate();
	trackers[Hand::HAND_LEFT]->set_tracker_type(XRServer::TRACKER_CONTROLLER);
	trackers[Hand::HAND_LEFT]->set_tracker_name(TRACKER_NAME_LEFT);
	trackers[Hand::HAND_LEFT]->set_tracker_desc("FB Aim tracker Left");
	xr_server->add_tracker(trackers[Hand::HAND_LEFT]);

	trackers[Hand::HAND_RIGHT].instantiate();
	trackers[Hand::HAND_RIGHT]->set_tracker_type(XRServer::TRACKER_CONTROLLER);
	trackers[Hand::HAND_RIGHT]->set_tracker_name(TRACKER_NAME_RIGHT);
	trackers[Hand::HAND_RIGHT]->set_tracker_desc("FB Aim tracker Right");
	xr_server->add_tracker(trackers[Hand::HAND_RIGHT]);
}

void OpenXRFbHandTrackingAimExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

uint64_t OpenXRFbHandTrackingAimExtensionWrapper::_set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) {
	ERR_FAIL_INDEX_V_MSG(p_hand_index, Hand::HAND_MAX, reinterpret_cast<uint64_t>(p_next_pointer), vformat("Invalid hand index %d", p_hand_index));

	if (!fb_hand_tracking_aim_ext) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	aim_state[p_hand_index] = {
		XR_TYPE_HAND_TRACKING_AIM_STATE_FB, // type
		p_next_pointer, // next
		0, // status
		{ { 0, 0, 0, 0 }, { 0, 0, 0 } }, // aimPose
		0, // pinchStrengthIndex
		0, // pinchStrengthMiddle
		0, // pinchStrengthRing
		0, // pinchStrengthLittle
	};

	return reinterpret_cast<uint64_t>(&aim_state[p_hand_index]);
}

void OpenXRFbHandTrackingAimExtensionWrapper::_on_process() {
	if (!is_enabled() || !is_project_setting_enabled) {
		return;
	}

	for (int i = 0; i < Hand::HAND_MAX; i++) {
		if (!trackers[i].is_valid()) {
			continue;
		}

		XrPosef aim_pose = aim_state[i].aimPose;
		XrQuaternionf aim_quat = aim_pose.orientation;
		XrVector3f aim_position = aim_pose.position;
		Quaternion quat = Quaternion(aim_quat.x, aim_quat.y, aim_quat.z, aim_quat.w);
		Vector3 origin = Vector3(aim_position.x, aim_position.y, aim_position.z);

		Transform3D transform = Transform3D(quat, origin);
		Vector3 linear_velocity = Vector3(0.0, 0.0, 0.0);
		Vector3 angular_velocity = Vector3(0.0, 0.0, 0.0);

		XRPose::TrackingConfidence confidence = XRPose::TrackingConfidence::XR_TRACKING_CONFIDENCE_LOW;
		if (!(aim_state[i].status & XR_HAND_TRACKING_AIM_VALID_BIT_FB)) {
			confidence = XRPose::TrackingConfidence::XR_TRACKING_CONFIDENCE_NONE;
		}

		trackers[i]->set_pose("default", transform, linear_velocity, angular_velocity, confidence);
		trackers[i]->set_input("index_pinch", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB));
		trackers[i]->set_input("middle_pinch", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_MIDDLE_PINCHING_BIT_FB));
		trackers[i]->set_input("ring_pinch", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_RING_PINCHING_BIT_FB));
		trackers[i]->set_input("little_pinch", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_LITTLE_PINCHING_BIT_FB));
		trackers[i]->set_input("index_pinch_strength", aim_state[i].pinchStrengthIndex);
		trackers[i]->set_input("middle_pinch_strength", aim_state[i].pinchStrengthMiddle);
		trackers[i]->set_input("ring_pinch_strength", aim_state[i].pinchStrengthRing);
		trackers[i]->set_input("little_pinch_strength", aim_state[i].pinchStrengthLittle);
		trackers[i]->set_input("system_gesture", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB));
		trackers[i]->set_input("menu_gesture", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB));
		trackers[i]->set_input("dominant_hand", (bool)(aim_state[i].status & XR_HAND_TRACKING_AIM_DOMINANT_HAND_BIT_FB));
	}
}

void OpenXRFbHandTrackingAimExtensionWrapper::add_project_setting() {
	String p_name = "xr/openxr/extensions/hand_tracking_aim";
	if (!ProjectSettings::get_singleton()->has_setting(p_name)) {
		ProjectSettings::get_singleton()->set_setting(p_name, false);
	}

	ProjectSettings::get_singleton()->set_initial_value(p_name, false);
	Dictionary property_info;
	property_info["name"] = p_name;
	property_info["type"] = Variant::Type::BOOL;
	property_info["hint"] = PROPERTY_HINT_NONE;
	ProjectSettings::get_singleton()->add_property_info(property_info);
}
