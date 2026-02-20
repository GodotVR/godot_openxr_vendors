/**************************************************************************/
/*  openxr_android_trackables_extension.cpp                               */
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

#include "extensions/openxr_android_trackables_extension.h"
#include "classes/openxr_android_trackable_object_tracker.h"
#include "classes/openxr_android_trackable_plane_tracker.h"
#include "extensions/openxr_android_device_anchor_persistence_extension.h"
#include "extensions/openxr_android_raycast_extension.h"
#include "godot_cpp/classes/xr_server.hpp"

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidTrackablesExtension *OpenXRAndroidTrackablesExtension::singleton = nullptr;

OpenXRAndroidTrackablesExtension *OpenXRAndroidTrackablesExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidTrackablesExtension());
	}
	return singleton;
}

int OpenXRAndroidTrackablesExtension::get_next_tracker_id() {
	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();

	int ret = wrapper->next_tracker_id;
	++wrapper->next_tracker_id;

	return ret;
}

OpenXRAndroidTrackablesExtension::OpenXRAndroidTrackablesExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidTrackablesExtension singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_TRACKABLES_EXTENSION_NAME] = &available;
	request_extensions[XR_ANDROID_TRACKABLES_OBJECT_EXTENSION_NAME] = &obj_available;
}

OpenXRAndroidTrackablesExtension::~OpenXRAndroidTrackablesExtension() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidTrackablesExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRAndroidTrackablesExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (available) {
		system_trackables_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_trackables_properties);
	}
	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRAndroidTrackablesExtension::_on_instance_created(uint64_t p_instance) {
	if (!available) {
		return;
	}

	if (!_initialize_openxr_android_trackables_extension()) {
		UtilityFunctions::print("Failed to initialize trackables extension");
		available = false;
	}
}

void OpenXRAndroidTrackablesExtension::_on_session_created(uint64_t p_session_instance) {
	{
		uint32_t trackable_type_count_output = 0;
		XrInstance xr_instance = (XrInstance)get_openxr_api()->get_instance();
		XrSystemId xr_system_id = (XrSystemId)get_openxr_api()->get_system_id();
		XrResult result = xrEnumerateSupportedTrackableTypesANDROID(xr_instance, xr_system_id, 0, &trackable_type_count_output, nullptr);
		if (result != XR_SUCCESS) {
			UtilityFunctions::printerr("OpenXR: Failed to enumerate supported trackable types; ", get_openxr_api()->get_error_string(result));
			available = false;
			return;
		}

		if (trackable_type_count_output == 0) {
			WARN_PRINT("OpenXR: received 0 supported trackables types");
			available = false;
			return;
		}

		supported_trackable_types.resize(trackable_type_count_output);
		result = xrEnumerateSupportedTrackableTypesANDROID(xr_instance, xr_system_id, trackable_type_count_output, &trackable_type_count_output, supported_trackable_types.ptr());
		if (result != XR_SUCCESS) {
			UtilityFunctions::printerr("OpenXR: Failed to enumerate supported trackable types; ", get_openxr_api()->get_error_string(result));
			available = false;
			return;
		}

		if (supported_trackable_types.size() != trackable_type_count_output) {
			WARN_PRINT("OpenXR: enumerate supported trackable types returned a different count");
			if (supported_trackable_types.size() < trackable_type_count_output) {
				UtilityFunctions::printerr("OpenXR: somehow received more supported trackable types on the second query; ", get_openxr_api()->get_error_string(result));
				available = false;
				return;
			}

			if (trackable_type_count_output == 0) {
				UtilityFunctions::printerr("OpenXR: second enumerate supported trackable types returned zero");
				available = false;
				return;
			}

			supported_trackable_types.resize(trackable_type_count_output);
		}
	}

	{
		uint32_t trackable_type_count_output = 0;
		XrInstance xr_instance = (XrInstance)get_openxr_api()->get_instance();
		XrSystemId xr_system_id = (XrSystemId)get_openxr_api()->get_system_id();
		XrResult result = xrEnumerateSupportedAnchorTrackableTypesANDROID(xr_instance, xr_system_id, 0, &trackable_type_count_output, nullptr);
		if (result != XR_SUCCESS) {
			UtilityFunctions::printerr("OpenXR: Failed to enumerate supported anchor trackable types; ", get_openxr_api()->get_error_string(result));
			available = false;
			return;
		}

		if (trackable_type_count_output == 0) {
			WARN_PRINT("OpenXR: received 0 supported anchor trackables types");
			available = false;
			return;
		}

		supported_anchor_trackable_types.resize(trackable_type_count_output);
		result = xrEnumerateSupportedAnchorTrackableTypesANDROID(xr_instance, xr_system_id, trackable_type_count_output, &trackable_type_count_output, supported_anchor_trackable_types.ptr());
		if (result != XR_SUCCESS) {
			UtilityFunctions::printerr("OpenXR: Failed to enumerate supported anchor trackable types; ", get_openxr_api()->get_error_string(result));
			available = false;
			return;
		}

		if (supported_anchor_trackable_types.size() != trackable_type_count_output) {
			WARN_PRINT("OpenXR: enumerate supported anchor trackable types returned a different count");
			if (supported_anchor_trackable_types.size() < trackable_type_count_output) {
				UtilityFunctions::printerr("OpenXR: somehow received more supported anchor trackable types on the second query; ", get_openxr_api()->get_error_string(result));
				available = false;
				return;
			}

			if (trackable_type_count_output == 0) {
				UtilityFunctions::printerr("OpenXR: second enumerate supported anchor trackable types returned zero");
				available = false;
				return;
			}

			supported_anchor_trackable_types.resize(trackable_type_count_output);
		}
	}
}

