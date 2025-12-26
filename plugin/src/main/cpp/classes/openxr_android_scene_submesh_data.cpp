/**************************************************************************/
/*  openxr_android_scene_submesh_data.cpp                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "classes/openxr_android_scene_submesh_data.h"
#include "extensions/openxr_android_scene_meshing_extension_wrapper.h"
#include "util.h"
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {
constexpr bool IS_INDEXED_ARRAY = true;
constexpr bool COPY_NORMALS = true;
constexpr bool COPY_SEMANTICS = true;

_FORCE_INLINE_ void xr_to_godot(Vector3 &r_godot_vector3, const XrVector3f &p_xr_vector3) {
	r_godot_vector3.x = p_xr_vector3.x;
	r_godot_vector3.y = p_xr_vector3.y;
	r_godot_vector3.z = p_xr_vector3.z;
}

OpenXRAndroidSceneSubmeshData::SemanticLabel xr_semantic_label_to_godot(uint8_t p_xr_semantic_label) {
	switch ((XrSceneMeshSemanticLabelANDROID)p_xr_semantic_label) {
		case XR_SCENE_MESH_SEMANTIC_LABEL_OTHER_ANDROID:
			return OpenXRAndroidSceneSubmeshData::SEMANTIC_LABEL_OTHER;
		case XR_SCENE_MESH_SEMANTIC_LABEL_FLOOR_ANDROID:
			return OpenXRAndroidSceneSubmeshData::SEMANTIC_LABEL_FLOOR;
		case XR_SCENE_MESH_SEMANTIC_LABEL_CEILING_ANDROID:
			return OpenXRAndroidSceneSubmeshData::SEMANTIC_LABEL_CEILING;
		case XR_SCENE_MESH_SEMANTIC_LABEL_WALL_ANDROID:
			return OpenXRAndroidSceneSubmeshData::SEMANTIC_LABEL_WALL;
		case XR_SCENE_MESH_SEMANTIC_LABEL_TABLE_ANDROID:
			return OpenXRAndroidSceneSubmeshData::SEMANTIC_LABEL_TABLE;
		default:
			WARN_PRINT(vformat("OpenXR: got unexpected mesh semantic label %d", p_xr_semantic_label));
			return OpenXRAndroidSceneSubmeshData::SEMANTIC_LABEL_OTHER;
	}
}
} //namespace

OpenXRAndroidSceneSubmeshData::OpenXRAndroidSceneSubmeshData() {}
OpenXRAndroidSceneSubmeshData::~OpenXRAndroidSceneSubmeshData() {}

void OpenXRAndroidSceneSubmeshData::set_is_deleted(uint64_t p_update_idx) {
	update_state = UPDATE_STATE_DELETED;
	update_idx = p_update_idx;
}

uint64_t OpenXRAndroidSceneSubmeshData::get_update_idx() const {
	return update_idx;
}

void OpenXRAndroidSceneSubmeshData::update(UpdateState p_update_state, uint64_t p_update_idx, const StringName &p_submesh_uuid, const XrSceneSubmeshStateANDROID &p_submesh_state, XrSceneMeshSnapshotANDROID p_snapshot, bool p_enable_normals, OpenXRAndroidSceneMeshing::SemanticLabelSet p_semantic_label_set) {
	if (!_update(p_update_state, p_update_idx, p_submesh_uuid, p_submesh_state, p_snapshot, p_enable_normals, p_semantic_label_set)) {
		WARN_PRINT("Update submesh data failed; assuming UpdateState is unchanged");
		update_state = UPDATE_STATE_UNCHANGED;
	}
}

Array OpenXRAndroidSceneSubmeshData::get_arrays() const {
	return _get_arrays<!IS_INDEXED_ARRAY>();
}

Array OpenXRAndroidSceneSubmeshData::get_indexed_arrays() const {
	return _get_arrays<IS_INDEXED_ARRAY>();
}

Array OpenXRAndroidSceneSubmeshData::get_vertex_semantics() const {
	Array ret;
	if (vertex_semantics_buffer.is_empty()) {
		return ret;
	}

	ret.resize(indices_buffer.size());
	for (int i = 0; i < indices_buffer.size(); i += 3) {
		ret[i] = xr_semantic_label_to_godot(vertex_semantics_buffer[indices_buffer[i]]);
		ret[i + 1] = xr_semantic_label_to_godot(vertex_semantics_buffer[indices_buffer[i + 2]]);
		ret[i + 2] = xr_semantic_label_to_godot(vertex_semantics_buffer[indices_buffer[i + 1]]);
	}

	return ret;
}

Array OpenXRAndroidSceneSubmeshData::get_indexed_vertex_semantics() const {
	Array ret;
	if (vertex_semantics_buffer.is_empty()) {
		return ret;
	}

	ret.resize(vertex_semantics_buffer.size());
	for (int i = 0; i < vertex_semantics_buffer.size(); ++i) {
		ret[i] = xr_semantic_label_to_godot(vertex_semantics_buffer[i]);
	}

	return ret;
}

StringName OpenXRAndroidSceneSubmeshData::get_submesh_id() const {
	return submesh_uuid;
}

int64_t OpenXRAndroidSceneSubmeshData::get_last_update_time() const {
	return last_update_time;
}

const Transform3D &OpenXRAndroidSceneSubmeshData::get_transform() const {
	return submesh_pose_in_base_space;
}

const Vector3 &OpenXRAndroidSceneSubmeshData::get_bounds() const {
	return bounds;
}

OpenXRAndroidSceneSubmeshData::UpdateState OpenXRAndroidSceneSubmeshData::get_update_state() const {
	return update_state;
}

void OpenXRAndroidSceneSubmeshData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_arrays"), &OpenXRAndroidSceneSubmeshData::get_arrays);
	ClassDB::bind_method(D_METHOD("get_indexed_arrays"), &OpenXRAndroidSceneSubmeshData::get_indexed_arrays);
	ClassDB::bind_method(D_METHOD("get_vertex_semantics"), &OpenXRAndroidSceneSubmeshData::get_vertex_semantics);
	ClassDB::bind_method(D_METHOD("get_indexed_vertex_semantics"), &OpenXRAndroidSceneSubmeshData::get_indexed_vertex_semantics);
	ClassDB::bind_method(D_METHOD("get_submesh_id"), &OpenXRAndroidSceneSubmeshData::get_submesh_id);
	ClassDB::bind_method(D_METHOD("get_last_update_time"), &OpenXRAndroidSceneSubmeshData::get_last_update_time);
	ClassDB::bind_method(D_METHOD("get_transform"), &OpenXRAndroidSceneSubmeshData::get_transform);
	ClassDB::bind_method(D_METHOD("get_bounds"), &OpenXRAndroidSceneSubmeshData::get_bounds);
	ClassDB::bind_method(D_METHOD("get_update_state"), &OpenXRAndroidSceneSubmeshData::get_update_state);
	BIND_ENUM_CONSTANT(UPDATE_STATE_CREATED);
	BIND_ENUM_CONSTANT(UPDATE_STATE_UNCHANGED);
	BIND_ENUM_CONSTANT(UPDATE_STATE_UPDATED);
	BIND_ENUM_CONSTANT(UPDATE_STATE_DELETED);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_OTHER);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_FLOOR);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_CEILING);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_WALL);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_TABLE);
}

template <bool IsIndexedArray, bool CopyNormals, bool CopySemantics>
Array OpenXRAndroidSceneSubmeshData::_create_arrays_from_buffers() const {
	PackedInt32Array indices;
	PackedVector3Array vertices;
	PackedVector3Array normals;
	PackedByteArray custom0;
	int32_t *indices_dst = nullptr;
	Vector3 *vertices_dst = nullptr;
	Vector3 *normals_dst = nullptr;
	int32_t *custom0_dst = nullptr;

	// Ensure enough space, and get raw pointers to each buffer to avoid bounds checking.
	if constexpr (IsIndexedArray) {
		indices.resize(indices_buffer.size());
		vertices.resize(vertices_buffer.size());
		indices_dst = indices.ptrw();
		if constexpr (CopyNormals) {
			normals.resize(normals_buffer.size());
			normals_dst = normals.ptrw();

			// Sanity; ensure OpenXRAndroidSceneSubmeshData::_update() set these buffers up correctly
			ERR_FAIL_COND_V_MSG(vertices.size() != normals.size(), Array(), "Expected equal number of vertices and normals for indexed array.");
		}

		if constexpr (CopySemantics) {
			custom0.resize(vertex_semantics_buffer.size() * 4);
			custom0_dst = (int32_t *)custom0.ptrw();

			// Sanity; ensure OpenXRAndroidSceneSubmeshData::_update() set these buffers up correctly
			ERR_FAIL_COND_V_MSG(vertices.size() != vertex_semantics_buffer.size(), Array(), "Expected equal number of vertices and vertex semantics for indexed array.");
		}
	} else {
		vertices.resize(indices_buffer.size());

		if constexpr (CopyNormals) {
			normals.resize(indices_buffer.size());
			normals_dst = normals.ptrw();
		}

		if constexpr (CopySemantics) {
			custom0.resize(indices_buffer.size() * 4);
			custom0_dst = (int32_t *)custom0.ptrw();
		}
	}
	vertices_dst = vertices.ptrw();

	// Flip the winding of the indices from counterclockwise to clockwise before copying
	const uint32_t *indices_src = indices_buffer.ptr();
	const XrVector3f *vertices_src = vertices_buffer.ptr();
	const XrVector3f *normals_src = normals_buffer.ptr();
	const uint8_t *vertex_semantics_src = vertex_semantics_buffer.ptr();
	for (int i = 0; i < indices_buffer.size(); i += 3) {
		int32_t index0 = indices_src[i];
		int32_t index1 = indices_src[i + 2];
		int32_t index2 = indices_src[i + 1];
		if constexpr (IsIndexedArray) {
			indices_dst[i] = index0;
			indices_dst[i + 1] = index1;
			indices_dst[i + 2] = index2;
		} else {
			xr_to_godot(vertices_dst[i], vertices_src[index0]);
			xr_to_godot(vertices_dst[i + 1], vertices_src[index1]);
			xr_to_godot(vertices_dst[i + 2], vertices_src[index2]);

			if constexpr (CopyNormals) {
				xr_to_godot(normals_dst[i], normals_src[index0]);
				xr_to_godot(normals_dst[i + 1], normals_src[index1]);
				xr_to_godot(normals_dst[i + 2], normals_src[index2]);
			}

			if constexpr (CopySemantics) {
				custom0_dst[i] = vertex_semantics_src[index0];
				custom0_dst[i + 1] = vertex_semantics_src[index1];
				custom0_dst[i + 2] = vertex_semantics_src[index2];
			}
		}
	}

	if constexpr (IsIndexedArray) {
		for (int i = 0; i < vertices.size(); ++i) {
			xr_to_godot(vertices_dst[i], vertices_src[i]);
			if constexpr (CopyNormals) {
				xr_to_godot(normals_dst[i], normals_src[i]);
			}

			if constexpr (CopySemantics) {
				custom0_dst[i] = vertex_semantics_src[i];
			}
		}
	}

	Array ret;
	ret.resize(Mesh::ARRAY_MAX);

	if constexpr (IsIndexedArray) {
		ret[Mesh::ARRAY_INDEX] = indices;
	}

	ret[Mesh::ARRAY_VERTEX] = vertices;

	if constexpr (CopyNormals) {
		ret[Mesh::ARRAY_NORMAL] = normals;
	}

	if constexpr (CopySemantics) {
		ret[Mesh::ARRAY_CUSTOM0] = custom0;
	}

	return ret;
}

template <bool IsIndexedArray>
Array OpenXRAndroidSceneSubmeshData::_get_arrays() const {
	if (vertices_buffer.is_empty() || indices_buffer.is_empty()) {
		UtilityFunctions::printerr("OpenXR: received zero vertices or indices for submesh data");
		return Array();
	}

	if (normals_buffer.is_empty()) {
		return vertex_semantics_buffer.is_empty() ? _create_arrays_from_buffers<IsIndexedArray, !COPY_NORMALS, !COPY_SEMANTICS>() : _create_arrays_from_buffers<IsIndexedArray, !COPY_NORMALS, COPY_SEMANTICS>();
	}

	return vertex_semantics_buffer.is_empty() ? _create_arrays_from_buffers<IsIndexedArray, COPY_NORMALS, !COPY_SEMANTICS>() : _create_arrays_from_buffers<IsIndexedArray, COPY_NORMALS, COPY_SEMANTICS>();
}

bool OpenXRAndroidSceneSubmeshData::_update(UpdateState p_update_state, uint64_t p_update_idx, const StringName &p_submesh_uuid, const XrSceneSubmeshStateANDROID &p_submesh_state, XrSceneMeshSnapshotANDROID p_snapshot, bool p_enable_normals, OpenXRAndroidSceneMeshing::SemanticLabelSet p_semantic_label_set) {
	update_state = p_update_state;
	update_idx = p_update_idx;

	submesh_uuid = p_submesh_uuid;
	last_update_time = p_submesh_state.lastUpdatedTime;

	submesh_pose_in_base_space.origin.x = p_submesh_state.submeshPoseInBaseSpace.position.x;
	submesh_pose_in_base_space.origin.y = p_submesh_state.submeshPoseInBaseSpace.position.y;
	submesh_pose_in_base_space.origin.z = p_submesh_state.submeshPoseInBaseSpace.position.z;
	submesh_pose_in_base_space.basis = Basis{ Quaternion{ p_submesh_state.submeshPoseInBaseSpace.orientation.x, p_submesh_state.submeshPoseInBaseSpace.orientation.y, p_submesh_state.submeshPoseInBaseSpace.orientation.z, p_submesh_state.submeshPoseInBaseSpace.orientation.w } };

	bounds = Vector3{ p_submesh_state.bounds.width, p_submesh_state.bounds.height, p_submesh_state.bounds.depth };

	if (p_update_state == UPDATE_STATE_UNCHANGED) {
		return true;
	}

	OpenXRAndroidSceneMeshingExtensionWrapper *wrapper = OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton();
	ERR_FAIL_NULL_V_MSG(wrapper, false, "Cannot get submesh data without an OpenXRAndroidSceneMeshingExtensionWrapper");

	// retrieve the counts for vertices and indices
	XrSceneSubmeshDataANDROID xrsubmesh_data{
		XR_TYPE_SCENE_SUBMESH_DATA_ANDROID, // type
		nullptr, // next
		p_submesh_state.submeshId, // submeshId
		0, // vertexCapacityInput
		0, // vertexCountOutput
		nullptr, // vertexPositions
		nullptr, // vertexNormals
		nullptr, // vertexSemantics
		0, // indexCapacityInput
		0, // indexCountOutput
		nullptr, // indices
	};

	XrResult result = wrapper->xrGetSubmeshDataANDROID(p_snapshot, 1, &xrsubmesh_data);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get submesh data; ", wrapper->get_openxr_api()->get_error_string(result));
		return false;
	}

	// Sanity check what we received from the xr runtime.
	ERR_FAIL_COND_V_MSG(xrsubmesh_data.vertexCountOutput == 0, false, "OpenXR: received zero vertices for submesh data");
	ERR_FAIL_COND_V_MSG(xrsubmesh_data.indexCountOutput == 0, false, "OpenXR: received zero indices for submesh data");
	ERR_FAIL_COND_V_MSG((xrsubmesh_data.indexCountOutput % 3) != 0, false, "OpenXR: expected the number of indices to be divisible by 3");

	// Resize our buffers.
	vertices_buffer.resize(xrsubmesh_data.vertexCountOutput);

	if (p_enable_normals) {
		normals_buffer.resize(xrsubmesh_data.vertexCountOutput);
	} else {
		normals_buffer.resize(0);
	}

	if (p_semantic_label_set == OpenXRAndroidSceneMeshing::SemanticLabelSet::SEMANTIC_LABEL_SET_DEFAULT) {
		vertex_semantics_buffer.resize(xrsubmesh_data.vertexCountOutput);
	} else {
		vertex_semantics_buffer.resize(0);
	}

	indices_buffer.resize(xrsubmesh_data.indexCountOutput);

	xrsubmesh_data = XrSceneSubmeshDataANDROID{
		XR_TYPE_SCENE_SUBMESH_DATA_ANDROID, // type
		nullptr, // next
		xrsubmesh_data.submeshId, // submeshId
		(uint32_t)vertices_buffer.size(), // vertexCapacityInput
		0, // vertexCountOutput
		vertices_buffer.ptr(), // vertexPositions
		normals_buffer.ptr(), // vertexNormals
		vertex_semantics_buffer.ptr(), // vertexSemantics
		(uint32_t)indices_buffer.size(), // indexCapacityInput
		0, // indexCountOutput
		indices_buffer.ptr(), // indices
	};

	// and query again, which will write to our buffers
	result = wrapper->xrGetSubmeshDataANDROID(p_snapshot, 1, &xrsubmesh_data);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get submesh data; ", wrapper->get_openxr_api()->get_error_string(result));
		return false;
	}

	return true;
}
