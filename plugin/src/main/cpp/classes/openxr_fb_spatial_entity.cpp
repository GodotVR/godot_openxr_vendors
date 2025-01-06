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

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/plane_mesh.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/templates/local_vector.hpp>

#include "extensions/openxr_fb_scene_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_sharing_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"
#include "extensions/openxr_meta_spatial_entity_mesh_extension_wrapper.h"

#include "classes/openxr_fb_spatial_entity_user.h"

using namespace godot;

void OpenXRFbSpatialEntity::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_uuid"), &OpenXRFbSpatialEntity::get_uuid);
	ClassDB::bind_method(D_METHOD("set_custom_data"), &OpenXRFbSpatialEntity::set_custom_data);
	ClassDB::bind_method(D_METHOD("get_custom_data"), &OpenXRFbSpatialEntity::get_custom_data);

	ClassDB::bind_method(D_METHOD("get_supported_components"), &OpenXRFbSpatialEntity::get_supported_components);
	ClassDB::bind_method(D_METHOD("is_component_supported", "component"), &OpenXRFbSpatialEntity::is_component_supported);
	ClassDB::bind_method(D_METHOD("is_component_enabled", "component"), &OpenXRFbSpatialEntity::is_component_enabled);
	ClassDB::bind_method(D_METHOD("set_component_enabled", "component", "enabled"), &OpenXRFbSpatialEntity::set_component_enabled);

	ClassDB::bind_method(D_METHOD("get_semantic_labels"), &OpenXRFbSpatialEntity::get_semantic_labels);
	ClassDB::bind_method(D_METHOD("get_room_layout"), &OpenXRFbSpatialEntity::get_room_layout);
	ClassDB::bind_method(D_METHOD("get_contained_uuids"), &OpenXRFbSpatialEntity::get_contained_uuids);
	ClassDB::bind_method(D_METHOD("get_bounding_box_2d"), &OpenXRFbSpatialEntity::get_bounding_box_2d);
	ClassDB::bind_method(D_METHOD("get_bounding_box_3d"), &OpenXRFbSpatialEntity::get_bounding_box_3d);
	ClassDB::bind_method(D_METHOD("get_boundary_2d"), &OpenXRFbSpatialEntity::get_boundary_2d);
	ClassDB::bind_method(D_METHOD("get_triangle_mesh"), &OpenXRFbSpatialEntity::get_triangle_mesh);

	ClassDB::bind_method(D_METHOD("track"), &OpenXRFbSpatialEntity::track);
	ClassDB::bind_method(D_METHOD("untrack"), &OpenXRFbSpatialEntity::untrack);
	ClassDB::bind_method(D_METHOD("is_tracked"), &OpenXRFbSpatialEntity::is_tracked);

	ClassDB::bind_method(D_METHOD("create_mesh_instance"), &OpenXRFbSpatialEntity::create_mesh_instance);
	ClassDB::bind_method(D_METHOD("create_collision_shape"), &OpenXRFbSpatialEntity::create_collision_shape);

	ClassDB::bind_static_method("OpenXRFbSpatialEntity", D_METHOD("create_spatial_anchor", "transform"), &OpenXRFbSpatialEntity::create_spatial_anchor);

	ClassDB::bind_method(D_METHOD("save_to_storage", "location"), &OpenXRFbSpatialEntity::save_to_storage, DEFVAL(STORAGE_LOCAL));
	ClassDB::bind_method(D_METHOD("erase_from_storage", "location"), &OpenXRFbSpatialEntity::erase_from_storage, DEFVAL(STORAGE_LOCAL));
	ClassDB::bind_method(D_METHOD("share_with_users", "users"), &OpenXRFbSpatialEntity::share_with_users);
	ClassDB::bind_method(D_METHOD("destroy"), &OpenXRFbSpatialEntity::destroy);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "uuid", PROPERTY_HINT_NONE, ""), "", "get_uuid");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "custom_data", PROPERTY_HINT_NONE, ""), "set_custom_data", "get_custom_data");

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

	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_set_component_enabled_completed", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "component"), PropertyInfo(Variant::Type::BOOL, "enabled")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_created", PropertyInfo(Variant::Type::BOOL, "succeeded")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_saved", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "location")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_erased", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "location")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_shared", PropertyInfo(Variant::Type::BOOL, "succeeded")));
}