void OpenXRAndroidTrackablesExtension::_on_process() {
	if (!available) {
		return;
	}

	_on_process_anchors();
	_on_process_trackables();
}

void OpenXRAndroidTrackablesExtension::_on_session_destroyed() {
	_maybe_destroy_trackable_tracker(&plane_trackable_tracker);
	_maybe_destroy_trackable_tracker(&depth_trackable_tracker);
	_maybe_destroy_trackable_tracker(&object_trackable_tracker);
	current_trackables.clear();
	current_anchor_trackers.clear();
	supported_trackable_types.clear();
	supported_anchor_trackable_types.clear();
}

void OpenXRAndroidTrackablesExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_anchor_tracker", "pose", "trackable"), &OpenXRAndroidTrackablesExtension::create_anchor_tracker, DEFVAL(Ref<OpenXRAndroidTrackableTracker>()));
	ClassDB::bind_method(D_METHOD("destroy_anchor_tracker", "anchor_tracker"), &OpenXRAndroidTrackablesExtension::destroy_anchor_tracker);
	ClassDB::bind_method(D_METHOD("get_location_flags", "anchor_tracker", "update"), &OpenXRAndroidTrackablesExtension::get_location_flags, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_location_pose", "anchor_tracker", "update"), &OpenXRAndroidTrackablesExtension::get_location_pose, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_anchor_trackers"), &OpenXRAndroidTrackablesExtension::get_anchor_trackers);
	ClassDB::bind_method(D_METHOD("set_trackable_discovery_cooldown", "cooldown"), &OpenXRAndroidTrackablesExtension::set_trackable_discovery_cooldown);
	ClassDB::bind_method(D_METHOD("discover_trackables", "update_trackers"), &OpenXRAndroidTrackablesExtension::discover_trackables);
	ClassDB::bind_method(D_METHOD("set_anchor_tracker_update_cooldown", "cooldown"), &OpenXRAndroidTrackablesExtension::set_anchor_tracker_update_cooldown);
	ClassDB::bind_method(D_METHOD("update_anchor_trackers"), &OpenXRAndroidTrackablesExtension::update_anchor_trackers);
	ClassDB::bind_method(D_METHOD("can_create_more_anchors"), &OpenXRAndroidTrackablesExtension::can_create_more_anchors);
	ClassDB::bind_method(D_METHOD("is_trackables_supported"), &OpenXRAndroidTrackablesExtension::is_trackables_supported);
}

