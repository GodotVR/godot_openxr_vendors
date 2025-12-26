/**************************************************************************/
/*  openxr_android_scene_submesh_data.h                                   */
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

#ifndef OPENXR_ANDROID_SCENE_SUBMESH_DATA_H
#define OPENXR_ANDROID_SCENE_SUBMESH_DATA_H

#include "classes/openxr_android_scene_meshing.h"
#include <androidxr/androidxr.h>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

using namespace godot;

class OpenXRAndroidSceneSubmeshData : public RefCounted {
	GDCLASS(OpenXRAndroidSceneSubmeshData, RefCounted);

public:
	OpenXRAndroidSceneSubmeshData();
	virtual ~OpenXRAndroidSceneSubmeshData() override;

	enum UpdateState {
		UPDATE_STATE_CREATED,
		UPDATE_STATE_UNCHANGED,
		UPDATE_STATE_UPDATED,
		UPDATE_STATE_DELETED,
	};

	enum SemanticLabel {
		SEMANTIC_LABEL_OTHER,
		SEMANTIC_LABEL_FLOOR,
		SEMANTIC_LABEL_CEILING,
		SEMANTIC_LABEL_WALL,
		SEMANTIC_LABEL_TABLE,
	};

	void set_is_deleted(uint64_t p_update_idx);
	uint64_t get_update_idx() const;
	void update(UpdateState p_update_state, uint64_t p_update_idx, const StringName &p_submesh_uuid, const XrSceneSubmeshStateANDROID &p_submesh_state, XrSceneMeshSnapshotANDROID p_snapshot, bool p_enable_normals, OpenXRAndroidSceneMeshing::SemanticLabelSet p_semantic_label_set);

	Array get_arrays() const;
	Array get_indexed_arrays() const;
	Array get_vertex_semantics() const;
	Array get_indexed_vertex_semantics() const;
	StringName get_submesh_id() const;
	int64_t get_last_update_time() const;
	const Transform3D &get_transform() const;
	const Vector3 &get_bounds() const;
	UpdateState get_update_state() const;

protected:
	static void _bind_methods();

private:
	template <bool IsIndexedArray, bool CopyNormals, bool CopySemantics>
	Array _create_arrays_from_buffers() const;

	template <bool IsIndexedArray>
	Array _get_arrays() const;

	bool _update(UpdateState p_update_state, uint64_t p_update_idx, const StringName &p_submesh_uuid, const XrSceneSubmeshStateANDROID &p_submesh_state, XrSceneMeshSnapshotANDROID p_snapshot, bool p_enable_normals, OpenXRAndroidSceneMeshing::SemanticLabelSet p_semantic_label_set);

	UpdateState update_state = UPDATE_STATE_CREATED;
	uint32_t update_idx = 0;

	LocalVector<XrVector3f> vertices_buffer;
	LocalVector<XrVector3f> normals_buffer;
	LocalVector<uint32_t> indices_buffer;
	LocalVector<uint8_t> vertex_semantics_buffer;
	StringName submesh_uuid;
	XrTime last_update_time;
	Transform3D submesh_pose_in_base_space;
	Vector3 bounds;
};

VARIANT_ENUM_CAST(OpenXRAndroidSceneSubmeshData::UpdateState);
VARIANT_ENUM_CAST(OpenXRAndroidSceneSubmeshData::SemanticLabel);

#endif // OPENXR_ANDROID_SCENE_SUBMESH_DATA_H
