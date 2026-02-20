/**************************************************************************/
/*  openxr_android_trackables_extension.h                                 */
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
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>

#include "classes/openxr_android_anchor_tracker.h"
#include "classes/openxr_android_trackable_tracker.h"
#include "util.h"

using namespace godot;

class OpenXRAndroidTrackablesExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidTrackablesExtension, OpenXRExtensionWrapper);

public:
	static OpenXRAndroidTrackablesExtension *get_singleton();
	static int get_next_tracker_id();

	OpenXRAndroidTrackablesExtension();
	virtual ~OpenXRAndroidTrackablesExtension() override;

	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_session_created(uint64_t p_session_instance) override;
	virtual void _on_process() override;
	virtual void _on_session_destroyed() override;

	XrTrackableTrackerANDROID get_or_create_xrtrackable_tracker(XrTrackableTypeANDROID p_xrtrackable_type);
	Ref<OpenXRAndroidTrackableTracker> get_or_create_tracker_and_update(XrTrackableANDROID p_xrtrackable, XrTrackableTypeANDROID p_xrtrackable_type, bool p_update_tracker);
	void set_trackable_discovery_cooldown(int p_cooldown);
	void discover_trackables(bool p_update_trackers);

	Ref<OpenXRAndroidAnchorTracker> create_anchor_tracker(const Transform3D &p_pose, Ref<OpenXRAndroidTrackableTracker> p_tracker);
	Ref<OpenXRAndroidAnchorTracker> xrcreate_anchor_tracker(XrSpace p_xrspace, const StringName &p_persist_uuid, Ref<OpenXRAndroidTrackableTracker> p_tracker);
	void destroy_anchor_tracker(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker);
	BitField<OpenXRAndroidAnchorTracker::LocationFlags> get_location_flags(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, bool p_update) const;
	Transform3D get_location_pose(Ref<OpenXRAndroidAnchorTracker> p_anchor_tracker, bool p_update) const;
	TypedArray<OpenXRAndroidAnchorTracker> get_anchor_trackers() const;
	void set_anchor_tracker_update_cooldown(int p_cooldown);
	void update_anchor_trackers();
	bool can_create_more_anchors() const;

	bool is_trackables_supported() const;

	EXT_PROTO_XRRESULT_FUNC3(xrGetTrackablePlaneANDROID, (XrTrackableTrackerANDROID), trackableTracker, (const XrTrackableGetInfoANDROID *), getInfo, (XrTrackablePlaneANDROID *), planeOutput);
	EXT_PROTO_XRRESULT_FUNC3(xrGetTrackableObjectANDROID, (XrTrackableTrackerANDROID), trackableTracker, (const XrTrackableGetInfoANDROID *), getInfo, (XrTrackableObjectANDROID *), objectOutput);
	EXT_PROTO_XRRESULT_FUNC4(xrLocateSpace, (XrSpace), space, (XrSpace), baseSpace, (XrTime), time, (XrSpaceLocation *), location);

protected:
	static void _bind_methods();

private:
	static OpenXRAndroidTrackablesExtension *singleton;

	bool _initialize_openxr_android_trackables_extension();
	void _maybe_destroy_trackable_tracker(XrTrackableTrackerANDROID *trackable_tracker);
	void _find_and_update_all_trackers(XrTrackableTypeANDROID p_xrtrackable_type, bool p_update_trackers, HashMap<XrTrackableANDROID, Ref<OpenXRAndroidTrackableTracker>> &p_trackables_to_delete);
	void _on_process_anchors();
	void _on_process_trackables();

	HashMap<String, XrPath> paths;
	HashMap<String, bool *> request_extensions;
	bool available = false;
	bool obj_available = false;
	XrSystemTrackablesPropertiesANDROID system_trackables_properties = {
		XR_TYPE_SYSTEM_TRACKABLES_PROPERTIES_ANDROID, // type
		nullptr, // next
		XR_FALSE, // supportsAnchor
		0, // maxAnchors
	};

	LocalVector<XrTrackableTypeANDROID> supported_trackable_types;
	HashMap<XrTrackableANDROID, Ref<OpenXRAndroidTrackableTracker>> current_trackables;
	int trackable_discovery_cooldown = 60;
	int trackable_discovery_cooldown_cur = 0;
	XrTrackableTrackerANDROID plane_trackable_tracker = XR_NULL_HANDLE;
	XrTrackableTrackerANDROID depth_trackable_tracker = XR_NULL_HANDLE;
	XrTrackableTrackerANDROID object_trackable_tracker = XR_NULL_HANDLE;

	LocalVector<XrTrackableTypeANDROID> supported_anchor_trackable_types;
	HashMap<XrSpace, Ref<OpenXRAndroidAnchorTracker>> current_anchor_trackers;
	int next_tracker_id = 1;
	int anchor_update_cooldown = 60;
	int anchor_update_cooldown_cur = 0;

	EXT_PROTO_XRRESULT_FUNC5(xrEnumerateSupportedTrackableTypesANDROID, (XrInstance), instance, (XrSystemId), systemId, (uint32_t), trackableTypeCapacityInput, (uint32_t *), trackableTypeCountOutput, (XrTrackableTypeANDROID *), trackableTypes);
	EXT_PROTO_XRRESULT_FUNC5(xrEnumerateSupportedAnchorTrackableTypesANDROID, (XrInstance), instance, (XrSystemId), systemId, (uint32_t), trackableTypeCapacityInput, (uint32_t *), trackableTypeCountOutput, (XrTrackableTypeANDROID *), trackableTypes);
	EXT_PROTO_XRRESULT_FUNC3(xrCreateTrackableTrackerANDROID, (XrSession), session, (const XrTrackableTrackerCreateInfoANDROID *), createInfo, (XrTrackableTrackerANDROID *), trackableTracker);
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyTrackableTrackerANDROID, (XrTrackableTrackerANDROID), trackableTracker);
	EXT_PROTO_XRRESULT_FUNC4(xrGetAllTrackablesANDROID, (XrTrackableTrackerANDROID), trackableTracker, (uint32_t), trackableCapacityCount, (uint32_t *), trackableCountOutput, (XrTrackableANDROID *), trackablesOutput);
	EXT_PROTO_XRRESULT_FUNC3(xrCreateAnchorSpaceANDROID, (XrSession), session, (const XrAnchorSpaceCreateInfoANDROID *), createInfo, (XrSpace *), anchorOutput);
	EXT_PROTO_XRRESULT_FUNC1(xrDestroySpace, (XrSpace), space);
};
