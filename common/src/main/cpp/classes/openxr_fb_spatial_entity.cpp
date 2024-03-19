/**************************************************************************/
/*  openxr_fb_spatial_entity.cpp                                          */
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

#include "classes/openxr_fb_spatial_entity.h"

#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/plane_mesh.hpp>

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include "extensions/openxr_fb_scene_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialEntity::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_uuid"), &OpenXRFbSpatialEntity::get_uuid);

	ClassDB::bind_method(D_METHOD("get_supported_components"), &OpenXRFbSpatialEntity::get_supported_components);
	ClassDB::bind_method(D_METHOD("is_component_supported", "component"), &OpenXRFbSpatialEntity::is_component_supported);
	ClassDB::bind_method(D_METHOD("is_component_enabled", "component"), &OpenXRFbSpatialEntity::is_component_enabled);
	ClassDB::bind_method(D_METHOD("set_component_enabled", "component"), &OpenXRFbSpatialEntity::set_component_enabled);

	ClassDB::bind_method(D_METHOD("get_semantic_labels"), &OpenXRFbSpatialEntity::get_semantic_labels);
	ClassDB::bind_method(D_METHOD("get_room_layout"), &OpenXRFbSpatialEntity::get_room_layout);
	ClassDB::bind_method(D_METHOD("get_contained_uuids"), &OpenXRFbSpatialEntity::get_contained_uuids);
	ClassDB::bind_method(D_METHOD("get_bounding_box_2d"), &OpenXRFbSpatialEntity::get_bounding_box_2d);
	ClassDB::bind_method(D_METHOD("get_bounding_box_3d"), &OpenXRFbSpatialEntity::get_bounding_box_3d);
	ClassDB::bind_method(D_METHOD("get_boundary_2d"), &OpenXRFbSpatialEntity::get_boundary_2d);

	ClassDB::bind_method(D_METHOD("track"), &OpenXRFbSpatialEntity::track);
	ClassDB::bind_method(D_METHOD("untrack"), &OpenXRFbSpatialEntity::untrack);
	ClassDB::bind_method(D_METHOD("is_tracked"), &OpenXRFbSpatialEntity::is_tracked);

	ClassDB::bind_method(D_METHOD("create_mesh_instance"), &OpenXRFbSpatialEntity::create_mesh_instance);
	ClassDB::bind_method(D_METHOD("create_collision_shape"), &OpenXRFbSpatialEntity::create_collision_shape);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "uuid", PROPERTY_HINT_NONE, ""), "", "get_uuid");

	BIND_ENUM_CONSTANT(STORAGE_LOCAL);
	BIND_ENUM_CONSTANT(STORAGE_CLOUD);

	BIND_ENUM_CONSTANT(COMPONENT_TYPE_LOCATABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_STORABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SHARABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_BOUNDED_2D);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_BOUNDED_3D);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SEMANTIC_LABELS);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_ROOM_LAYOUT);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_CONTAINER);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_TRIANGLE_MESH);

	ADD_SIGNAL(MethodInfo("set_component_enabled_completed", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "component"), PropertyInfo(Variant::Type::BOOL, "enabled")));
}

String OpenXRFbSpatialEntity::_to_string() const {
	return String("[OpenXRFbSpatialEntity ") + uuid + String("]");
}

StringName OpenXRFbSpatialEntity::get_uuid() const {
	return uuid;
}

Array OpenXRFbSpatialEntity::get_supported_components() const {
	Array ret;

	Vector<XrSpaceComponentTypeFB> components = OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->get_support_components(space);
	ret.resize(components.size());
	for (int i = 0; i < components.size(); i++) {
		ret[i] = from_openxr_component_type(components[i]);
	}

	return ret;
}

bool OpenXRFbSpatialEntity::is_component_supported(ComponentType p_component) const {
	return get_supported_components().has(p_component);
}

bool OpenXRFbSpatialEntity::is_component_enabled(ComponentType p_component) const {
	return OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(space, to_openxr_component_type(p_component));
}

void OpenXRFbSpatialEntity::set_component_enabled(ComponentType p_component, bool p_enabled) {
	Ref<OpenXRFbSpatialEntity> *userdata = memnew(Ref<OpenXRFbSpatialEntity>(this));
	XrAsyncRequestIdFB request_id = OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(space, to_openxr_component_type(p_component), p_enabled, OpenXRFbSpatialEntity::_on_set_component_enabled_completed, userdata);
}

