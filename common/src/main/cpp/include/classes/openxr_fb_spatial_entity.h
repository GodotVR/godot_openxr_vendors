/**************************************************************************/
/*  openxr_fb_spatial_entity.h                                            */
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

#ifndef OPENXR_FB_SPATIAL_ENTITY_H
#define OPENXR_FB_SPATIAL_ENTITY_H

#include <openxr/openxr.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot {
class Node3D;
class MeshInstance3D;

class OpenXRFbSpatialEntity : public RefCounted {
	GDCLASS(OpenXRFbSpatialEntity, RefCounted);

public:

	enum StorageLocation {
		STORAGE_LOCAL,
		STORAGE_CLOUD,
	};

	enum ComponentType {
		COMPONENT_TYPE_LOCATABLE,
		COMPONENT_TYPE_STORABLE,
		COMPONENT_TYPE_SHARABLE,
		COMPONENT_TYPE_BOUNDED_2D,
		COMPONENT_TYPE_BOUNDED_3D,
		COMPONENT_TYPE_SEMANTIC_LABELS,
		COMPONENT_TYPE_ROOM_LAYOUT,
		COMPONENT_TYPE_CONTAINER,
		COMPONENT_TYPE_TRIANGLE_MESH,
	};

private:
	XrSpace space = XR_NULL_HANDLE;
	StringName uuid;

protected:
	static void _bind_methods();

	static void _on_set_component_enabled_completed(XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *userdata);

	String _to_string() const;

public:
	StringName get_uuid() const;

	Array get_supported_components() const;
	bool is_component_supported(ComponentType p_component) const;
	bool is_component_enabled(ComponentType p_component) const;
	void set_component_enabled(ComponentType p_component, bool p_enabled);

	PackedStringArray get_semantic_labels() const;
	Dictionary get_room_layout() const;
	Array get_contained_uuids() const;
	Rect2 get_bounding_box_2d() const;
	AABB get_bounding_box_3d() const;
	PackedVector2Array get_boundary_2d() const;

	void track();
	void untrack();
	bool is_tracked() const;

	MeshInstance3D *create_mesh_instance() const;
	Node3D *create_collision_shape() const;

	static XrSpaceStorageLocationFB to_openxr_storage_location(StorageLocation p_location);
	static XrSpaceComponentTypeFB to_openxr_component_type(ComponentType p_component);
	static ComponentType from_openxr_component_type(XrSpaceComponentTypeFB p_component);

	XrSpace get_space();

	OpenXRFbSpatialEntity() = default;
	OpenXRFbSpatialEntity(XrSpace p_space, const XrUuidEXT &p_uuid);
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRFbSpatialEntity::StorageLocation);
VARIANT_ENUM_CAST(OpenXRFbSpatialEntity::ComponentType);

#endif