String OpenXRFbSpatialEntity::_to_string() const {
	return String("[OpenXRFbSpatialEntity ") + uuid + String("]");
}

StringName OpenXRFbSpatialEntity::get_uuid() const {
	return uuid;
}

void OpenXRFbSpatialEntity::set_custom_data(const Dictionary &p_custom_data) {
	custom_data = p_custom_data;
}

Dictionary OpenXRFbSpatialEntity::get_custom_data() const {
	return custom_data;
}

Array OpenXRFbSpatialEntity::get_supported_components() const {
	Array ret;

	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, ret, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	Vector<XrSpaceComponentTypeFB> components = OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->get_support_components(space);
	ret.resize(components.size());
	for (int i = 0; i < components.size(); i++) {
		ret[i] = from_openxr_component_type(components[i]);
	}

	return ret;
}

bool OpenXRFbSpatialEntity::is_component_supported(ComponentType p_component) const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, false, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	ERR_FAIL_COND_V(p_component == COMPONENT_TYPE_UNKNOWN, false);
	return get_supported_components().has(p_component);
}

bool OpenXRFbSpatialEntity::is_component_enabled(ComponentType p_component) const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, false, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	ERR_FAIL_COND_V(p_component == COMPONENT_TYPE_UNKNOWN, false);
	return OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(space, to_openxr_component_type(p_component));
}

void OpenXRFbSpatialEntity::set_component_enabled(ComponentType p_component, bool p_enabled) {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	ERR_FAIL_COND(p_component == COMPONENT_TYPE_UNKNOWN);
	Ref<OpenXRFbSpatialEntity> *userdata = memnew(Ref<OpenXRFbSpatialEntity>(this));
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(space, to_openxr_component_type(p_component), p_enabled, OpenXRFbSpatialEntity::_on_set_component_enabled_completed, userdata);
}

void OpenXRFbSpatialEntity::_on_set_component_enabled_completed(XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata) {
	Ref<OpenXRFbSpatialEntity> *userdata = (Ref<OpenXRFbSpatialEntity> *)p_userdata;
	(*userdata)->emit_signal("openxr_fb_spatial_entity_set_component_enabled_completed", XR_SUCCEEDED(p_result), from_openxr_component_type(p_component), p_enabled);
	memdelete(userdata);
}

PackedStringArray OpenXRFbSpatialEntity::get_semantic_labels() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, PackedStringArray(), "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_semantic_labels(space);
}

Dictionary OpenXRFbSpatialEntity::get_room_layout() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, Dictionary(), "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

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
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, Array(), "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	Vector<XrUuidEXT> uuids = OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton()->get_contained_uuids(space);

	Array ret;
	ret.resize(uuids.size());
	for (int i = 0; i < uuids.size(); i++) {
		ret[i] = OpenXRUtilities::uuid_to_string_name(uuids[i]);
	}
	return ret;
}

Rect2 OpenXRFbSpatialEntity::get_bounding_box_2d() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, Rect2(), "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_bounding_box_2d(space);
}

AABB OpenXRFbSpatialEntity::get_bounding_box_3d() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, AABB(), "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_bounding_box_3d(space);
}

PackedVector2Array OpenXRFbSpatialEntity::get_boundary_2d() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, PackedVector2Array(), "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_boundary_2d(space);
}

void OpenXRFbSpatialEntity::track() {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	ERR_FAIL_COND_MSG(!is_component_enabled(COMPONENT_TYPE_LOCATABLE), vformat("Cannot track spatial entity %s because COMPONENT_TYPE_LOCATABLE isn't enabled.", uuid));
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->track_entity(uuid, space);
}

