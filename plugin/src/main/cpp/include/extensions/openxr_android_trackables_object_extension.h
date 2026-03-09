/**************************************************************************/
/*  openxr_android_trackables_object_extension.h                          */
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
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/variant/string.hpp>

#include "classes/openxr_android_trackable_tracker.h"
#include "util.h"

using namespace godot;

class OpenXRAndroidTrackablesObjectExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidTrackablesObjectExtension, OpenXRExtensionWrapper);

public:
	static OpenXRAndroidTrackablesObjectExtension *get_singleton();
	static int get_next_object_tracker_id();

	OpenXRAndroidTrackablesObjectExtension();
	virtual ~OpenXRAndroidTrackablesObjectExtension() override;

	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_process() override;
	virtual void _on_session_destroyed() override;

	bool is_trackables_object_supported() const;
	void set_default_object_context_enabled(bool p_enabled);
	void set_object_tracker_discovery_cooldown(int p_cooldown);
	void discover_object_trackers(bool p_update_trackers, RID p_object_context = RID());
	RID get_default_object_context();
	RID create_object_context(Array p_object_labels);
	RID get_object_context(XrTrackableTrackerANDROID p_xrtrackable_tracker);
	void free_object_context(RID p_object_context);

	EXT_PROTO_XRRESULT_FUNC3(xrGetTrackableObjectANDROID, (XrTrackableTrackerANDROID), trackableTracker, (const XrTrackableGetInfoANDROID *), getInfo, (XrTrackableObjectANDROID *), objectOutput);

protected:
	static void _bind_methods();

private:
	static OpenXRAndroidTrackablesObjectExtension *singleton;

	bool _initialize_openxr_android_trackables_object_extension();
	RID _make_object_context(XrTrackableTrackerANDROID p_xrtrackable_tracker);

	HashMap<String, bool *> request_extensions;
	bool available = false;

	int object_trackable_discovery_cooldown = 60;
	int object_trackable_discovery_cooldown_cur = 0;
	bool default_object_context_enabled = true;
	RID default_object_context;

	struct ObjectContext {
		XrTrackableTrackerANDROID xrtrackable_tracker;
		HashMap<XrTrackableANDROID, Ref<OpenXRAndroidTrackableTracker>> xrtrackable_to_tracker;
	};
	RID_Owner<ObjectContext> object_context_owner;

	int next_object_tracker_id = 1;
};
