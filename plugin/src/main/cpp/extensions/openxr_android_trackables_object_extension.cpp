/**************************************************************************/
/*  openxr_android_trackables_object_extension.cpp                        */
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

#include "extensions/openxr_android_trackables_object_extension.h"
#include "classes/openxr_android_trackable_object_tracker.h"
#include "extensions/openxr_android_trackables_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidTrackablesObjectExtension *OpenXRAndroidTrackablesObjectExtension::singleton = nullptr;

OpenXRAndroidTrackablesObjectExtension *OpenXRAndroidTrackablesObjectExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidTrackablesObjectExtension());
	}
	return singleton;
}

int OpenXRAndroidTrackablesObjectExtension::get_next_object_tracker_id() {
	OpenXRAndroidTrackablesObjectExtension *wrapper = OpenXRAndroidTrackablesObjectExtension::get_singleton();
	ERR_FAIL_NULL_V(wrapper, -1);

	int ret = wrapper->next_object_tracker_id;
	++wrapper->next_object_tracker_id;

	return ret;
}

OpenXRAndroidTrackablesObjectExtension::OpenXRAndroidTrackablesObjectExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidTrackablesObjectExtension singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_TRACKABLES_OBJECT_EXTENSION_NAME] = &available;
}

OpenXRAndroidTrackablesObjectExtension::~OpenXRAndroidTrackablesObjectExtension() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidTrackablesObjectExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRAndroidTrackablesObjectExtension::_on_instance_created(uint64_t p_instance) {
	if (!available) {
		return;
	}

	if (!_initialize_openxr_android_trackables_object_extension()) {
		UtilityFunctions::print("Failed to initialize trackables extension");
		available = false;
	}
}

void OpenXRAndroidTrackablesObjectExtension::_on_process() {
	if (!available || object_trackable_discovery_cooldown < 0) {
		return;
	}

	if (0 < object_trackable_discovery_cooldown_cur) {
		--object_trackable_discovery_cooldown_cur;
		return;
	}
	object_trackable_discovery_cooldown_cur = object_trackable_discovery_cooldown;

	discover_object_trackers(true);
}

void OpenXRAndroidTrackablesObjectExtension::_on_session_destroyed() {
	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	ERR_FAIL_NULL(wrapper);

	set_default_object_context_enabled(false);

	List<RID> object_context_rids;
	object_context_owner.get_owned_list(&object_context_rids);
	for (const RID &object_context_rid : object_context_rids) {
		free_object_context(object_context_rid);
	}
}

bool OpenXRAndroidTrackablesObjectExtension::is_trackables_object_supported() const {
	return available;
}

void OpenXRAndroidTrackablesObjectExtension::set_default_object_context_enabled(bool p_enabled) {
	if (default_object_context_enabled == p_enabled) {
		return;
	}

	default_object_context_enabled = p_enabled;
	if (!default_object_context_enabled) {
		free_object_context(default_object_context);
		default_object_context = RID();
	}
}

void OpenXRAndroidTrackablesObjectExtension::set_object_tracker_discovery_cooldown(int p_cooldown) {
	object_trackable_discovery_cooldown = p_cooldown;
	object_trackable_discovery_cooldown_cur = p_cooldown;
}

void OpenXRAndroidTrackablesObjectExtension::discover_object_trackers(bool p_update_trackers, RID p_object_context) {
	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	ERR_FAIL_NULL(wrapper);

	ObjectContext *object_context = object_context_owner.get_or_null(p_object_context);
	if (object_context == nullptr) {
		// ensure the default object context is created
		get_default_object_context();

		// update every object context (which includes the default object context)
		List<RID> object_context_rids;
		object_context_owner.get_owned_list(&object_context_rids);
		for (const RID &object_context_rid : object_context_rids) {
			object_context = object_context_owner.get_or_null(object_context_rid);
			wrapper->find_and_update_all_trackers(object_context->xrtrackable_tracker, XR_TRACKABLE_TYPE_OBJECT_ANDROID, p_update_trackers, object_context->xrtrackable_to_tracker);
		}
	} else {
		wrapper->find_and_update_all_trackers(object_context->xrtrackable_tracker, XR_TRACKABLE_TYPE_OBJECT_ANDROID, p_update_trackers, object_context->xrtrackable_to_tracker);
	}
}

RID OpenXRAndroidTrackablesObjectExtension::get_default_object_context() {
	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	ERR_FAIL_NULL_V(wrapper, RID());

	if (default_object_context_enabled && !default_object_context.is_valid()) {
		XrTrackableTrackerANDROID xrtrackable_tracker = wrapper->get_or_create_xrtrackable_tracker(XR_TRACKABLE_TYPE_OBJECT_ANDROID, XR_NULL_HANDLE, nullptr);
		default_object_context = _make_object_context(xrtrackable_tracker);
	}

	return default_object_context;
}

