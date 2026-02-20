/**************************************************************************/
/*  openxr_android_trackable_plane_tracker.h                              */
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

#pragma once

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/templates/local_vector.hpp>

#include "classes/openxr_android_trackable_tracker.h"

using namespace godot;

class OpenXRAndroidTrackablePlaneTracker : public OpenXRAndroidTrackableTracker {
	GDCLASS(OpenXRAndroidTrackablePlaneTracker, OpenXRAndroidTrackableTracker);

public:
	OpenXRAndroidTrackablePlaneTracker();
	virtual ~OpenXRAndroidTrackablePlaneTracker() override;

	enum PlaneType {
		PLANE_TYPE_HORIZONTAL_DOWNWARD_FACING,
		PLANE_TYPE_HORIZONTAL_UPWARD_FACING,
		PLANE_TYPE_VERTICAL,
		PLANE_TYPE_ARBITRARY,
	};

	enum PlaneLabel {
		PLANE_LABEL_UNKNOWN,
		PLANE_LABEL_WALL,
		PLANE_LABEL_FLOOR,
		PLANE_LABEL_CEILING,
		PLANE_LABEL_TABLE,
	};

	static Ref<OpenXRAndroidTrackablePlaneTracker> create(XrTrackableANDROID p_trackable);

	Vector2 get_extents();
	PlaneType get_plane_type();
	PlaneLabel get_plane_label();
	Ref<OpenXRAndroidTrackablePlaneTracker> get_subsumed_by_plane();
	PackedVector3Array get_vertices();
	Ref<ArrayMesh> get_mesh();
	Ref<ConcavePolygonShape3D> get_shape(float p_thickness);

protected:
	static void _bind_methods();

private:
	void _ensure_updated() override;
	void _ensure_updated_impl(bool p_query_vertices);
	bool _get_xrtrackable_info(bool p_called_first_time_for_this_frame, bool p_query_vertices);

	template <typename ResourceType>
	Ref<ResourceType> _get_mesh_or_shape(Ref<ResourceType> &p_mesh_or_shape, float p_thickness = 0.0);

	// NOTE: this is different than the parent's "process_frame". This is for the last_frame that we
	// updated our fields (extents, plane_type, vertices, etc)
	uint64_t last_frame = -1;

	XrTime last_updated_time_vertices = -1;

	Vector2 extents;
	PlaneType plane_type = PLANE_TYPE_ARBITRARY;
	PlaneLabel plane_label = PLANE_LABEL_UNKNOWN;
	Ref<OpenXRAndroidTrackablePlaneTracker> subsumed_by_plane;
	PackedVector3Array vertices;

	XrTrackablePlaneANDROID xr_trackable_plane = {};
	LocalVector<XrVector2f> xrvertices;
	uint32_t vertex_count_output = 0;
	Ref<ArrayMesh> mesh;
	Ref<ConcavePolygonShape3D> shape;
	float shape_thickness = 0.0;
};

VARIANT_ENUM_CAST(OpenXRAndroidTrackablePlaneTracker::PlaneType);
VARIANT_ENUM_CAST(OpenXRAndroidTrackablePlaneTracker::PlaneLabel);