void OpenXRFbSpatialEntity::_on_set_component_enabled_completed(XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata) {
	Ref<OpenXRFbSpatialEntity> *userdata = (Ref<OpenXRFbSpatialEntity> *)p_userdata;
	(*userdata)->emit_signal("set_component_enabled_completed", XR_SUCCEEDED(p_result), from_openxr_component_type(p_component), p_enabled);
	memdelete(userdata);
}

PackedStringArray OpenXRFbSpatialEntity::get_semantic_labels() const {
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_semantic_labels(space);
}

Dictionary OpenXRFbSpatialEntity::get_room_layout() const {
	OpenXRFbSceneExtensionWrapper::RoomLayout room_layout;
	if (!OpenXRFbSceneExtensionWrapper::get_singleton()->get_room_layout(space, room_layout)) {
		return Dictionary();
	}

	Dictionary ret;
	ret["floor"] = OpenXRUtilities::uuid_to_string_name(room_layout.floor);
	ret["ceiling"] = OpenXRUtilities::uuid_to_string_name(room_layout.ceiling);

	Array walls_array;
	walls_array.resize(room_layout.walls.size());
	for (int i = 0; i < room_layout.walls.size(); i++) {
		walls_array[i] = OpenXRUtilities::uuid_to_string_name(room_layout.walls[i]);
	}
	ret["walls"] = walls_array;

	return ret;
}

Array OpenXRFbSpatialEntity::get_contained_uuids() const {
	Vector<XrUuidEXT> uuids = OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton()->get_contained_uuids(space);

	Array ret;
	ret.resize(uuids.size());
	for (int i = 0; i < uuids.size(); i++) {
		ret[i] = OpenXRUtilities::uuid_to_string_name(uuids[i]);
	}
	return ret;
}

Rect2 OpenXRFbSpatialEntity::get_bounding_box_2d() const {
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_bounding_box_2d(space);
}

AABB OpenXRFbSpatialEntity::get_bounding_box_3d() const {
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_bounding_box_3d(space);
}

PackedVector2Array OpenXRFbSpatialEntity::get_boundary_2d() const {
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_boundary_2d(space);
}

void OpenXRFbSpatialEntity::track() {
	ERR_FAIL_COND_MSG(!is_component_enabled(COMPONENT_TYPE_LOCATABLE), vformat("Cannot track spatial entity %s because COMPONENT_TYPE_LOCATABLE isn't enabled.", uuid));
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->track_entity(uuid, space);
}

void OpenXRFbSpatialEntity::untrack() {
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->untrack_entity(uuid);
}

bool OpenXRFbSpatialEntity::is_tracked() const {
	return OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_entity_tracked(uuid);
}

MeshInstance3D *OpenXRFbSpatialEntity::create_mesh_instance() const {
	MeshInstance3D *mesh_instance = nullptr;

	if (is_component_enabled(COMPONENT_TYPE_BOUNDED_3D)) {
		Ref<BoxMesh> box_mesh;
		box_mesh.instantiate();

		AABB bounding_box = get_bounding_box_3d();
		box_mesh->set_size(bounding_box.size);

		mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_mesh(box_mesh);
		mesh_instance->set_position(bounding_box.get_center());

	} else if (is_component_enabled(COMPONENT_TYPE_BOUNDED_2D)) {
		Ref<PlaneMesh> plane_mesh;
		plane_mesh.instantiate();

		Rect2 bounding_box = get_bounding_box_2d();
		plane_mesh->set_size(bounding_box.size);

		mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_mesh(plane_mesh);

		Vector2 plane_center = bounding_box.get_center();
		mesh_instance->rotate_x(Math_PI / 2.0);
		mesh_instance->set_position(Vector3(plane_center.x, plane_center.y, 0));
	}

	return mesh_instance;
}