XrTrackableTrackerANDROID OpenXRAndroidTrackablesExtension::get_or_create_xrtrackable_tracker(XrTrackableTypeANDROID p_xrtrackable_type) {
	ERR_FAIL_COND_V(!supported_trackable_types.has(p_xrtrackable_type), XR_NULL_HANDLE);

	XrTrackableTrackerANDROID *trackable = nullptr;
	switch ((int)p_xrtrackable_type) {
		case XR_TRACKABLE_TYPE_PLANE_ANDROID:
			trackable = &plane_trackable_tracker;
			break;

		// There is no OpenXRAndroidTrackablesExtension::TrackableType::DEPTH, however it is valid to
		// (attempt to) create a XrTrackableTrackerANDROID for DEPTH
		case XR_TRACKABLE_TYPE_DEPTH_ANDROID:
			if (OpenXRAndroidRaycastExtension::get_singleton()->is_raycast_supported()) {
				trackable = &depth_trackable_tracker;
				break;
			}

			UtilityFunctions::printerr(vformat("OpenXR: unable to create Depth trackable tracker; %s is unavailable", XR_ANDROID_RAYCAST_EXTENSION_NAME));
			return XR_NULL_HANDLE;

		case XR_TRACKABLE_TYPE_OBJECT_ANDROID:
			if (obj_available) {
				trackable = &object_trackable_tracker;
				break;
			}

			UtilityFunctions::printerr(vformat("OpenXR: unable to create Object trackable tracker; %s is unavailable", XR_ANDROID_TRACKABLES_OBJECT_EXTENSION_NAME));
			return XR_NULL_HANDLE;

		case XR_TRACKABLE_TYPE_NOT_VALID_ANDROID:
		case XR_TRACKABLE_TYPE_MAX_ENUM_ANDROID:
		default:
			UtilityFunctions::printerr(vformat("OpenXR: invalid xr trackable type %d", p_xrtrackable_type));
			return XR_NULL_HANDLE;
	}

	// sanity, this should have been caught above, but sanity check anyway
	if (trackable == nullptr) {
		UtilityFunctions::printerr("OpenXR: 'trackable' is still nullptr!");
		return XR_NULL_HANDLE;
	}

	if (*trackable != XR_NULL_HANDLE) {
		return *trackable;
	}

	XrTrackableTrackerCreateInfoANDROID create_info = {
		XR_TYPE_TRACKABLE_TRACKER_CREATE_INFO_ANDROID, // type
		nullptr, // next
		p_xrtrackable_type, // trackableType
	};
	XrResult result = xrCreateTrackableTrackerANDROID(SESSION, &create_info, trackable);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to create trackable tracker; ", get_openxr_api()->get_error_string(result));
		return XR_NULL_HANDLE;
	}

	return *trackable;
}

Ref<OpenXRAndroidTrackableTracker> OpenXRAndroidTrackablesExtension::get_or_create_tracker_and_update(XrTrackableANDROID p_xrtrackable, XrTrackableTypeANDROID p_xrtrackable_type, bool p_update_tracker) {
	Ref<OpenXRAndroidTrackableTracker> ret;
	if (current_trackables.has(p_xrtrackable)) {
		ret = current_trackables[p_xrtrackable];
		if (p_update_tracker && ret.is_valid()) {
			ret->update();
		}
	} else {
		ERR_FAIL_COND_V(!supported_trackable_types.has(p_xrtrackable_type), ret);

		XRServer *xr_server = XRServer::get_singleton();
		ERR_FAIL_NULL_V(xr_server, ret);

		switch ((int)p_xrtrackable_type) {
			case XR_TRACKABLE_TYPE_PLANE_ANDROID:
				ret = OpenXRAndroidTrackablePlaneTracker::create(p_xrtrackable);
				break;
			case XR_TRACKABLE_TYPE_OBJECT_ANDROID:
				ret = OpenXRAndroidTrackableObjectTracker::create(p_xrtrackable);
				break;
			default:
				UtilityFunctions::printerr("OpenXR: Unsupported trackable type; ", p_xrtrackable_type);
				return ret;
		}

		if (ret.is_valid()) {
			// NOTE: Updating only makes sense for current trackers, since updating can notify other
			// objects that are connected to its signals.
			// For new trackers like this case, there is nothing connect to any of its signals because we
			// just created it.
			// Like all OpenXRAndroidTrackableTrackers, it'll update its data when/if anything calls one of its
			// getter functions.

			current_trackables[p_xrtrackable] = ret;
			xr_server->add_tracker(ret);
		}
	}

	return ret;
}