void OpenXRFbSpatialEntity::untrack() {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->untrack_entity(uuid);
}

bool OpenXRFbSpatialEntity::is_tracked() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, false, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	return OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_entity_tracked(uuid);
}

Array OpenXRFbSpatialEntity::get_triangle_mesh() const {
	OpenXRMetaSpatialEntityMeshExtensionWrapper::TriangleMesh mesh_data;
	if (!OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton()->get_triangle_mesh(space, mesh_data)) {
		return Array();
	}

	PackedVector3Array vertices;
	vertices.resize(mesh_data.vertices.size());
	for (int i = 0; i < mesh_data.vertices.size(); i++) {
		XrVector3f vertex = mesh_data.vertices[i];
		vertices[i] = Vector3(vertex.x, vertex.y, vertex.z);
	}

	PackedInt32Array indices;
	indices.resize(mesh_data.indices.size());
	for (int i = 0; i < mesh_data.indices.size(); i += 3) {
		// Reverse the winding order.
		indices[i] = mesh_data.indices[i + 2];
		indices[i + 1] = mesh_data.indices[i + 1];
		indices[i + 2] = mesh_data.indices[i];
	}

	Array mesh_array;
	mesh_array.resize(Mesh::ARRAY_MAX);
	mesh_array[Mesh::ARRAY_VERTEX] = vertices;
	mesh_array[Mesh::ARRAY_INDEX] = indices;
	return mesh_array;
}

MeshInstance3D *OpenXRFbSpatialEntity::create_mesh_instance() const {
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, nullptr, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	MeshInstance3D *mesh_instance = nullptr;

	if (is_component_enabled(COMPONENT_TYPE_TRIANGLE_MESH)) {
		Ref<SurfaceTool> surface_tool;
		surface_tool.instantiate();

		surface_tool->create_from_arrays(get_triangle_mesh(), Mesh::PRIMITIVE_TRIANGLES);
		surface_tool->generate_normals();
		surface_tool->generate_tangents();

		Ref<ArrayMesh> array_mesh;
		array_mesh.instantiate();
		array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface_tool->commit_to_arrays());

		mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_mesh(array_mesh);

	} else if (is_component_enabled(COMPONENT_TYPE_BOUNDED_3D)) {
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
	ERR_FAIL_COND_V_MSG(space == XR_NULL_HANDLE, nullptr, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	if (is_component_enabled(COMPONENT_TYPE_TRIANGLE_MESH)) {
		OpenXRMetaSpatialEntityMeshExtensionWrapper::TriangleMesh mesh_data;
		if (!OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton()->get_triangle_mesh(space, mesh_data)) {
			return nullptr;
		}

		PackedVector3Array faces;
		faces.resize(mesh_data.indices.size());
		for (int i = 0; i < mesh_data.indices.size(); i += 3) {
			XrVector3f vertex[3] = {
				mesh_data.vertices[mesh_data.indices[i]],
				mesh_data.vertices[mesh_data.indices[i + 1]],
				mesh_data.vertices[mesh_data.indices[i + 2]],
			};
			// Reverse the winding order.
			faces[i] = Vector3(vertex[2].x, vertex[2].y, vertex[2].z);
			faces[i + 1] = Vector3(vertex[1].x, vertex[1].y, vertex[1].z);
			faces[i + 2] = Vector3(vertex[0].x, vertex[0].y, vertex[0].z);
		}

		Ref<ConcavePolygonShape3D> polygon_shape;
		polygon_shape.instantiate();
		polygon_shape->set_faces(faces);

		CollisionShape3D *collision_shape = memnew(CollisionShape3D);
		collision_shape->set_shape(polygon_shape);

		return collision_shape;
	} else if (is_component_enabled(COMPONENT_TYPE_BOUNDED_3D)) {
		Ref<BoxShape3D> box_shape;
		box_shape.instantiate();

		AABB bounding_box = get_bounding_box_3d();
		box_shape->set_size(bounding_box.size);

		CollisionShape3D *collision_shape = memnew(CollisionShape3D);
		collision_shape->set_shape(box_shape);
		collision_shape->set_position(bounding_box.get_center());

		return collision_shape;
	} else if (is_component_enabled(COMPONENT_TYPE_BOUNDED_2D)) {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();
		ERR_FAIL_NULL_V(project_settings, nullptr);

		float collision_shape_2d_thickness = project_settings->get_setting_with_override("xr/openxr/extensions/meta_scene_api/collision_shape_2d_thickness");

		Ref<BoxShape3D> box_shape;
		box_shape.instantiate();

		Rect2 bounding_box = get_bounding_box_2d();
		box_shape->set_size(Vector3(bounding_box.size.x, collision_shape_2d_thickness, bounding_box.size.y));

		CollisionShape3D *collision_shape = memnew(CollisionShape3D);
		collision_shape->set_shape(box_shape);

		Vector2 plane_center = bounding_box.get_center();
		collision_shape->rotate_x(Math_PI / 2.0);
		collision_shape->set_position(Vector3(plane_center.x, plane_center.y, -(collision_shape_2d_thickness / 2.0)));

		return collision_shape;
	}

	return nullptr;
}

Ref<OpenXRFbSpatialEntity> OpenXRFbSpatialEntity::create_spatial_anchor(const Transform3D &p_transform) {
	Ref<OpenXRFbSpatialEntity> *userdata = memnew(Ref<OpenXRFbSpatialEntity>());
	(*userdata).instantiate();
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->create_spatial_anchor(p_transform, &OpenXRFbSpatialEntity::_on_spatial_anchor_created, userdata)) {
		// If it fails to create, then _on_spatial_anchor_created() would have been called immediately,
		// which will have already done memdelete(userdata), so we need to return a new null value.
		return Ref<OpenXRFbSpatialEntity>();
	}
	return *userdata;
}