Node3D *OpenXRFbSpatialEntity::create_collision_shape() const {
	if (is_component_enabled(COMPONENT_TYPE_BOUNDED_3D)) {
		Ref<BoxShape3D> box_shape;
		box_shape.instantiate();

		AABB bounding_box = get_bounding_box_3d();
		box_shape->set_size(bounding_box.size);

		CollisionShape3D *collision_shape = memnew(CollisionShape3D);
		collision_shape->set_shape(box_shape);
		collision_shape->set_position(bounding_box.get_center());

		return collision_shape;
	} else if (is_component_enabled(COMPONENT_TYPE_BOUNDED_2D)) {
		Ref<BoxShape3D> box_shape;
		box_shape.instantiate();

		Rect2 bounding_box = get_bounding_box_2d();
		box_shape->set_size(Vector3(bounding_box.size.x, 0, bounding_box.size.y));

		CollisionShape3D *collision_shape = memnew(CollisionShape3D);
		collision_shape->set_shape(box_shape);

		Vector2 plane_center = bounding_box.get_center();
		collision_shape->rotate_x(Math_PI / 2.0);
		collision_shape->set_position(Vector3(plane_center.x, plane_center.y, 0));

		return collision_shape;
	}

	return nullptr;
}

XrSpaceStorageLocationFB OpenXRFbSpatialEntity::to_openxr_storage_location(StorageLocation p_location) {
	switch (p_location) {
		case OpenXRFbSpatialEntity::STORAGE_LOCAL: {
			return XR_SPACE_STORAGE_LOCATION_LOCAL_FB;
		} break;
		case OpenXRFbSpatialEntity::STORAGE_CLOUD: {
			return XR_SPACE_STORAGE_LOCATION_CLOUD_FB;
		} break;
		default: {
			return XR_SPACE_STORAGE_LOCATION_INVALID_FB;
		}
	}
}

XrSpaceComponentTypeFB OpenXRFbSpatialEntity::to_openxr_component_type(ComponentType p_component) {
	switch (p_component) {
		case COMPONENT_TYPE_LOCATABLE: {
			return XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB;
		} break;
		case COMPONENT_TYPE_STORABLE: {
			return XR_SPACE_COMPONENT_TYPE_STORABLE_FB;
		} break;
		case COMPONENT_TYPE_SHARABLE: {
			return XR_SPACE_COMPONENT_TYPE_SHARABLE_FB;
		} break;
		case COMPONENT_TYPE_BOUNDED_2D: {
			return XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB;
		} break;
		case COMPONENT_TYPE_BOUNDED_3D: {
			return XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB;
		} break;
		case COMPONENT_TYPE_SEMANTIC_LABELS: {
			return XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB;
		} break;
		case COMPONENT_TYPE_ROOM_LAYOUT: {
			return XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB;
		} break;
		case COMPONENT_TYPE_CONTAINER: {
			return XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB;
		} break;
		case COMPONENT_TYPE_TRIANGLE_MESH: {
			return XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META;
		} break;
		default: {
			ERR_FAIL_V_MSG(XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB, vformat("Unknown component type: %s", p_component));
		}
	}
}

OpenXRFbSpatialEntity::ComponentType OpenXRFbSpatialEntity::from_openxr_component_type(XrSpaceComponentTypeFB p_component) {
	switch (p_component) {
		case XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB: {
			return COMPONENT_TYPE_LOCATABLE;
		} break;
		case XR_SPACE_COMPONENT_TYPE_STORABLE_FB: {
			return COMPONENT_TYPE_STORABLE;
		} break;
		case XR_SPACE_COMPONENT_TYPE_SHARABLE_FB: {
			return COMPONENT_TYPE_SHARABLE;
		} break;
		case XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB: {
			return COMPONENT_TYPE_BOUNDED_2D;
		} break;
		case XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB: {
			return COMPONENT_TYPE_BOUNDED_3D;
		} break;
		case XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB: {
			return COMPONENT_TYPE_SEMANTIC_LABELS;
		} break;
		case XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB: {
			return COMPONENT_TYPE_ROOM_LAYOUT;
		} break;
		case XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB: {
			return COMPONENT_TYPE_CONTAINER;
		} break;
		case XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META: {
			return COMPONENT_TYPE_TRIANGLE_MESH;
		} break;
		case XR_SPACE_COMPONENT_TYPE_MAX_ENUM_FB:
		default: {
			ERR_FAIL_V_MSG(COMPONENT_TYPE_LOCATABLE, vformat("Unknown OpenXR component type: %s", p_component));
		}
	}
}

XrSpace OpenXRFbSpatialEntity::get_space() {
	return space;
}

OpenXRFbSpatialEntity::OpenXRFbSpatialEntity(XrSpace p_space, const XrUuidEXT &p_uuid) {
	space = p_space;
	uuid = OpenXRUtilities::uuid_to_string_name(p_uuid);
}