void OpenXRAndroidTrackablesExtension::set_trackable_discovery_cooldown(int p_cooldown) {
	trackable_discovery_cooldown = p_cooldown;
	trackable_discovery_cooldown_cur = p_cooldown;
}

void OpenXRAndroidTrackablesExtension::discover_trackables(bool p_update_trackers) {
	if (supported_trackable_types.is_empty()) {
		return;
	}

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL(xr_server);

	HashMap<XrTrackableANDROID, Ref<OpenXRAndroidTrackableTracker>> trackables_to_delete = current_trackables;

	_find_and_update_all_trackers(XR_TRACKABLE_TYPE_PLANE_ANDROID, p_update_trackers, trackables_to_delete);
	_find_and_update_all_trackers(XR_TRACKABLE_TYPE_OBJECT_ANDROID, p_update_trackers, trackables_to_delete);

	// delete trackables that we didn't find
	for (const auto &[xr_trackable_to_delete, tracker] : trackables_to_delete) {
		current_trackables.erase(xr_trackable_to_delete);
		xr_server->remove_tracker(tracker);
	}
}

Ref<OpenXRAndroidAnchorTracker> OpenXRAndroidTrackablesExtension::create_anchor_tracker(const Transform3D &p_pose, Ref<OpenXRAndroidTrackableTracker> p_tracker) {
	Ref<OpenXRAndroidAnchorTracker> ret{};
	if (!can_create_more_anchors()) {
		UtilityFunctions::printerr("OpenXR: Unable to create more anchor trackers");
		return ret;
	}

	if (p_tracker.is_valid() && !supported_anchor_trackable_types.has(p_tracker->get_trackable_type())) {
		UtilityFunctions::printerr("OpenXR: Failed to create anchor tracker; unsupported trackable type given");
		return ret;
	}

	XrPosef xr_pose{};
	xr_pose.position.x = p_pose.origin.x;
	xr_pose.position.y = p_pose.origin.y;
	xr_pose.position.z = p_pose.origin.z;

	Quaternion quat = p_pose.basis.get_quaternion();
	xr_pose.orientation.w = quat.w;
	xr_pose.orientation.x = quat.x;
	xr_pose.orientation.y = quat.y;
	xr_pose.orientation.z = quat.z;

	XrAnchorSpaceCreateInfoANDROID create_info = {
		XR_TYPE_ANCHOR_SPACE_CREATE_INFO_ANDROID, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // space
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
		xr_pose, // pose
		p_tracker.is_null() ? XR_NULL_TRACKABLE_ANDROID : p_tracker->get_trackable(), // trackable
	};
	XrSpace output;
	XrResult result = xrCreateAnchorSpaceANDROID(SESSION, &create_info, &output);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to create anchor space; ", get_openxr_api()->get_error_string(result));
		return ret;
	}

	return xrcreate_anchor_tracker(output, StringName{}, p_tracker);
}

