/**************************************************************************/
/*  openxr_fb_spatial_entity_query.cpp                                    */
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

#include "classes/openxr_fb_spatial_entity_query.h"

#include <godot_cpp/templates/local_vector.hpp>

#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialEntityQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_results", "count"), &OpenXRFbSpatialEntityQuery::set_max_results);
	ClassDB::bind_method(D_METHOD("get_max_results"), &OpenXRFbSpatialEntityQuery::get_max_results);
	ClassDB::bind_method(D_METHOD("set_timeout", "seconds"), &OpenXRFbSpatialEntityQuery::set_timeout);
	ClassDB::bind_method(D_METHOD("get_timeout"), &OpenXRFbSpatialEntityQuery::get_timeout);
	ClassDB::bind_method(D_METHOD("query_all"), &OpenXRFbSpatialEntityQuery::query_all);
	ClassDB::bind_method(D_METHOD("query_by_uuid", "uuids", "location"), &OpenXRFbSpatialEntityQuery::query_by_uuid, DEFVAL(OpenXRFbSpatialEntity::STORAGE_LOCAL));
	ClassDB::bind_method(D_METHOD("query_by_component", "component", "location"), &OpenXRFbSpatialEntityQuery::query_by_component, DEFVAL(OpenXRFbSpatialEntity::STORAGE_LOCAL));
	ClassDB::bind_method(D_METHOD("get_query_type"), &OpenXRFbSpatialEntityQuery::get_query_type);
	ClassDB::bind_method(D_METHOD("get_storage_location"), &OpenXRFbSpatialEntityQuery::get_storage_location);
	ClassDB::bind_method(D_METHOD("get_uuids"), &OpenXRFbSpatialEntityQuery::get_uuids);
	ClassDB::bind_method(D_METHOD("get_component_type"), &OpenXRFbSpatialEntityQuery::get_component_type);
	ClassDB::bind_method(D_METHOD("execute"), &OpenXRFbSpatialEntityQuery::execute);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_results", PROPERTY_HINT_NONE, ""), "set_max_results", "get_max_results");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "timeout", PROPERTY_HINT_RANGE, "0.001,4096,0.001,or_greater,exp,suffix:s"), "set_timeout", "get_timeout");

	BIND_ENUM_CONSTANT(QUERY_ALL);
	BIND_ENUM_CONSTANT(QUERY_BY_UUID);
	BIND_ENUM_CONSTANT(QUERY_BY_COMPONENT);

	ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::Type::ARRAY, "results")));
}

void OpenXRFbSpatialEntityQuery::set_max_results(uint32_t p_max_results) {
	max_results = p_max_results;
}

uint32_t OpenXRFbSpatialEntityQuery::get_max_results() const {
	return max_results;
}

void OpenXRFbSpatialEntityQuery::set_timeout(float p_timeout) {
	timeout = p_timeout;
}

void OpenXRFbSpatialEntityQuery::query_all() {
	query_type = QUERY_ALL;
	// Reset data used for other query types.
	component_type = OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE;
	location = OpenXRFbSpatialEntity::STORAGE_LOCAL;
	uuids.clear();
}

void OpenXRFbSpatialEntityQuery::query_by_uuid(Array p_uuids, OpenXRFbSpatialEntity::StorageLocation p_location) {
	query_type = QUERY_BY_UUID;
	uuids = p_uuids;
	location = p_location;
	// Reset data used for other query types.
	component_type = OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE;
}

void OpenXRFbSpatialEntityQuery::query_by_component(OpenXRFbSpatialEntity::ComponentType p_component_type, OpenXRFbSpatialEntity::StorageLocation p_location) {
	query_type = QUERY_BY_COMPONENT;
	component_type = p_component_type;
	location = p_location;
	// Reset data used for other query types.
	uuids.clear();
}

float OpenXRFbSpatialEntityQuery::get_timeout() const {
	return timeout;
}

OpenXRFbSpatialEntityQuery::QueryType OpenXRFbSpatialEntityQuery::get_query_type() const {
	return query_type;
}

OpenXRFbSpatialEntity::StorageLocation OpenXRFbSpatialEntityQuery::get_storage_location() const {
	return location;
}

Array OpenXRFbSpatialEntityQuery::get_uuids() const {
	return uuids;
}

OpenXRFbSpatialEntity::ComponentType OpenXRFbSpatialEntityQuery::get_component_type() const {
	return component_type;
}

