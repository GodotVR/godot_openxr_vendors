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
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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

void OpenXRFbSpatialEntityExtensionWrapper::_on_process() {
	for (KeyValue<StringName, TrackedEntity> &E : tracked_entities) {
		if (E.value.tracker.is_null()) {
			E.value.tracker.instantiate();
			E.value.tracker->set_tracker_name(E.key);
			E.value.tracker->set_tracker_desc(String("Anchor ") + E.key);
			E.value.tracker->set_tracker_type(XRServer::TRACKER_ANCHOR);
			XRServer::get_singleton()->add_tracker(E.value.tracker);
		}

		XrSpaceLocation location = {
			XR_TYPE_SPACE_LOCATION, // type
			nullptr, // next
			0, // locationFlags
			{
					{ 0.0, 0.0, 0.0, 0.0 }, // orientation
					{ 0.0, 0.0, 0.0 } // position
			} // pose
		};

		XrResult result = xrLocateSpace(E.value.space, reinterpret_cast<XrSpace>(get_openxr_api()->get_play_space()), get_openxr_api()->get_predicted_display_time(), &location);
		if (XR_FAILED(result)) {
			WARN_PRINT("OpenXR: failed to locate anchor " + E.key);
			WARN_PRINT(get_openxr_api()->get_error_string(result));
			continue;
		}

		if ((location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) && (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
			Transform3D transform(
					Basis(Quaternion(location.pose.orientation.x, location.pose.orientation.y, location.pose.orientation.z, location.pose.orientation.w)),
					Vector3(location.pose.position.x, location.pose.position.y, location.pose.position.z));

			E.value.tracker->set_pose("default", transform, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);
		} else {
			Ref<XRPose> default_pose = E.value.tracker->get_pose("default");
			if (default_pose.is_valid()) {
				// Set the tracking confidence to none, while maintaining the existing transform.
				default_pose->set_tracking_confidence(XRPose::XR_TRACKING_CONFIDENCE_NONE);
			} else {
				E.value.tracker->set_pose("default", Transform3D(), Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_NONE);
			}
		}
	}
}

bool OpenXRFbSpatialEntityExtensionWrapper::initialize_fb_spatial_entity_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSpatialAnchorFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceUuidFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSpaceSupportedComponentsFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetSpaceComponentStatusFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceComponentStatusFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySpace);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateSpace);

	return true;
}

bool OpenXRFbSpatialEntityExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPATIAL_ANCHOR_CREATE_COMPLETE_FB) {
		on_spatial_anchor_created((const XrEventDataSpatialAnchorCreateCompleteFB *)event);
		return true;
	}

	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB) {
		on_set_component_enabled_complete((const XrEventDataSpaceSetStatusCompleteFB *)event);
		return true;
	}

	return false;
}

bool OpenXRFbSpatialEntityExtensionWrapper::create_spatial_anchor(const Transform3D &p_transform, SpatialAnchorCreatedCallback p_callback, void *p_userdata) {
    XrAsyncRequestIdFB request_id = 0;

    Quaternion quat = Quaternion(p_transform.basis);
    Vector3 pos = p_transform.origin;
    XrPosef pose = {
        {
            static_cast<float>(quat.x),
            static_cast<float>(quat.y),
            static_cast<float>(quat.z),
            static_cast<float>(quat.w)
        },
        {
            static_cast<float>(pos.x),
            static_cast<float>(pos.y),
            static_cast<float>(pos.z)
        }
    };

    XrSpatialAnchorCreateInfoFB info = {
        XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_FB, // type
        nullptr, // next
        reinterpret_cast<XrSpace>(get_openxr_api()->get_play_space()), // space
        pose, // poseInSpace
        get_openxr_api()->get_predicted_display_time(), // time
    };

    const XrResult result = xrCreateSpatialAnchorFB(SESSION, &info, &request_id);
    if (!XR_SUCCEEDED(result)) {
        WARN_PRINT("xrCreateSpatialAnchorFB failed!");
        WARN_PRINT(get_openxr_api()->get_error_string(result));
        p_callback(result, nullptr, nullptr, p_userdata);
        return false;
    }

    spatial_anchor_creation_info[request_id] = SpatialAnchorCreationInfo(p_callback, p_userdata);
    return true;
}

