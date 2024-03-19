/**************************************************************************/
/*  openxr_fb_spatial_entity_query.h                                      */
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

#ifndef OPENXR_FB_SPATIAL_ENTITY_QUERY_H
#define OPENXR_FB_SPATIAL_ENTITY_QUERY_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "classes/openxr_fb_spatial_entity.h"

namespace godot {
class OpenXRFbSpatialEntityQuery : public RefCounted {
	GDCLASS(OpenXRFbSpatialEntityQuery, RefCounted);

public:
	enum QueryType {
		QUERY_ALL,
		QUERY_BY_UUID,
		QUERY_BY_COMPONENT,
	};

private:
	QueryType query_type = QUERY_ALL;
	OpenXRFbSpatialEntity::StorageLocation location = OpenXRFbSpatialEntity::STORAGE_LOCAL;
	OpenXRFbSpatialEntity::ComponentType component_type = OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE;
	uint32_t max_results = 25;
	float timeout = 0.0f;
	Array uuids;

	bool executed = false;

protected:
	static void _bind_methods();

	bool _execute_query_all();
	bool _execute_query_by_uuid();
	bool _execute_query_by_component();

public:
	void set_max_results(uint32_t p_max_results);
	uint32_t get_max_results() const;

	void set_timeout(float p_timeout);
	float get_timeout() const;

	void query_all();
	void query_by_uuid(Array p_uuids, OpenXRFbSpatialEntity::StorageLocation p_location = OpenXRFbSpatialEntity::STORAGE_LOCAL);
	void query_by_component(OpenXRFbSpatialEntity::ComponentType p_component_type, OpenXRFbSpatialEntity::StorageLocation p_location = OpenXRFbSpatialEntity::STORAGE_LOCAL);

	QueryType get_query_type() const;
	OpenXRFbSpatialEntity::StorageLocation get_storage_location() const;
	Array get_uuids() const;
	OpenXRFbSpatialEntity::ComponentType get_component_type() const;

	Error execute();
	bool is_executed() const;

	static void _results_callback(const Vector<XrSpaceQueryResultFB> &p_results, void *p_userdata);
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRFbSpatialEntityQuery::QueryType);

#endif