RID OpenXRAndroidTrackablesObjectExtension::create_object_context(Array p_object_labels) {
	LocalVector<XrObjectLabelANDROID> active_labels;
	for (int i = 0; i < p_object_labels.size(); ++i) {
		switch ((int)p_object_labels[i]) {
			case OpenXRAndroidTrackableObjectTracker::OBJECT_LABEL_UNKNOWN:
				active_labels.push_back(XR_OBJECT_LABEL_UNKNOWN_ANDROID);
				break;
			case OpenXRAndroidTrackableObjectTracker::OBJECT_LABEL_KEYBOARD:
				active_labels.push_back(XR_OBJECT_LABEL_KEYBOARD_ANDROID);
				break;
			case OpenXRAndroidTrackableObjectTracker::OBJECT_LABEL_MOUSE:
				active_labels.push_back(XR_OBJECT_LABEL_MOUSE_ANDROID);
				break;
			case OpenXRAndroidTrackableObjectTracker::OBJECT_LABEL_LAPTOP:
				active_labels.push_back(XR_OBJECT_LABEL_LAPTOP_ANDROID);
				break;
			default:
				WARN_PRINT(vformat("Got an unexpected object label: %d", p_object_labels[i]));
				break;
		}
	}

	ERR_FAIL_COND_V_MSG(active_labels.is_empty(), RID(), "At least one object label must be provided.");

	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	ERR_FAIL_NULL_V(wrapper, RID());

	XrTrackableObjectConfigurationANDROID object_configuration{
		XR_TYPE_TRACKABLE_OBJECT_CONFIGURATION_ANDROID, // type
		nullptr, // next
		active_labels.size(), // labelCount
		active_labels.ptr() // activeLabels
	};
	XrTrackableTrackerANDROID xrtrackable_tracker = wrapper->get_or_create_xrtrackable_tracker(XR_TRACKABLE_TYPE_OBJECT_ANDROID, XR_NULL_HANDLE, &object_configuration);
	return _make_object_context(xrtrackable_tracker);
}

RID OpenXRAndroidTrackablesObjectExtension::get_object_context(XrTrackableTrackerANDROID p_xrtrackable_tracker) {
	List<RID> object_context_rids;
	object_context_owner.get_owned_list(&object_context_rids);
	for (const RID &object_context_rid : object_context_rids) {
		ObjectContext *object_context = object_context_owner.get_or_null(object_context_rid);
		if (object_context != nullptr && object_context->xrtrackable_tracker == p_xrtrackable_tracker) {
			return object_context_rid;
		}
	}

	return RID();
}

void OpenXRAndroidTrackablesObjectExtension::free_object_context(RID p_object_context) {
	ObjectContext *object_context = object_context_owner.get_or_null(p_object_context);
	if (object_context == nullptr) {
		return;
	}

	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	ERR_FAIL_NULL(wrapper);

	wrapper->maybe_destroy_trackable_tracker(object_context->xrtrackable_tracker, object_context->xrtrackable_to_tracker);
	object_context_owner.free(p_object_context);
}

void OpenXRAndroidTrackablesObjectExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_trackables_object_supported"), &OpenXRAndroidTrackablesObjectExtension::is_trackables_object_supported);
	ClassDB::bind_method(D_METHOD("set_default_object_context_enabled", "enabled"), &OpenXRAndroidTrackablesObjectExtension::set_default_object_context_enabled);
	ClassDB::bind_method(D_METHOD("set_object_tracker_discovery_cooldown", "cooldown"), &OpenXRAndroidTrackablesObjectExtension::set_object_tracker_discovery_cooldown);
	ClassDB::bind_method(D_METHOD("discover_object_trackers", "update_trackers", "object_context"), &OpenXRAndroidTrackablesObjectExtension::discover_object_trackers, DEFVAL(RID()));
	ClassDB::bind_method(D_METHOD("get_default_object_context"), &OpenXRAndroidTrackablesObjectExtension::get_default_object_context);
	ClassDB::bind_method(D_METHOD("create_object_context", "object_labels"), &OpenXRAndroidTrackablesObjectExtension::create_object_context);
	ClassDB::bind_method(D_METHOD("free_object_context", "object_context"), &OpenXRAndroidTrackablesObjectExtension::free_object_context);
}

bool OpenXRAndroidTrackablesObjectExtension::_initialize_openxr_android_trackables_object_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetTrackableObjectANDROID);
	return true;
}

RID OpenXRAndroidTrackablesObjectExtension::_make_object_context(XrTrackableTrackerANDROID p_xrtrackable_tracker) {
	ObjectContext object_context = {
		p_xrtrackable_tracker, // xrtrackable_tracker
		{}, // xrtrackable_to_tracker
	};
	return object_context_owner.make_rid(object_context);
}