void OpenXRFbSpatialEntityExtensionWrapper::on_spatial_anchor_created(const XrEventDataSpatialAnchorCreateCompleteFB *event) {
	if (!spatial_anchor_creation_info.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPATIAL_ANCHOR_CREATE_COMPLETE_FB");
		return;
	}

	SpatialAnchorCreationInfo *info = spatial_anchor_creation_info.getptr(event->requestId);
	info->callback(event->result, event->space, &event->uuid, info->userdata);
	spatial_anchor_creation_info.erase(event->requestId);
}

bool OpenXRFbSpatialEntityExtensionWrapper::destroy_space(const XrSpace &p_space) {
	return XR_SUCCEEDED(xrDestroySpace(p_space));
}

Vector<XrSpaceComponentTypeFB> OpenXRFbSpatialEntityExtensionWrapper::get_support_components(const XrSpace &space) {
	Vector<XrSpaceComponentTypeFB> components;

	uint32_t numComponents = 0;
	xrEnumerateSpaceSupportedComponentsFB(space, 0, &numComponents, nullptr);
	components.resize(numComponents);
	xrEnumerateSpaceSupportedComponentsFB(space, numComponents, &numComponents, components.ptrw());

	return components;
}

bool OpenXRFbSpatialEntityExtensionWrapper::is_component_enabled(const XrSpace &space, XrSpaceComponentTypeFB type) {
	XrSpaceComponentStatusFB status = { XR_TYPE_SPACE_COMPONENT_STATUS_FB, nullptr };
	xrGetSpaceComponentStatusFB(space, type, &status);
	return (status.enabled && !status.changePending);
}

bool OpenXRFbSpatialEntityExtensionWrapper::set_component_enabled(const XrSpace &p_space, XrSpaceComponentTypeFB p_component, bool p_enabled, SetComponentEnabledCallback p_callback, void *p_userdata) {
	XrSpaceComponentStatusSetInfoFB request = {
		XR_TYPE_SPACE_COMPONENT_STATUS_SET_INFO_FB,
		nullptr,
		p_component,
		p_enabled,
		0,
	};
	XrAsyncRequestIdFB request_id = 0;
	XrResult result = xrSetSpaceComponentStatusFB(p_space, &request, &request_id);
	if (!XR_SUCCEEDED(result)) {
		p_callback(result, p_component, p_enabled, p_userdata);
		return false;
	}

	set_component_enabled_info[request_id] = SetComponentEnabledInfo(p_callback, p_userdata);

	return true;
}

void OpenXRFbSpatialEntityExtensionWrapper::on_set_component_enabled_complete(const XrEventDataSpaceSetStatusCompleteFB *event) {
	if (!set_component_enabled_info.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB");
		return;
	}

	SetComponentEnabledInfo *info = set_component_enabled_info.getptr(event->requestId);
	info->callback(event->result, event->componentType, event->enabled, info->userdata);
	set_component_enabled_info.erase(event->requestId);
}

void OpenXRFbSpatialEntityExtensionWrapper::track_entity(const StringName &p_name, const XrSpace &p_space) {
	tracked_entities[p_name] = TrackedEntity(p_space);
}

void OpenXRFbSpatialEntityExtensionWrapper::untrack_entity(const StringName &p_name) {
	TrackedEntity *entity = tracked_entities.getptr(p_name);
	if (entity) {
		if (entity->tracker.is_valid()) {
			XRServer::get_singleton()->remove_tracker(entity->tracker);
			entity->tracker.unref();
		}
		tracked_entities.erase(p_name);
	}
}

bool OpenXRFbSpatialEntityExtensionWrapper::is_entity_tracked(const StringName &p_name) const {
	return tracked_entities.has(p_name);
}
