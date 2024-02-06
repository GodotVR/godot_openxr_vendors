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
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

#include <functional>
#include <map>
#include <optional>

using namespace godot;

typedef std::function<void(const XrEventDataSpaceSetStatusCompleteFB *eventData)> SetSpaceComponentStatusCallback_t;

// Wrapper for the set of Facebook XR spatial entity extension.
class OpenXRFbSpatialEntityExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntityExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	bool is_spatial_entity_supported() {
		return fb_spatial_entity_ext;
	}

	bool is_component_supported(const XrSpace &space, XrSpaceComponentTypeFB type);
	bool is_component_enabled(const XrSpace &space, XrSpaceComponentTypeFB type);

	// Attempts to set the enabled status for the given component of an XrSpace. The callback will
	// run to deliver results, with an arg of either:
	// - nullptr on immediate failure
	// - A valid XrEventDataSpaceSetStatusCompleteFB* delivered by the runtime later on
	// Both cases should be handled, and the second may still indicate an error depending on the
	// contained XrResult.
	void set_component_enabled(
			const XrSpace &space,
			XrSpaceComponentTypeFB type,
			bool status,
			std::optional<SetSpaceComponentStatusCallback_t> callback = std::nullopt);

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

	bool initialize_fb_spatial_entity_extension(const XrInstance &instance);

	HashMap<String, bool *> request_extensions;
	HashMap<XrAsyncRequestIdFB, SetSpaceComponentStatusCallback_t> set_status_callbacks;

	void cleanup();

	static OpenXRFbSpatialEntityExtensionWrapper *singleton;

	bool fb_spatial_entity_ext = false;
};
