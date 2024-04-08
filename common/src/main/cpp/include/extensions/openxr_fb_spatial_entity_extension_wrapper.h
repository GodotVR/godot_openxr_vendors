/**************************************************************************/
/*  openxr_fb_spatial_entity_extension_wrapper.h                          */
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

#pragma once

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook XR spatial entity extension.
class OpenXRFbSpatialEntityExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntityExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;
	void _on_process() override;

	bool is_spatial_entity_supported() {
		return fb_spatial_entity_ext;
	}

	typedef void (*SetComponentEnabledCallback)(XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata);

	Vector<XrSpaceComponentTypeFB> get_support_components(const XrSpace &p_space);
	bool is_component_enabled(const XrSpace &p_space, XrSpaceComponentTypeFB p_component);
	bool set_component_enabled(const XrSpace &p_space, XrSpaceComponentTypeFB p_component, bool p_enabled, SetComponentEnabledCallback p_callback, void *p_userdata);

	void track_entity(const StringName &p_name, const XrSpace &p_space);
	void untrack_entity(const StringName &p_name);
	bool is_entity_tracked(const StringName &p_name) const;

	virtual bool _on_event_polled(const void *event) override;

	static OpenXRFbSpatialEntityExtensionWrapper *get_singleton();

	OpenXRFbSpatialEntityExtensionWrapper();
	~OpenXRFbSpatialEntityExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateSpatialAnchorFB,
			(XrSession), session,
			(const XrSpatialAnchorCreateInfoFB *), request,
			(XrAsyncRequestIdFB *), requestId)

	EXT_PROTO_XRRESULT_FUNC2(xrGetSpaceUuidFB,
			(XrSpace), space,
			(XrUuidEXT *), uuid)

	EXT_PROTO_XRRESULT_FUNC4(xrEnumerateSpaceSupportedComponentsFB,
			(XrSpace), space,
			(uint32_t), componentTypeCapacityInput,
			(uint32_t *), componentTypeCountOutput,
			(XrSpaceComponentTypeFB *), componentTypes)

	EXT_PROTO_XRRESULT_FUNC3(xrSetSpaceComponentStatusFB,
			(XrSpace), space,
			(const XrSpaceComponentStatusSetInfoFB *), info,
			(XrAsyncRequestIdFB *), requestId)

	EXT_PROTO_XRRESULT_FUNC3(xrGetSpaceComponentStatusFB,
			(XrSpace), space,
			(XrSpaceComponentTypeFB), componentType,
			(XrSpaceComponentStatusFB *), status)

	EXT_PROTO_XRRESULT_FUNC4(xrLocateSpace,
		(XrSpace), space,
		(XrSpace), baseSpace,
		(XrTime), time,
		(XrSpaceLocation *), location)

	bool initialize_fb_spatial_entity_extension(const XrInstance &instance);
	void on_set_component_enabled_complete(const XrEventDataSpaceSetStatusCompleteFB *event);

	HashMap<String, bool *> request_extensions;

	struct SetComponentEnabledInfo {
		SetComponentEnabledCallback callback = nullptr;
		void *userdata = nullptr;

		SetComponentEnabledInfo() { }

		SetComponentEnabledInfo(SetComponentEnabledCallback p_callback, void *p_userdata) {
			callback = p_callback;
			userdata = p_userdata;
		}
	};
	HashMap<XrAsyncRequestIdFB, SetComponentEnabledInfo> set_component_enabled_info;

	struct TrackedEntity {
		XrSpace space = XR_NULL_HANDLE;
		Ref<XRPositionalTracker> tracker;

		TrackedEntity(XrSpace p_space) {
			space = p_space;
		}

		TrackedEntity() {};
	};
	HashMap<StringName, TrackedEntity> tracked_entities;

	void cleanup();

	static OpenXRFbSpatialEntityExtensionWrapper *singleton;

	bool fb_spatial_entity_ext = false;
};