Ref<OpenXRAndroidAnchorTracker> OpenXRAndroidTrackablesExtension::xrcreate_anchor_tracker(XrSpace p_xrspace, const StringName &p_persist_uuid, Ref<OpenXRAndroidTrackableTracker> p_tracker) {
	Ref<OpenXRAndroidAnchorTracker> ret;
	ERR_FAIL_COND_V(!can_create_more_anchors(), ret);

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V(xr_server, ret);

	if (current_anchor_trackers.has(p_xrspace)) {
		UtilityFunctions::printerr("OpenXR: Failed to create anchor space; XrSpace already exists");
		return ret;
	}

	ret = OpenXRAndroidAnchorTracker::create(p_xrspace, p_persist_uuid, p_tracker);
	current_anchor_trackers[p_xrspace] = ret;
	xr_server->add_tracker(ret);

	return ret;
}

void OpenXRAndroidTrackablesExtension::destroy_anchor_tracker(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker) {
	if (p_anchor_tracker.is_null()) {
		return;
	}

	if (!current_anchor_trackers.has(p_anchor_tracker->get_xrspace())) {
		UtilityFunctions::printerr("OpenXR: Failed to destroy anchor space; where did this tracker come from?");
		return;
	}

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL(xr_server);

	XrResult result = xrDestroySpace(p_anchor_tracker->get_xrspace());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to destroy anchor space; ", get_openxr_api()->get_error_string(result));
	}

	// The anchor tracker was destroyed
	// If it was persisted, it'll remain persisted, however we have to remove it from our bookkeeping
	// to avoid using the now-destroyed xrspace if the caller decides to call
	// OpenXRAndroidDeviceAnchorPersistenceExtension::create_persisted_anchor_tracker()
	// (which would return this broken tracker)
	OpenXRAndroidDeviceAnchorPersistenceExtension *wrapper = OpenXRAndroidDeviceAnchorPersistenceExtension::get_singleton();
	wrapper->on_xranchor_tracker_destroyed(p_anchor_tracker->get_persist_uuid());

	current_anchor_trackers.erase(p_anchor_tracker->get_xrspace());
	xr_server->remove_tracker(p_anchor_tracker);
}

BitField<OpenXRAndroidAnchorTracker::LocationFlags> OpenXRAndroidTrackablesExtension::get_location_flags(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, bool p_update) const {
	if (p_anchor_tracker.is_null()) {
		return BitField<OpenXRAndroidAnchorTracker::LocationFlags>{ 0 };
	}

	return p_anchor_tracker->get_location_flags(p_update);
}

Transform3D OpenXRAndroidTrackablesExtension::get_location_pose(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, bool p_update) const {
	if (p_anchor_tracker.is_null()) {
		return Transform3D{};
	}

	return p_anchor_tracker->get_location_pose(p_update);
}

TypedArray<OpenXRAndroidAnchorTracker> OpenXRAndroidTrackablesExtension::get_anchor_trackers() const {
	TypedArray<OpenXRAndroidAnchorTracker> ret;
	ret.resize(current_anchor_trackers.size());

	int i = 0;
	for (const auto &[_, anchor_tracker] : current_anchor_trackers) {
		ret[i] = anchor_tracker;
		++i;
	}

	return ret;
}

void OpenXRAndroidTrackablesExtension::set_anchor_tracker_update_cooldown(int p_cooldown) {
	anchor_update_cooldown = p_cooldown;
	anchor_update_cooldown_cur = p_cooldown;
}

void OpenXRAndroidTrackablesExtension::update_anchor_trackers() {
	for (const auto &[xrspace, anchor_tracker] : current_anchor_trackers) {
		if (anchor_tracker.is_null()) {
			continue;
		}

		anchor_tracker->update();
	}
}

bool OpenXRAndroidTrackablesExtension::can_create_more_anchors() const {
	return available && system_trackables_properties.supportsAnchor == XR_TRUE && current_anchor_trackers.size() < system_trackables_properties.maxAnchors;
}

bool OpenXRAndroidTrackablesExtension::is_trackables_supported() const {
	return available;
}

bool OpenXRAndroidTrackablesExtension::_initialize_openxr_android_trackables_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSupportedTrackableTypesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSupportedAnchorTrackableTypesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateTrackableTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyTrackableTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetAllTrackablesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetTrackablePlaneANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetTrackableObjectANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateAnchorSpaceANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateSpace);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySpace);
	return true;
}

