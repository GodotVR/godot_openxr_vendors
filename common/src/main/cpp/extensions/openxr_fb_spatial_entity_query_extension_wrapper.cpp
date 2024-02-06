/**************************************************************************/
/*  openxr_fb_spatial_entity_query_extension_wrapper.cpp                  */
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

#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "utils/xr_godot_utils.h"

using namespace godot;

static const uint32_t MAX_PERSISTENT_SPACES = 100;

OpenXRFbSpatialEntityQueryExtensionWrapper *OpenXRFbSpatialEntityQueryExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityQueryExtensionWrapper *OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityQueryExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityQueryExtensionWrapper::OpenXRFbSpatialEntityQueryExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntityQueryExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_QUERY_EXTENSION_NAME] = &fb_spatial_entity_query_ext;
	singleton = this;
}

OpenXRFbSpatialEntityQueryExtensionWrapper::~OpenXRFbSpatialEntityQueryExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_query_supported"), &OpenXRFbSpatialEntityQueryExtensionWrapper::is_spatial_entity_query_supported);
	ClassDB::bind_method(D_METHOD("query_persisted_anchors"), &OpenXRFbSpatialEntityQueryExtensionWrapper::query_persisted_anchors);
	ADD_SIGNAL(MethodInfo("persisted_anchors_found", PropertyInfo(Variant::PACKED_STRING_ARRAY, "name")));
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::cleanup() {
	fb_spatial_entity_query_ext = false;
}

Dictionary OpenXRFbSpatialEntityQueryExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_query_ext) {
		bool result = initialize_fb_spatial_entity_query_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity_query extension");
			fb_spatial_entity_query_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityQueryExtensionWrapper::initialize_fb_spatial_entity_query_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrQuerySpacesFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrRetrieveSpaceQueryResultsFB);

	return true;
}

bool OpenXRFbSpatialEntityQueryExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB) {
		on_space_query_results((const XrEventDataSpaceQueryResultsAvailableFB *)event);
		return true;
	}

	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB) {
		on_space_query_complete((const XrEventDataSpaceQueryCompleteFB *)event);
		return true;
	}

	return false;
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::query_persisted_anchors() {
	XrSpaceQueryInfoFB queryInfo = {
			XR_TYPE_SPACE_QUERY_INFO_FB,
			nullptr,
			XR_SPACE_QUERY_ACTION_LOAD_FB,
			MAX_PERSISTENT_SPACES,
			0,
			nullptr,
			nullptr};
	query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB*) &queryInfo, [=](Vector<XrSpaceQueryResultFB> results) {
		PackedStringArray found_anchor_uuids;
		for (const auto& result : results) {
			// TODO: Filter for the persisted anchors only, ignore the room itself
			OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->print_supported_components(result.space);
			found_anchor_uuids.push_back(String(XrGodotUtils::uuidToString(result.uuid).c_str()));
		}

		emit_signal("persisted_anchors_found", found_anchor_uuids);
	});
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::query_spatial_entities_by_uuid(const String& uuid, SpaceQueryCompleteCallback_t callback) {
	std::vector<XrUuidEXT> uuids = {XrGodotUtils::uuidFromString(std::string(uuid.ascii()))};

	XrSpaceStorageLocationFilterInfoFB storageLocationFilterInfo = {
		XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB,
		nullptr,
		XR_SPACE_STORAGE_LOCATION_LOCAL_FB
	};

	// First query the anchor by UUID
	XrSpaceUuidFilterInfoFB uuidFilter = {
		XR_TYPE_SPACE_UUID_FILTER_INFO_FB,
		&storageLocationFilterInfo,
		1,
		&uuids[0],
	};

	XrSpaceQueryInfoFB queryInfo = {
			XR_TYPE_SPACE_QUERY_INFO_FB,
			nullptr,
			XR_SPACE_QUERY_ACTION_LOAD_FB,
			MAX_PERSISTENT_SPACES,
			0,
			(XrSpaceFilterInfoBaseHeaderFB*)&uuidFilter,
			nullptr};

		OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB*) &queryInfo, callback);
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::query_spatial_entities(const XrSpaceQueryInfoBaseHeaderFB *info, SpaceQueryCompleteCallback_t callback) {
	XrAsyncRequestIdFB requestId;
	const XrResult result = xrQuerySpacesFB(SESSION, info, &requestId);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrQuerySpacesFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		callback({});
		return;
	}
	query_complete_callbacks[requestId] = callback;
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::on_space_query_results(const XrEventDataSpaceQueryResultsAvailableFB *event) {
	// Query the results that are now available using two-call idiom
	XrSpaceQueryResultsFB queryResults{
		XR_TYPE_SPACE_QUERY_RESULTS_FB, // type
		nullptr, // next
		0, // resultCapacityInput
		0, // resultCapacityOutput
		nullptr, // results
	};
	XrResult result = xrRetrieveSpaceQueryResultsFB(SESSION, event->requestId, &queryResults);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrRetrieveSpaceQueryResultsFB (first call) failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return;
	}

	Vector<XrSpaceQueryResultFB> results;
	results.resize(queryResults.resultCountOutput);
	queryResults.resultCapacityInput = results.size();
	queryResults.resultCountOutput = 0;
	queryResults.results = results.ptrw();

	result = xrRetrieveSpaceQueryResultsFB(SESSION, event->requestId, &queryResults);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrRetrieveSpaceQueryResultsFB 2 failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return;
	}

	// Store the results to send via callback later
	query_results[event->requestId].append_array(results);
}

void OpenXRFbSpatialEntityQueryExtensionWrapper::on_space_query_complete(const XrEventDataSpaceQueryCompleteFB *event) {
	if (!query_complete_callbacks.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB");
		return;
	}
	query_complete_callbacks[event->requestId](query_results[event->requestId]);
	query_complete_callbacks.erase(event->requestId);
	query_results.erase(event->requestId);
}