void OpenXRFbSpatialEntity::_on_spatial_anchor_created(XrResult p_result, XrSpace p_space, const XrUuidEXT *p_uuid, void *p_userdata) {
	Ref<OpenXRFbSpatialEntity> *userdata = (Ref<OpenXRFbSpatialEntity> *)p_userdata;
	bool success = XR_SUCCEEDED(p_result);
	if (success) {
		(*userdata)->space = p_space;
		(*userdata)->uuid = OpenXRUtilities::uuid_to_string_name(*p_uuid);
	}
	(*userdata)->emit_signal("openxr_fb_spatial_entity_created", success);
	memdelete(userdata);
}

void OpenXRFbSpatialEntity::save_to_storage(StorageLocation p_location) {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	XrSpaceSaveInfoFB save_info = {
		XR_TYPE_SPACE_SAVE_INFO_FB, // type
		nullptr, // next
		space, // space
		to_openxr_storage_location(p_location), // location
		XR_SPACE_PERSISTENCE_MODE_INDEFINITE_FB, // persistenceMode
	};

	Ref<OpenXRFbSpatialEntity> *userdata = memnew(Ref<OpenXRFbSpatialEntity>(this));
	OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton()->save_space(&save_info, OpenXRFbSpatialEntity::_on_save_to_storage, userdata);
}

void OpenXRFbSpatialEntity::_on_save_to_storage(XrResult p_result, XrSpaceStorageLocationFB p_location, void *p_userdata) {
	Ref<OpenXRFbSpatialEntity> *userdata = (Ref<OpenXRFbSpatialEntity> *)p_userdata;
	(*userdata)->emit_signal("openxr_fb_spatial_entity_saved", XR_SUCCEEDED(p_result), from_openxr_storage_location(p_location));
	memdelete(userdata);
}

