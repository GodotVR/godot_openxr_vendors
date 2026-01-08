/**************************************************************************/
/*  openxr_android_meshing.h                                              */
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

#ifndef OPENXR_ANDROID_SCENE_MESHING_H
#define OPENXR_ANDROID_SCENE_MESHING_H

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/local_vector.hpp>

using namespace godot;

class OpenXRAndroidSceneMeshing : public RefCounted {
	GDCLASS(OpenXRAndroidSceneMeshing, RefCounted);

public:
	OpenXRAndroidSceneMeshing();
	virtual ~OpenXRAndroidSceneMeshing() override;

	enum SemanticLabelSet {
		SEMANTIC_LABEL_SET_NONE,
		SEMANTIC_LABEL_SET_DEFAULT,
	};

	bool initialize(SemanticLabelSet p_semantic_label_set, bool p_enable_normals);
	SemanticLabelSet get_semantic_label_set() const;
	bool are_normals_enabled() const;
	Dictionary get_submesh_data(const Transform3D &p_pose, const Vector3 &p_extents);

protected:
	static void _bind_methods();

private:
	XrSceneMeshSnapshotANDROID _create_snapshot(const Transform3D &p_pose, const Vector3 &p_extents);
	LocalVector<XrSceneSubmeshStateANDROID> _get_all_submesh_states(XrSceneMeshSnapshotANDROID xr_snapshot);

	XrSceneMeshingTrackerANDROID tracker = XR_NULL_HANDLE;
	SemanticLabelSet semantic_label_set = SEMANTIC_LABEL_SET_NONE;
	bool enable_normals = false;
	uint64_t update_idx = 0;
	Dictionary submeshes;
};

VARIANT_ENUM_CAST(OpenXRAndroidSceneMeshing::SemanticLabelSet);

#endif // OPENXR_ANDROID_SCENE_MESHING_H
