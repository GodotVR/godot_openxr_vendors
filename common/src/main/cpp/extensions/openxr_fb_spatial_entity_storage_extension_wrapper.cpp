/**************************************************************************/
/*  openxr_fb_spatial_entity_storage_extension_wrapper.cpp                */
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

#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"
#include "utils/xr_godot_utils.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/object.hpp>

#define LOG_FAIL(EVENT, ERR) if(EVENT) WARN_PRINT(String(ERR) + String(" - ") + get_openxr_api()->get_error_string(EVENT->result)); else WARN_PRINT(String(ERR) + String(" - NULL EVENT"))
#define FAILED(EVENT) !EVENT || !XR_SUCCEEDED(EVENT->result)

using namespace godot;

static const uint32_t MAX_PERSISTENT_SPACES = 100;

OpenXRFbSpatialEntityStorageExtensionWrapper *OpenXRFbSpatialEntityStorageExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityStorageExtensionWrapper *OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityStorageExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityStorageExtensionWrapper::OpenXRFbSpatialEntityStorageExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntityStorageExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME] = &fb_spatial_entity_storage_ext;
	singleton = this;
}

OpenXRFbSpatialEntityStorageExtensionWrapper::~OpenXRFbSpatialEntityStorageExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_storage_supported"), &OpenXRFbSpatialEntityStorageExtensionWrapper::is_spatial_entity_storage_supported);
	ClassDB::bind_method(D_METHOD("create_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::create_persistent_anchor);
	ClassDB::bind_method(D_METHOD("delete_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::delete_persistent_anchor);
	ClassDB::bind_method(D_METHOD("start_tracking_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::start_tracking_persistent_anchor);
	ClassDB::bind_method(D_METHOD("stop_tracking_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::stop_tracking_persistent_anchor);

	ADD_SIGNAL(MethodInfo("create_persistent_anchor_failed", PropertyInfo(Variant::STRING, "tracker_name")));
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::cleanup() {
	fb_spatial_entity_storage_ext = false;
}

Dictionary OpenXRFbSpatialEntityStorageExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext: request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_storage_ext) {
		bool result = initialize_fb_spatial_entity_storage_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity_storage extension");
			fb_spatial_entity_storage_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::initialize_fb_spatial_entity_storage_extension(const XrInstance& p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrSaveSpaceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrEraseSpaceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateSpace);
	return true;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer*>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB) {
		on_save_complete((const XrEventDataSpaceSaveCompleteFB*) event);
		return true;
	}

	if (static_cast<const XrEventDataBuffer*>(event)->type == XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB) {
		on_erase_complete((const XrEventDataSpaceEraseCompleteFB*) event);
		return true;
	}

	return false;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::save_spatial_anchor(const XrSpaceSaveInfoFB* info, SaveCompleteCallback_t callback) {
	XrAsyncRequestIdFB requestId;
	const XrResult result = xrSaveSpaceFB(SESSION, info, &requestId);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrSaveSpaceFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		callback(nullptr);
		return;
	}
	save_complete_callbacks[requestId] = callback;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::erase_spatial_anchor(const XrSpaceEraseInfoFB* info, EraseCompleteCallback_t callback) {
	XrAsyncRequestIdFB requestId;
	const XrResult result = xrEraseSpaceFB(SESSION, info, &requestId);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrEraseSpaceFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		callback(nullptr);
		return;
	}
	erase_complete_callbacks[requestId] = callback;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::on_save_complete(const XrEventDataSpaceSaveCompleteFB* event) {
	if (!save_complete_callbacks.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB");
		return;
	}
	save_complete_callbacks[event->requestId](event);
	save_complete_callbacks.erase(event->requestId);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::on_erase_complete(const XrEventDataSpaceEraseCompleteFB* event) {
	if (!erase_complete_callbacks.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB");
		return;
	}
	erase_complete_callbacks[event->requestId](event);
	erase_complete_callbacks.erase(event->requestId);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::start_tracking_persistent_anchor(const String& uuid, const String& tracker_name) {
	OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities_by_uuid(uuid,[=](Vector<XrSpaceQueryResultFB> results) {
		if (results.size() == 1) {
			anchor_spaces[uuid] = results[0].space;

			Ref<XRPositionalTracker> positional_tracker;
			positional_tracker.instantiate();
			positional_tracker->set_tracker_type(XRServer::TRACKER_ANCHOR);
			positional_tracker->set_tracker_name(tracker_name);
			positional_tracker->set_input("uuid", uuid);

			XRServer *xr_server = XRServer::get_singleton();
			xr_server->add_tracker(positional_tracker);
			trackers[uuid] = positional_tracker;
		} else {
			WARN_PRINT("Could not find anchor" + uuid);
		}
	});
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::stop_tracking_persistent_anchor(const String& uuid) {
	if (trackers.has(uuid)) {
		XRServer *xr_server = XRServer::get_singleton();
		xr_server->remove_tracker(trackers[uuid]);
		trackers.erase(uuid);
		anchor_spaces.erase(uuid);
	} else {
		WARN_PRINT("Trying to delete unknown tracker: " + uuid);
	}
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::create_persistent_anchor(const Transform3D &transform, const String& tracker_name) {
	XrTime display_time = (XrTime) get_openxr_api()->get_next_frame_time();
	XrSpace play_space = (XrSpace) get_openxr_api()->get_play_space();

	// First create the anchor for this session
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->create_spatial_anchor(
		play_space, display_time, XrGodotUtils::transformToPose(transform),
		[tracker_name, this](const XrEventDataSpatialAnchorCreateCompleteFB* createResult) {
			if (FAILED(createResult)) {
				LOG_FAIL(createResult, "Unable to create XrSpace");
				emit_signal("create_persistent_anchor_failed", tracker_name);
				return;
			}
			// Next enable storable
			OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(
				createResult->space, XR_SPACE_COMPONENT_TYPE_STORABLE_FB, true,
				[tracker_name, this](const XrEventDataSpaceSetStatusCompleteFB* enableResult) {
					if (FAILED(enableResult)) {
						LOG_FAIL(enableResult, "Unable to enable XR_SPACE_COMPONENT_TYPE_STORABLE_FB for XrSpace");
						emit_signal("create_persistent_anchor_failed", tracker_name);
						return;
					}

					// Finally, save the anchor
					XrSpaceSaveInfoFB saveInfo = {
						XR_TYPE_SPACE_SAVE_INFO_FB,
						nullptr,
						enableResult->space,
						XR_SPACE_STORAGE_LOCATION_LOCAL_FB,
						XR_SPACE_PERSISTENCE_MODE_INDEFINITE_FB
					};
					OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton()->save_spatial_anchor(
						&saveInfo,
						[tracker_name, this](const XrEventDataSpaceSaveCompleteFB* saveResult) {
							if (FAILED(saveResult)) {
								LOG_FAIL(saveResult, "Unable to save XrSpace");
								emit_signal("create_persistent_anchor_failed", tracker_name);
								return;
							}
							start_tracking_persistent_anchor(String(XrGodotUtils::uuidToString(saveResult->uuid).c_str()), tracker_name);
					});
				});
		}
	);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::delete_persistent_anchor(const String& uuid) {
	OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities_by_uuid(uuid, [=](Vector<XrSpaceQueryResultFB> results) {
		if (results.size() == 1) {
			stop_tracking_persistent_anchor(uuid);
			if (OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(
				results[0].space, XR_SPACE_COMPONENT_TYPE_STORABLE_FB
			)) {
				XrSpaceEraseInfoFB info = {
					XR_TYPE_SPACE_ERASE_INFO_FB,
					nullptr,
					results[0].space,
					XR_SPACE_STORAGE_LOCATION_LOCAL_FB, // TODO: Don't hardcode this
				};

				erase_spatial_anchor(&info, [](const XrEventDataSpaceEraseCompleteFB* eventData){});
			} else {
				// Enable storable so we can delete it
				OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(
					results[0].space, XR_SPACE_COMPONENT_TYPE_STORABLE_FB, true,
					[results, this](const XrEventDataSpaceSetStatusCompleteFB* enableResult) {
						if (FAILED(enableResult)) {
							LOG_FAIL(enableResult, "Unable to enable XR_SPACE_COMPONENT_TYPE_STORABLE_FB for XrSpace");
							return;
						}

						XrSpaceEraseInfoFB info = {
							XR_TYPE_SPACE_ERASE_INFO_FB,
							nullptr,
							results[0].space,
							XR_SPACE_STORAGE_LOCATION_LOCAL_FB, // TODO: Don't hardcode this
						};

						erase_spatial_anchor(&info, [](const XrEventDataSpaceEraseCompleteFB* eventData){});
					});
			}
		} else {
			WARN_PRINT("Could not find anchor: " + uuid);
		}
	});
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_process() {
	if (!fb_spatial_entity_storage_ext) {
		return;
	}

	// Locate every known anchor and update the corresponding tracker's position
	for (auto space: anchor_spaces) {
		XrSpaceLocation location = {
			XR_TYPE_SPACE_LOCATION, // type
			nullptr, // next
			0, // locationFlags
			{
					{ 0.0, 0.0, 0.0, 0.0 }, // orientation
					{ 0.0, 0.0, 0.0 } // position
			} // pose
		};

		xrLocateSpace(
			space.value,
			(XrSpace) get_openxr_api()->get_play_space(),
			get_openxr_api()->get_next_frame_time(),
			&location);

		trackers[space.key]->set_pose(
			"default",
			XrGodotUtils::poseToTransform(location.pose),
			Vector3 {},
			Vector3 {},
			XRPose::XR_TRACKING_CONFIDENCE_HIGH);
	}
}