void OpenXRFbSpatialEntity::erase_from_storage(StorageLocation p_location) {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	XrSpaceEraseInfoFB erase_info = {
		XR_TYPE_SPACE_ERASE_INFO_FB, // type
		nullptr, // next
		space, // space
		to_openxr_storage_location(p_location), // location
	};

	Ref<OpenXRFbSpatialEntity> *userdata = memnew(Ref<OpenXRFbSpatialEntity>(this));
	OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton()->erase_space(&erase_info, OpenXRFbSpatialEntity::_on_save_to_storage, userdata);
}

void OpenXRFbSpatialEntity::_on_erase_from_storage(XrResult p_result, XrSpaceStorageLocationFB p_location, void *p_userdata) {
	Ref<OpenXRFbSpatialEntity> *userdata = (Ref<OpenXRFbSpatialEntity> *)p_userdata;
	(*userdata)->emit_signal("openxr_fb_spatial_entity_erased", XR_SUCCEEDED(p_result), from_openxr_storage_location(p_location));
	memdelete(userdata);
}

void OpenXRFbSpatialEntity::share_with_users(const TypedArray<OpenXRFbSpatialEntityUser> &p_users) {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");

	LocalVector<XrSpaceUserFB> users;
	users.resize(p_users.size());
	for (int i = 0; i < p_users.size(); i++) {
		Ref<OpenXRFbSpatialEntityUser> user = p_users[i];
		users[i] = user->get_user_handle();
	}

	XrSpace spaces[1] = { space };

	XrSpaceShareInfoFB info = {
		XR_TYPE_SPACE_SHARE_INFO_FB, // type
		nullptr, // next
		1, // spaceCount
		spaces, // spaces
		(uint32_t)users.size(), // userCount
		users.ptr(), // users
	};

	Ref<OpenXRFbSpatialEntity> *userdata = memnew(Ref<OpenXRFbSpatialEntity>(this));
	OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton()->share_spaces(&info, OpenXRFbSpatialEntity::_on_share_with_users, userdata);
}

void OpenXRFbSpatialEntity::_on_share_with_users(XrResult p_result, void *p_userdata) {
	Ref<OpenXRFbSpatialEntity> *userdata = (Ref<OpenXRFbSpatialEntity> *)p_userdata;
	(*userdata)->emit_signal("openxr_fb_spatial_entity_shared", XR_SUCCEEDED(p_result));
	memdelete(userdata);
}

void OpenXRFbSpatialEntity::destroy() {
	ERR_FAIL_COND_MSG(space == XR_NULL_HANDLE, "Underlying spatial entity doesn't exist (yet) or has been destroyed.");
	OpenXRFbSpatialEntityExtensionWrapper *spatial_entity_extension_wrapper = OpenXRFbSpatialEntityExtensionWrapper::get_singleton();
	if (spatial_entity_extension_wrapper) {
		spatial_entity_extension_wrapper->untrack_entity(uuid);
		spatial_entity_extension_wrapper->destroy_space(space);
		space = XR_NULL_HANDLE;
	}
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

OpenXRFbSpatialEntity::StorageLocation OpenXRFbSpatialEntity::from_openxr_storage_location(XrSpaceStorageLocationFB p_location) {
	switch (p_location) {
		case XR_SPACE_STORAGE_LOCATION_LOCAL_FB: {
			return STORAGE_LOCAL;
		} break;
		case XR_SPACE_STORAGE_LOCATION_CLOUD_FB: {
			return STORAGE_CLOUD;
		} break;
		case XR_SPACE_STORAGE_LOCATION_INVALID_FB:
		case XR_SPACE_STORAGE_LOCATION_MAX_ENUM_FB:
		default: {
			WARN_PRINT_ONCE(vformat("Received invalid XrSpaceStorageLocationFB: %s.", (int)p_location));
			return STORAGE_LOCAL;
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
		case COMPONENT_TYPE_UNKNOWN:
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
			ERR_FAIL_V_MSG(COMPONENT_TYPE_UNKNOWN, vformat("Unknown OpenXR component type: %s", p_component));
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
