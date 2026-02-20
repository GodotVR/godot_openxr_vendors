/**************************************************************************/
/*  openxr_android_device_anchor_persistence_extension.cpp                */
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

#include "extensions/openxr_android_device_anchor_persistence_extension.h"

#include <androidxr/androidxr.h>

#include "extensions/openxr_android_trackables_extension.h"
#include "util.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidDeviceAnchorPersistenceExtension *OpenXRAndroidDeviceAnchorPersistenceExtension::singleton = nullptr;

OpenXRAndroidDeviceAnchorPersistenceExtension *OpenXRAndroidDeviceAnchorPersistenceExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidDeviceAnchorPersistenceExtension());
	}
	return singleton;
}

OpenXRAndroidDeviceAnchorPersistenceExtension::OpenXRAndroidDeviceAnchorPersistenceExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidDeviceAnchorPersistenceExtension singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_DEVICE_ANCHOR_PERSISTENCE_EXTENSION_NAME] = &available;
}

OpenXRAndroidDeviceAnchorPersistenceExtension::~OpenXRAndroidDeviceAnchorPersistenceExtension() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidDeviceAnchorPersistenceExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRAndroidDeviceAnchorPersistenceExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	anchor_persistence_properties.type = XR_TYPE_SYSTEM_DEVICE_ANCHOR_PERSISTENCE_PROPERTIES_ANDROID;
	anchor_persistence_properties.next = p_next_pointer;
	anchor_persistence_properties.supportsAnchorPersistence = XR_FALSE;
	return reinterpret_cast<uint64_t>(&anchor_persistence_properties);
}

void OpenXRAndroidDeviceAnchorPersistenceExtension::_on_instance_created(uint64_t p_instance) {
	// Only check 'available' here, because 'available' has been set by the XR runtime, but
	// 'anchor_persistence_properties.supportsAnchorPersistence' has not been set yet.
	// This makes sense because:
	// 1: _on_instance_created() is called right after the xr instance is created, AND
	// 2: _set_system_properties_and_get_next_pointer() is called right before
	//    xrGetSystemProperties(), which requires the xr instance.
	if (!available) {
		return;
	}

	if (!_initialize_androidxr_device_persistence_extension()) {
		UtilityFunctions::print("Failed to initialize device anchor persistence extension");
		available = false;
		return;
	}
}

void OpenXRAndroidDeviceAnchorPersistenceExtension::_on_session_created(uint64_t p_session_instance) {
	available = available && XR_TRUE == anchor_persistence_properties.supportsAnchorPersistence;

	if (!available) {
		return;
	}

	uint32_t trackable_type_count_output = 0;
	XrInstance xr_instance = (XrInstance)get_openxr_api()->get_instance();
	XrSystemId xr_system_id = (XrSystemId)get_openxr_api()->get_system_id();
	XrResult result = xrEnumerateSupportedPersistenceAnchorTypesANDROID(xr_instance, xr_system_id, 0, &trackable_type_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate supported persistence anchor types; ", get_openxr_api()->get_error_string(result));
		available = false;
		return;
	}

	if (trackable_type_count_output == 0) {
		// This is okay. Spatial anchors (which are not a Trackable type) are supported too.
		return;
	}

	LocalVector<XrTrackableTypeANDROID> trackable_types{};
	trackable_types.resize(trackable_type_count_output);
	result = xrEnumerateSupportedPersistenceAnchorTypesANDROID(xr_instance, xr_system_id, trackable_type_count_output, &trackable_type_count_output, trackable_types.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate supported persistence anchor types; ", get_openxr_api()->get_error_string(result));
		available = false;
		return;
	}

	if (trackable_types.size() != trackable_type_count_output) {
		WARN_PRINT("OpenXR: enumerate supported persistence anchor types returned a different count");
		if (trackable_types.size() < trackable_type_count_output) {
			UtilityFunctions::printerr("OpenXR: somehow received more supported persistence anchor types on the second query; ", get_openxr_api()->get_error_string(result));
			available = false;
			return;
		}

		if (trackable_type_count_output == 0) {
			UtilityFunctions::printerr("OpenXR: second enumerate supported persistence anchor types returned zero");
			available = false;
			return;
		}

		trackable_types.resize(trackable_type_count_output);
	}

	for (const XrTrackableTypeANDROID trackable_type : trackable_types) {
		supported_trackable_types.insert(trackable_type);
	}
}

