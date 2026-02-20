/**************************************************************************/
/*  openxr_android_device_anchor_persistence_extension.h                  */
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
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/string.hpp>

#include "classes/openxr_android_anchor_tracker.h"

#include "util.h"

using namespace godot;

class OpenXRAndroidDeviceAnchorPersistenceExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidDeviceAnchorPersistenceExtension, OpenXRExtensionWrapper);

public:
	static OpenXRAndroidDeviceAnchorPersistenceExtension *get_singleton();

	OpenXRAndroidDeviceAnchorPersistenceExtension();
	virtual ~OpenXRAndroidDeviceAnchorPersistenceExtension() override;

	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_session_created(uint64_t p_session_instance) override;
	virtual void _on_process() override;
	virtual void _on_session_destroyed() override;

	bool set_restore_persisted_anchors_cooldown(int p_cooldown);
	bool restore_persisted_anchor_trackers();

	TypedArray<StringName> get_all_persisted_anchors();
	Ref<OpenXRAndroidAnchorTracker> create_persisted_anchor_tracker(StringName p_uuid);

	bool persist_anchor_tracker(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker) const;
	bool persist_xranchor(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, StringName &o_uuid, XrUuidEXT &o_xruuid);

	OpenXRAndroidAnchorTracker::PersistState get_anchor_tracker_persist_state_uuid(StringName p_uuid, bool p_update = false);
	OpenXRAndroidAnchorTracker::PersistState get_anchor_tracker_persist_state(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, bool p_update = false) const;
	OpenXRAndroidAnchorTracker::PersistState get_xranchor_persist_state(const StringName &p_uuid, const XrUuidEXT &p_xruuid);

	TypedArray<OpenXRAndroidAnchorTracker> get_persisted_anchor_trackers() const;

	bool unpersist_anchor_uuid(StringName p_uuid);
	bool unpersist_anchor_tracker(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker) const;
	bool unpersist_xranchor(const StringName &p_uuid, const XrUuidEXT &p_xruuid);

	void on_xranchor_tracker_destroyed(const StringName &p_uuid);

	bool is_device_anchor_persistence_supported() const;

protected:
	static void _bind_methods();

private:
	static OpenXRAndroidDeviceAnchorPersistenceExtension *singleton;

	bool _initialize_androidxr_device_persistence_extension();
	XrSpace _create_persisted_anchor_tracker(StringName p_uuid);
	TypedArray<StringName> _get_all_persisted_anchors(bool &o_success);
	XrDeviceAnchorPersistenceANDROID _get_or_create_device_anchor_persistence();
	bool _is_xrspace_location_valid(XrSpace p_xrspace) const;

	HashMap<String, bool *> request_extensions;
	bool available = false;
	XrSystemDeviceAnchorPersistencePropertiesANDROID anchor_persistence_properties;
	XrDeviceAnchorPersistenceANDROID device_anchor_persistence = XR_NULL_HANDLE;
	HashSet<XrTrackableTypeANDROID> supported_trackable_types;
	HashMap<StringName, Ref<OpenXRAndroidAnchorTracker>> restored_persisted_anchor_trackers;

	bool already_retrieved_persisted_anchors_to_restore = false;
	HashMap<StringName, XrSpace> remaining_persisted_anchors_to_restore;
	int persisted_anchor_discovery_cooldown = 60;
	int persisted_anchor_discovery_cooldown_cur = 0;

	EXT_PROTO_XRRESULT_FUNC5(xrEnumerateSupportedPersistenceAnchorTypesANDROID, (XrInstance), instance, (XrSystemId), systemId, (uint32_t), trackableTypeCapacityInput, (uint32_t *), trackableTypeCountOutput, (XrTrackableTypeANDROID *), trackableTypes);
	EXT_PROTO_XRRESULT_FUNC3(xrCreateDeviceAnchorPersistenceANDROID, (XrSession), session, (const XrDeviceAnchorPersistenceCreateInfoANDROID *), createInfo, (XrDeviceAnchorPersistenceANDROID *), outHandle);
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyDeviceAnchorPersistenceANDROID, (XrDeviceAnchorPersistenceANDROID), handle);
	EXT_PROTO_XRRESULT_FUNC3(xrPersistAnchorANDROID, (XrDeviceAnchorPersistenceANDROID), handle, (const XrPersistedAnchorSpaceInfoANDROID *), persistedInfo, (XrUuidEXT *), anchorIdOutput);
	EXT_PROTO_XRRESULT_FUNC3(xrGetAnchorPersistStateANDROID, (XrDeviceAnchorPersistenceANDROID), handle, (const XrUuidEXT *), anchorId, (XrAnchorPersistStateANDROID *), persistState);
	EXT_PROTO_XRRESULT_FUNC3(xrCreatePersistedAnchorSpaceANDROID, (XrDeviceAnchorPersistenceANDROID), handle, (const XrPersistedAnchorSpaceCreateInfoANDROID *), createInfo, (XrSpace *), anchorOutput);
	EXT_PROTO_XRRESULT_FUNC4(xrEnumeratePersistedAnchorsANDROID, (XrDeviceAnchorPersistenceANDROID), handle, (uint32_t), anchorIdsCapacityInput, (uint32_t *), anchorIdsCountOutput, (XrUuidEXT *), anchorIds);
	EXT_PROTO_XRRESULT_FUNC2(xrUnpersistAnchorANDROID, (XrDeviceAnchorPersistenceANDROID), handle, (const XrUuidEXT *), anchorId);
};