void OpenXRAndroidTrackablesExtension::_maybe_destroy_trackable_tracker(XrTrackableTrackerANDROID *trackable_tracker) {
	if (XR_NULL_HANDLE == *trackable_tracker) {
		return;
	}

	XrResult result = xrDestroyTrackableTrackerANDROID(*trackable_tracker);
	if (result != XR_SUCCESS) {
		WARN_PRINT("OpenXR: Failed to destroy trackable tracker");
	}

	*trackable_tracker = XR_NULL_HANDLE;
}

void OpenXRAndroidTrackablesExtension::_find_and_update_all_trackers(XrTrackableTypeANDROID p_xrtrackable_type, bool p_update_trackers, HashMap<XrTrackableANDROID, Ref<OpenXRAndroidTrackableTracker>> &p_trackables_to_delete) {
	if (!supported_trackable_types.has(p_xrtrackable_type)) {
		return;
	}

	XrTrackableTrackerANDROID trackable_tracker = XR_NULL_HANDLE;
	switch ((int)p_xrtrackable_type) {
		case XR_TRACKABLE_TYPE_PLANE_ANDROID:
			trackable_tracker = get_or_create_xrtrackable_tracker(XR_TRACKABLE_TYPE_PLANE_ANDROID);
			break;
		case XR_TRACKABLE_TYPE_OBJECT_ANDROID:
			trackable_tracker = get_or_create_xrtrackable_tracker(XR_TRACKABLE_TYPE_OBJECT_ANDROID);
			break;
		default:
			UtilityFunctions::printerr(vformat("OpenXR: invalid trackable type %d", p_xrtrackable_type));
			break;
	}

	if (trackable_tracker == XR_NULL_HANDLE) {
		return;
	}

	uint32_t trackable_count_output = 0;
	XrResult result = xrGetAllTrackablesANDROID(trackable_tracker, 0, &trackable_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to query trackables count; ", get_openxr_api()->get_error_string(result));
		return;
	}

	if (trackable_count_output == 0) {
		// This is okay - TRACKABLE_TYPE_OBJECT usually returns 0
		return;
	}

	uint32_t trackable_count = trackable_count_output;
	LocalVector<XrTrackableANDROID> trackables{};
	trackables.resize(trackable_count);
	result = xrGetAllTrackablesANDROID(trackable_tracker, trackable_count, &trackable_count_output, trackables.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to query trackables count; ", get_openxr_api()->get_error_string(result));
		return;
	}

	if (trackable_count_output != trackable_count) {
		WARN_PRINT("OpenXR: trackable query count differs from actual query");
	}

	// Run through the queried trackables.
	// Absent trackables will remain in p_trackables_to_delete (and will be deleted later)
	// New trackables will be updated and added to the XRServer in get_or_create_tracker_and_update()
	// All found trackables will be updated (when p_update_trackers is true)
	for (int i = 0; i < trackable_count_output; ++i) {
		get_or_create_tracker_and_update(trackables[i], p_xrtrackable_type, p_update_trackers);
		p_trackables_to_delete.erase(trackables[i]);
	}
}

void OpenXRAndroidTrackablesExtension::_on_process_anchors() {
	if (anchor_update_cooldown < 0) {
		return;
	}

	if (0 < anchor_update_cooldown_cur) {
		--anchor_update_cooldown_cur;
		return;
	}
	anchor_update_cooldown_cur = anchor_update_cooldown;

	update_anchor_trackers();
}

void OpenXRAndroidTrackablesExtension::_on_process_trackables() {
	if (trackable_discovery_cooldown < 0) {
		return;
	}

	if (0 < trackable_discovery_cooldown_cur) {
		--trackable_discovery_cooldown_cur;
		return;
	}
	trackable_discovery_cooldown_cur = trackable_discovery_cooldown;

	discover_trackables(true);
}