void OpenXRAndroidDeviceAnchorPersistenceExtension::_on_process() {
	if (persisted_anchor_discovery_cooldown < 0 || !OpenXRAndroidTrackablesExtension::get_singleton()->can_create_more_anchors()) {
		return;
	}

	if (0 < persisted_anchor_discovery_cooldown_cur) {
		--persisted_anchor_discovery_cooldown_cur;
		return;
	}
	persisted_anchor_discovery_cooldown_cur = persisted_anchor_discovery_cooldown;

	if (!restore_persisted_anchor_trackers()) {
		// there are no more anchors to restore; stop trying to restore persisted anchor trackers
		persisted_anchor_discovery_cooldown = -1;
	}
}

void OpenXRAndroidDeviceAnchorPersistenceExtension::_on_session_destroyed() {
	supported_trackable_types.clear();
	restored_persisted_anchor_trackers.clear();
	remaining_persisted_anchors_to_restore.clear();
	already_retrieved_persisted_anchors_to_restore = false;
	if (XR_NULL_HANDLE == device_anchor_persistence) {
		return;
	}

	XrResult result = xrDestroyDeviceAnchorPersistenceANDROID(device_anchor_persistence);
	if (result != XR_SUCCESS) {
		WARN_PRINT("OpenXR: Failed to destroy anchor persistence");
	}

	device_anchor_persistence = XR_NULL_HANDLE;
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::set_restore_persisted_anchors_cooldown(int p_cooldown) {
	// don't bother setting a valid cooldown when there's nothing to restore
	if (already_retrieved_persisted_anchors_to_restore && remaining_persisted_anchors_to_restore.is_empty()) {
		persisted_anchor_discovery_cooldown = -1;
		return false;
	}

	persisted_anchor_discovery_cooldown = p_cooldown;
	persisted_anchor_discovery_cooldown_cur = p_cooldown;
	return true;
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::restore_persisted_anchor_trackers() {
	OpenXRAndroidTrackablesExtension *trackables_extension = OpenXRAndroidTrackablesExtension::get_singleton();
	ERR_FAIL_NULL_V(trackables_extension, false);

	if (!trackables_extension->can_create_more_anchors()) {
		WARN_PRINT("OpenXR: unable to restore persisted anchor trackers");
		return false;
	}

	if (!already_retrieved_persisted_anchors_to_restore) {
		TypedArray<StringName> all_persisted_anchors = _get_all_persisted_anchors(already_retrieved_persisted_anchors_to_restore);
		if (!already_retrieved_persisted_anchors_to_restore) {
			return true;
		}

		for (const StringName &uuid : all_persisted_anchors) {
			if (!restored_persisted_anchor_trackers.has(uuid)) {
				remaining_persisted_anchors_to_restore[uuid] = _create_persisted_anchor_tracker(uuid);
			}
		}

		already_retrieved_persisted_anchors_to_restore = true;
	}

	if (remaining_persisted_anchors_to_restore.is_empty()) {
		return false;
	}

	LocalVector<StringName> to_erase;
	for (const auto &[uuid, xrspace] : remaining_persisted_anchors_to_restore) {
		if (!trackables_extension->can_create_more_anchors()) {
			break;
		}

		if (!_is_xrspace_location_valid(xrspace)) {
			continue;
		}

		if (!restored_persisted_anchor_trackers.has(uuid)) {
			restored_persisted_anchor_trackers[uuid] = trackables_extension->xrcreate_anchor_tracker(xrspace, uuid, Ref<OpenXRAndroidTrackableTracker>{});
		}

		to_erase.push_back(uuid);
	}

	for (const StringName &uuid : to_erase) {
		remaining_persisted_anchors_to_restore.erase(uuid);
	}

	return !remaining_persisted_anchors_to_restore.is_empty();
}

TypedArray<StringName> OpenXRAndroidDeviceAnchorPersistenceExtension::get_all_persisted_anchors() {
	bool success;
	return _get_all_persisted_anchors(success);
}

Ref<OpenXRAndroidAnchorTracker> OpenXRAndroidDeviceAnchorPersistenceExtension::create_persisted_anchor_tracker(StringName p_uuid) {
	Ref<OpenXRAndroidAnchorTracker> ret;
	if (!available) {
		return ret;
	}

	if (!OpenXRAndroidTrackablesExtension::get_singleton()->can_create_more_anchors()) {
		WARN_PRINT("OpenXR: unable to create persisted anchor tracker");
		return ret;
	}

	if (p_uuid.is_empty()) {
		UtilityFunctions::printerr("OpenXR: Failed to create tracker from anchor; must provide a non-empty uuid");
		return ret;
	}

	if (restored_persisted_anchor_trackers.has(p_uuid)) {
		WARN_PRINT("OpenXR: anchor tracker is already persisted; returning existing instance");
		return restored_persisted_anchor_trackers.get(p_uuid);
	}

	XrSpace anchor_space;
	if (remaining_persisted_anchors_to_restore.has(p_uuid)) {
		anchor_space = remaining_persisted_anchors_to_restore[p_uuid];
	} else {
		anchor_space = _create_persisted_anchor_tracker(p_uuid);
		if (anchor_space == XR_NULL_HANDLE) {
			UtilityFunctions::printerr("OpenXR: Failed to create tracker from anchor; unable to create underlying XrSpace");
			return ret;
		}
	}

	if (!_is_xrspace_location_valid(anchor_space)) {
		UtilityFunctions::printerr("OpenXR: Failed to create tracker from anchor tracker; location is invalid");
		remaining_persisted_anchors_to_restore[p_uuid] = anchor_space;
		return ret;
	}

	ret = OpenXRAndroidTrackablesExtension::get_singleton()->xrcreate_anchor_tracker(anchor_space, p_uuid, Ref<OpenXRAndroidTrackableTracker>{});
	restored_persisted_anchor_trackers[p_uuid] = ret;
	remaining_persisted_anchors_to_restore.erase(p_uuid);

	return ret;
}

XrSpace OpenXRAndroidDeviceAnchorPersistenceExtension::_create_persisted_anchor_tracker(StringName p_uuid) {
	XrDeviceAnchorPersistenceANDROID anchor_persistence = _get_or_create_device_anchor_persistence();
	if (XR_NULL_HANDLE == anchor_persistence) {
		UtilityFunctions::printerr("OpenXR: Failed to get anchor persist state; unable to get or create device anchor persistence handle");
		return XR_NULL_HANDLE;
	}

	XrUuidEXT xruuid{ OpenXRUtilities::string_name_to_uuid(p_uuid) };
	XrPersistedAnchorSpaceCreateInfoANDROID create_info = {
		XR_TYPE_PERSISTED_ANCHOR_SPACE_CREATE_INFO_ANDROID, // type
		nullptr, // next
		xruuid, // anchorId
	};
	XrSpace anchor_output;
	XrResult result = xrCreatePersistedAnchorSpaceANDROID(anchor_persistence, &create_info, &anchor_output);
	if (result != XR_SUCCESS || XR_NULL_HANDLE == anchor_output) {
		UtilityFunctions::printerr("OpenXR: Failed to create device anchor persistence; ", get_openxr_api()->get_error_string(result));
		return XR_NULL_HANDLE;
	}

	return anchor_output;
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::persist_anchor_tracker(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker) const {
	if (p_anchor_tracker.is_null()) {
		UtilityFunctions::printerr("OpenXR: Failed to persist anchor tracker; must provide a non-null anchor space");
		return false;
	}

	return p_anchor_tracker->persist();
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::persist_xranchor(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, StringName &o_uuid, XrUuidEXT &o_xruuid) {
	if (!available || p_anchor_tracker.is_null()) {
		return false;
	}

	if (restored_persisted_anchor_trackers.has(p_anchor_tracker->get_persist_uuid())) {
		WARN_PRINT("OpenXR: anchor tracker is already persisted");
		return true;
	}

	// It's okay if OpenXRAndroidTrackables is null, since that means it's a Spatial Anchor.
	Ref<OpenXRAndroidTrackableTracker> tracker = p_anchor_tracker->get_tracker();
	if (tracker.is_valid()) {
		if (!supported_trackable_types.has(tracker->get_trackable_type())) {
			UtilityFunctions::printerr("OpenXR: Failed to persist anchor tracker; anchor tracker was created with an unsupported trackable type");
			return false;
		}
	}

	// XrSpace must have a valid location or else it cannot be persisted
	XrSpace xranchor_space = p_anchor_tracker->get_xrspace();
	if (!_is_xrspace_location_valid(xranchor_space)) {
		UtilityFunctions::printerr("OpenXR: Failed to persist anchor tracker; location is invalid");
		return false;
	}

	XrDeviceAnchorPersistenceANDROID anchor_persistence = _get_or_create_device_anchor_persistence();
	if (XR_NULL_HANDLE == anchor_persistence) {
		UtilityFunctions::printerr("OpenXR: Failed to persist anchor tracker; unable to get or create device anchor persistence handle");
		return false;
	}

	XrPersistedAnchorSpaceInfoANDROID persisted_info = {
		XR_TYPE_PERSISTED_ANCHOR_SPACE_INFO_ANDROID, // type
		nullptr, // next
		xranchor_space, // anchor
	};
	XrResult result = xrPersistAnchorANDROID(anchor_persistence, &persisted_info, &o_xruuid);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to persist anchor tracker; ", get_openxr_api()->get_error_string(result));
		return false;
	}

	o_uuid = OpenXRUtilities::uuid_to_string_name(o_xruuid);
	restored_persisted_anchor_trackers[o_uuid] = p_anchor_tracker;
	remaining_persisted_anchors_to_restore.erase(o_uuid);

	return true;
}

OpenXRAndroidAnchorTracker::PersistState OpenXRAndroidDeviceAnchorPersistenceExtension::get_anchor_tracker_persist_state_uuid(StringName p_uuid, bool p_update) {
	if (!restored_persisted_anchor_trackers.has(p_uuid)) {
		// still attempt to get persist state, since p_uuid could have been from
		// get_all_persisted_anchors() and a OpenXRAndroidAnchorTracker was never created from it
		return get_xranchor_persist_state(p_uuid, OpenXRUtilities::string_name_to_uuid(p_uuid));
	}

	return get_anchor_tracker_persist_state(restored_persisted_anchor_trackers.get(p_uuid), p_update);
}

OpenXRAndroidAnchorTracker::PersistState OpenXRAndroidDeviceAnchorPersistenceExtension::get_anchor_tracker_persist_state(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, bool p_update) const {
	if (p_anchor_tracker.is_null()) {
		// null anchor trackers cannot request persistance
		return OpenXRAndroidAnchorTracker::PERSIST_STATE_NOT_REQUESTED;
	}

	return p_anchor_tracker->get_persist_state(p_update);
}

OpenXRAndroidAnchorTracker::PersistState OpenXRAndroidDeviceAnchorPersistenceExtension::get_xranchor_persist_state(const StringName &p_uuid, const XrUuidEXT &p_xruuid) {
	OpenXRAndroidAnchorTracker::PersistState ret{ OpenXRAndroidAnchorTracker::PERSIST_STATE_ERROR };
	if (!available) {
		return ret;
	}

	if (p_uuid.is_empty()) {
		// don't log so callers don't have to check for empty uuid at every call site
		return OpenXRAndroidAnchorTracker::PERSIST_STATE_NOT_REQUESTED;
	}

	XrDeviceAnchorPersistenceANDROID anchor_persistence = _get_or_create_device_anchor_persistence();
	if (XR_NULL_HANDLE == anchor_persistence) {
		UtilityFunctions::printerr("OpenXR: Failed to get anchor persist state; unable to get or create device anchor persistence handle");
		return ret;
	}

	XrAnchorPersistStateANDROID persist_state{};
	XrResult result = xrGetAnchorPersistStateANDROID(anchor_persistence, &p_xruuid, &persist_state);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get anchor persist state; ", get_openxr_api()->get_error_string(result));
		return ret;
	}

	switch (persist_state) {
		case XR_ANCHOR_PERSIST_STATE_PERSIST_NOT_REQUESTED_ANDROID:
			ret = OpenXRAndroidAnchorTracker::PERSIST_STATE_NOT_REQUESTED;
			break;
		case XR_ANCHOR_PERSIST_STATE_PERSIST_PENDING_ANDROID:
			ret = OpenXRAndroidAnchorTracker::PERSIST_STATE_PENDING;
			break;
		case XR_ANCHOR_PERSIST_STATE_PERSISTED_ANDROID:
			ret = OpenXRAndroidAnchorTracker::PERSIST_STATE_PERSISTED;
			break;
		case XR_ANCHOR_PERSIST_STATE_MAX_ENUM_ANDROID:
		default:
			UtilityFunctions::printerr("OpenXR: Invalid anchor persist state; ", persist_state);
			break;
	}

	return ret;
}

TypedArray<OpenXRAndroidAnchorTracker> OpenXRAndroidDeviceAnchorPersistenceExtension::get_persisted_anchor_trackers() const {
	TypedArray<OpenXRAndroidAnchorTracker> ret;
	ret.resize(restored_persisted_anchor_trackers.size());

	int i = 0;
	for (const auto &[_, persisted_anchor_tracker] : restored_persisted_anchor_trackers) {
		ret[i] = persisted_anchor_tracker;
		++i;
	}

	return ret;
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::unpersist_anchor_uuid(StringName p_uuid) {
	if (!restored_persisted_anchor_trackers.has(p_uuid)) {
		// still attempt to unpersist, since p_uuid could have been from get_all_persisted_anchors() and
		// a OpenXRAndroidAnchorTracker was never created from it
		return unpersist_xranchor(p_uuid, OpenXRUtilities::string_name_to_uuid(p_uuid));
	}

	return unpersist_anchor_tracker(restored_persisted_anchor_trackers.get(p_uuid));
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::unpersist_anchor_tracker(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker) const {
	if (p_anchor_tracker.is_null()) {
		// null anchors are never persisted
		return true;
	}

	return p_anchor_tracker->unpersist();
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::unpersist_xranchor(const StringName &p_uuid, const XrUuidEXT &p_xruuid) {
	if (!available || p_uuid.is_empty()) {
		// don't log so callers don't have to check for empty uuid at every call site
		return true;
	}

	XrDeviceAnchorPersistenceANDROID anchor_persistence = _get_or_create_device_anchor_persistence();
	if (XR_NULL_HANDLE == anchor_persistence) {
		UtilityFunctions::printerr("OpenXR: Failed to unpersist anchor tracker; unable to get or create device anchor persistence handle");
		return false;
	}

	XrResult result = xrUnpersistAnchorANDROID(anchor_persistence, &p_xruuid);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to unpersist anchor tracker; ", get_openxr_api()->get_error_string(result));
		return false;
	}

	// this p_uuid may not have been used to create a OpenXRAndroidAnchorTracker, which is okay
	on_xranchor_tracker_destroyed(p_uuid);

	return true;
}

void OpenXRAndroidDeviceAnchorPersistenceExtension::on_xranchor_tracker_destroyed(const StringName &p_uuid) {
	if (restored_persisted_anchor_trackers.has(p_uuid)) {
		restored_persisted_anchor_trackers.erase(p_uuid);
	}

	if (remaining_persisted_anchors_to_restore.has(p_uuid)) {
		remaining_persisted_anchors_to_restore.erase(p_uuid);
	}
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::is_device_anchor_persistence_supported() const {
	return available;
}

TypedArray<StringName> OpenXRAndroidDeviceAnchorPersistenceExtension::_get_all_persisted_anchors(bool &o_success) {
	o_success = false;
	TypedArray<StringName> ret{};
	if (!available) {
		return ret;
	}

	XrDeviceAnchorPersistenceANDROID anchor_persistence = _get_or_create_device_anchor_persistence();
	if (XR_NULL_HANDLE == anchor_persistence) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate persisted anchors; unable to get or create device anchor persistence handle");
		return ret;
	}

	uint32_t persisted_anchor_ids_count_output = 0;
	XrResult result = xrEnumeratePersistedAnchorsANDROID(anchor_persistence, 0, &persisted_anchor_ids_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate persisted anchors; ", get_openxr_api()->get_error_string(result));
		return ret;
	}

	if (persisted_anchor_ids_count_output == 0) {
		o_success = true;
		return ret;
	}

	LocalVector<XrUuidEXT> persisted_anchor_ids;
	persisted_anchor_ids.resize(persisted_anchor_ids_count_output);
	result = xrEnumeratePersistedAnchorsANDROID(anchor_persistence, persisted_anchor_ids_count_output, &persisted_anchor_ids_count_output, persisted_anchor_ids.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate persisted anchors; ", get_openxr_api()->get_error_string(result));
		return ret;
	}

	if (persisted_anchor_ids.size() != persisted_anchor_ids_count_output) {
		WARN_PRINT("OpenXR: enumerate persisted anchors returned a different count");
		if (persisted_anchor_ids.size() < persisted_anchor_ids_count_output) {
			UtilityFunctions::printerr("OpenXR: somehow received more persisted anchors on the second query; ", get_openxr_api()->get_error_string(result));
			return ret;
		}

		if (persisted_anchor_ids_count_output == 0) {
			UtilityFunctions::printerr("OpenXR: second enumerate persisted anchors returned zero");
			return ret;
		}

		persisted_anchor_ids.resize(persisted_anchor_ids_count_output);
	}

	for (const XrUuidEXT &persisted_anchor_id : persisted_anchor_ids) {
		ret.push_back(OpenXRUtilities::uuid_to_string_name(persisted_anchor_id));
	}

	o_success = true;
	return ret;
}

XrDeviceAnchorPersistenceANDROID OpenXRAndroidDeviceAnchorPersistenceExtension::_get_or_create_device_anchor_persistence() {
	if (XR_NULL_HANDLE != device_anchor_persistence) {
		return device_anchor_persistence;
	}

	XrDeviceAnchorPersistenceCreateInfoANDROID create_info = {
		XR_TYPE_DEVICE_ANCHOR_PERSISTENCE_CREATE_INFO_ANDROID, // type
		nullptr, // next
	};
	XrResult result = xrCreateDeviceAnchorPersistenceANDROID(SESSION, &create_info, &device_anchor_persistence);
	if (result != XR_SUCCESS || XR_NULL_HANDLE == device_anchor_persistence) {
		UtilityFunctions::printerr("OpenXR: Failed to create device anchor persistence; ", get_openxr_api()->get_error_string(result));
		available = false;
		device_anchor_persistence = XR_NULL_HANDLE;
		return XR_NULL_HANDLE;
	}

	return device_anchor_persistence;
}

void OpenXRAndroidDeviceAnchorPersistenceExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_restore_persisted_anchors_cooldown", "cooldown"), &OpenXRAndroidDeviceAnchorPersistenceExtension::set_restore_persisted_anchors_cooldown);
	ClassDB::bind_method(D_METHOD("restore_persisted_anchor_trackers"), &OpenXRAndroidDeviceAnchorPersistenceExtension::restore_persisted_anchor_trackers);
	ClassDB::bind_method(D_METHOD("get_all_persisted_anchors"), &OpenXRAndroidDeviceAnchorPersistenceExtension::get_all_persisted_anchors);
	ClassDB::bind_method(D_METHOD("create_persisted_anchor_tracker", "uuid"), &OpenXRAndroidDeviceAnchorPersistenceExtension::create_persisted_anchor_tracker);
	ClassDB::bind_method(D_METHOD("persist_anchor_tracker", "anchor_tracker"), &OpenXRAndroidDeviceAnchorPersistenceExtension::persist_anchor_tracker);
	ClassDB::bind_method(D_METHOD("get_anchor_tracker_persist_state", "anchor_tracker", "update"), &OpenXRAndroidDeviceAnchorPersistenceExtension::get_anchor_tracker_persist_state, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_anchor_tracker_persist_state_uuid", "uuid", "update"), &OpenXRAndroidDeviceAnchorPersistenceExtension::get_anchor_tracker_persist_state_uuid, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_persisted_anchor_trackers"), &OpenXRAndroidDeviceAnchorPersistenceExtension::get_persisted_anchor_trackers);
	ClassDB::bind_method(D_METHOD("unpersist_anchor_tracker", "anchor_tracker"), &OpenXRAndroidDeviceAnchorPersistenceExtension::unpersist_anchor_tracker);
	ClassDB::bind_method(D_METHOD("unpersist_anchor_uuid", "uuid"), &OpenXRAndroidDeviceAnchorPersistenceExtension::unpersist_anchor_uuid);
	ClassDB::bind_method(D_METHOD("is_device_anchor_persistence_supported"), &OpenXRAndroidDeviceAnchorPersistenceExtension::is_device_anchor_persistence_supported);
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::_initialize_androidxr_device_persistence_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSupportedPersistenceAnchorTypesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateDeviceAnchorPersistenceANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyDeviceAnchorPersistenceANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrPersistAnchorANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetAnchorPersistStateANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePersistedAnchorSpaceANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumeratePersistedAnchorsANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrUnpersistAnchorANDROID);
	return true;
}

bool OpenXRAndroidDeviceAnchorPersistenceExtension::_is_xrspace_location_valid(XrSpace p_xrspace) const {
	XrSpaceLocation xrlocation;
	if (!OpenXRAndroidAnchorTracker::get_xranchor_space_location(p_xrspace, xrlocation)) {
		return false;
	}

	if (0 == (xrlocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) || 0 == (xrlocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)) {
		return false;
	}

	return true;
}
