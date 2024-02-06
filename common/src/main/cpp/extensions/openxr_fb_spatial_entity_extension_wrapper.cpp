/**************************************************************************/
/*  openxr_fb_spatial_entity_extension_wrapper.cpp                        */
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

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace {

static const std::map<XrSpaceComponentTypeFB, std::string> component_names = {
	{XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB, "XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB"},
	{XR_SPACE_COMPONENT_TYPE_STORABLE_FB, "XR_SPACE_COMPONENT_TYPE_STORABLE_FB"},
	{XR_SPACE_COMPONENT_TYPE_SHARABLE_FB, "XR_SPACE_COMPONENT_TYPE_SHARABLE_FB"},
	{XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB, "XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB"},
	{XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB, "XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB"},
	{XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB, "XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB"},
	{XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB, "XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB"},
	{XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB, "XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB"},
};

} // anonymous namespace

using namespace godot;

OpenXRFbSpatialEntityExtensionWrapper *OpenXRFbSpatialEntityExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityExtensionWrapper *OpenXRFbSpatialEntityExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityExtensionWrapper::OpenXRFbSpatialEntityExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntityExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_EXTENSION_NAME] = &fb_spatial_entity_ext;
	singleton = this;
}

OpenXRFbSpatialEntityExtensionWrapper::~OpenXRFbSpatialEntityExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_supported"), &OpenXRFbSpatialEntityExtensionWrapper::is_spatial_entity_supported);
}

void OpenXRFbSpatialEntityExtensionWrapper::cleanup() {
	fb_spatial_entity_ext = false;
}

Dictionary OpenXRFbSpatialEntityExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_ext) {
		bool result = initialize_fb_spatial_entity_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity extension");
			fb_spatial_entity_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityExtensionWrapper::initialize_fb_spatial_entity_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSpatialAnchorFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceUuidFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSpaceSupportedComponentsFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetSpaceComponentStatusFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceComponentStatusFB);

	return true;
}

bool OpenXRFbSpatialEntityExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPATIAL_ANCHOR_CREATE_COMPLETE_FB) {
		auto eventData = (const XrEventDataSpatialAnchorCreateCompleteFB *)event;
		if (create_anchor_callbacks.has(eventData->requestId)) {
			create_anchor_callbacks[eventData->requestId](eventData);
			create_anchor_callbacks.erase(eventData->requestId);
		}
		return true;
	}

	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB) {
		auto eventData = (const XrEventDataSpaceSetStatusCompleteFB *)event;
		if (set_status_callbacks.has(eventData->requestId)) {
			set_status_callbacks[eventData->requestId](eventData);
			set_status_callbacks.erase(eventData->requestId);
		}
		return true;
	}

	return false;
}

void OpenXRFbSpatialEntityExtensionWrapper::create_spatial_anchor(
		const XrSpace& playSpace,
		const XrTime& frameTime,
		const XrPosef& poseInSpace,
		CreateSpatialAnchorCallback_t callback) {
	XrSpatialAnchorCreateInfoFB anchorCreateInfo = {
		XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_FB,
		nullptr,
		playSpace,
		poseInSpace,
		frameTime,
	};

	XrAsyncRequestIdFB requestId;
	if (!XR_SUCCEEDED(xrCreateSpatialAnchorFB(SESSION, &anchorCreateInfo, &requestId))) {
		callback(nullptr);
	}
	create_anchor_callbacks[requestId] = callback;
}

bool OpenXRFbSpatialEntityExtensionWrapper::is_component_supported(const XrSpace &space, XrSpaceComponentTypeFB type) {
	uint32_t numComponents = 0;
	xrEnumerateSpaceSupportedComponentsFB(space, 0, &numComponents, nullptr);
	Vector<XrSpaceComponentTypeFB> components;
	components.resize(numComponents);
	xrEnumerateSpaceSupportedComponentsFB(space, numComponents, &numComponents, components.ptrw());

	bool supported = false;
	for (uint32_t c = 0; c < numComponents; ++c) {
		if (components[c] == type) {
			supported = true;
			break;
		}
	}
	return supported;
}

void OpenXRFbSpatialEntityExtensionWrapper::print_supported_components(const XrSpace& space) {
	uint32_t numComponents = 0;
	xrEnumerateSpaceSupportedComponentsFB(space, 0, &numComponents, nullptr);
	Vector<XrSpaceComponentTypeFB> components;
	components.resize(numComponents);
	xrEnumerateSpaceSupportedComponentsFB(space, numComponents, &numComponents, components.ptrw());

	String output("Supported: ");
	for (uint32_t c = 0; c < numComponents; ++c) {
		if (component_names.count(components[c])) {
			output = output + String(", ") + String(component_names.at(components[c]).c_str());
		} else {
			output = output + String(", UNKOWN: [") + String(std::to_string((int) components[c]).c_str()) + String("]");
		}
	}
	WARN_PRINT(output);
}


bool OpenXRFbSpatialEntityExtensionWrapper::is_component_enabled(const XrSpace &space, XrSpaceComponentTypeFB type) {
	XrSpaceComponentStatusFB status = { XR_TYPE_SPACE_COMPONENT_STATUS_FB, nullptr };
	xrGetSpaceComponentStatusFB(space, type, &status);
	return (status.enabled && !status.changePending);
}

void OpenXRFbSpatialEntityExtensionWrapper::set_component_enabled(
		const XrSpace &space,
		XrSpaceComponentTypeFB type,
		bool status,
		std::optional<SetSpaceComponentStatusCallback_t> callback) {
	XrSpaceComponentStatusSetInfoFB request = {
		XR_TYPE_SPACE_COMPONENT_STATUS_SET_INFO_FB,
		nullptr,
		type,
		status,
		0,
	};
	XrAsyncRequestIdFB requestId;
	if (!XR_SUCCEEDED(xrSetSpaceComponentStatusFB(space, &request, &requestId))) {
		if (callback != std::nullopt) {
			(*callback)(nullptr);
		}
	}
	if (callback != std::nullopt) {
		set_status_callbacks[requestId] = *callback;
	}
}