Error OpenXRFbSpatialEntityQuery::execute() {
	ERR_FAIL_COND_V(executed, ERR_ALREADY_IN_USE);
	executed = true;

	bool succeeded = false;
	switch (query_type) {
		case QUERY_ALL: {
			succeeded = _execute_query_all();
		} break;
		case QUERY_BY_UUID: {
			succeeded = _execute_query_by_uuid();
		} break;
		case QUERY_BY_COMPONENT: {
			succeeded = _execute_query_by_component();
		} break;
		default:
			return ERR_INVALID_DATA;
	}

	return succeeded ? OK : FAILED;
}

bool OpenXRFbSpatialEntityQuery::_execute_query_all() {
	XrSpaceQueryInfoFB query = {
		XR_TYPE_SPACE_QUERY_INFO_FB, // type
		nullptr, // next
		XR_SPACE_QUERY_ACTION_LOAD_FB, // queryAction
		max_results, // maxResultsCount
		(XrDuration)(timeout * 1000000), // timeout
		nullptr, // filter
		nullptr, // excludeFilter
	};

	Ref<OpenXRFbSpatialEntityQuery> *userdata = memnew(Ref<OpenXRFbSpatialEntityQuery>(this));
	return OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB *)&query, &OpenXRFbSpatialEntityQuery::_results_callback, userdata);
}

bool OpenXRFbSpatialEntityQuery::_execute_query_by_uuid() {
	XrSpaceStorageLocationFilterInfoFB location_filter = {
		XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB, // type
		nullptr, // next
		OpenXRFbSpatialEntity::to_openxr_storage_location(location), // location
	};

	LocalVector<XrUuidEXT> uuid_array;
	uuid_array.resize(uuids.size());
	for (int i = 0; i < uuids.size(); i++) {
		PackedByteArray uuid_data = String(uuids[i]).replace("-", "").hex_decode();
		ERR_CONTINUE_MSG(uuid_data.size() != 16, vformat("Invalid UUID: %s", uuids[i]));
		memcpy(uuid_array[i].data, uuid_data.ptr(), 16);
	}

	XrSpaceUuidFilterInfoFB filter = {
		XR_TYPE_SPACE_UUID_FILTER_INFO_FB, // type
		&location_filter, // next
		uuid_array.size(), // uuidCount
		uuid_array.ptr(), // uuids
	};

	XrSpaceQueryInfoFB query = {
		XR_TYPE_SPACE_QUERY_INFO_FB, // type
		nullptr, // next
		XR_SPACE_QUERY_ACTION_LOAD_FB, // queryAction
		max_results, // maxResultsCount
		(XrDuration)(timeout * 1000000), // timeout
		(XrSpaceFilterInfoBaseHeaderFB *)&filter, // filter
		nullptr, // excludeFilter
	};

	Ref<OpenXRFbSpatialEntityQuery> *userdata = memnew(Ref<OpenXRFbSpatialEntityQuery>(this));
	return OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB *)&query, &OpenXRFbSpatialEntityQuery::_results_callback, userdata);
}

bool OpenXRFbSpatialEntityQuery::_execute_query_by_component() {
	XrSpaceStorageLocationFilterInfoFB location_filter = {
		XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB, // type
		nullptr, // next
		OpenXRFbSpatialEntity::to_openxr_storage_location(location), // location
	};

	XrSpaceComponentFilterInfoFB filter = {
		XR_TYPE_SPACE_COMPONENT_FILTER_INFO_FB, // type
		&location_filter, // next
		OpenXRFbSpatialEntity::to_openxr_component_type(component_type), // componentType
	};

	XrSpaceQueryInfoFB query = {
		XR_TYPE_SPACE_QUERY_INFO_FB, // type
		nullptr, // next
		XR_SPACE_QUERY_ACTION_LOAD_FB, // queryAction
		max_results, // maxResultsCount
		(XrDuration)(timeout * 1000000), // timeout
		(XrSpaceFilterInfoBaseHeaderFB *)&filter, // filter
		nullptr, // excludeFilter
	};

	Ref<OpenXRFbSpatialEntityQuery> *userdata = memnew(Ref<OpenXRFbSpatialEntityQuery>(this));
	return OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB *)&query, &OpenXRFbSpatialEntityQuery::_results_callback, userdata);
}

void OpenXRFbSpatialEntityQuery::_results_callback(const Vector<XrSpaceQueryResultFB> &p_results, void *p_userdata) {
	Ref<OpenXRFbSpatialEntityQuery> *userdata = (Ref<OpenXRFbSpatialEntityQuery> *)p_userdata;

	Array results;
	results.resize(p_results.size());
	for (int i = 0; i < p_results.size(); i++) {
		Ref<OpenXRFbSpatialEntity> entity = Ref<OpenXRFbSpatialEntity>(memnew(OpenXRFbSpatialEntity(p_results[i].space, p_results[i].uuid)));
		results[i] = entity;
	}

	(*userdata)->emit_signal("completed", results);

	memdelete(userdata);
}
