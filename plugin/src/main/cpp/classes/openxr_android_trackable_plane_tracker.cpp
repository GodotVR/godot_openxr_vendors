/**************************************************************************/
/*  openxr_android_trackable_plane_tracker.cpp                            */
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

#include "classes/openxr_android_trackable_plane_tracker.h"

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "extensions/openxr_android_trackables_extension.h"
#include "godot_cpp/classes/xr_server.hpp"

using namespace godot;

OpenXRAndroidTrackablePlaneTracker::OpenXRAndroidTrackablePlaneTracker() {}
OpenXRAndroidTrackablePlaneTracker::~OpenXRAndroidTrackablePlaneTracker() {}

Ref<OpenXRAndroidTrackablePlaneTracker> OpenXRAndroidTrackablePlaneTracker::create(XrTrackableANDROID p_trackable) {
	Ref<OpenXRAndroidTrackablePlaneTracker> ret{};
	if (p_trackable == XR_NULL_TRACKABLE_ANDROID) {
		return ret;
	}

	ret.instantiate();
	ret->_init(p_trackable, XR_TRACKABLE_TYPE_PLANE_ANDROID);
	ret->xr_trackable_plane = {
		XR_TYPE_TRACKABLE_PLANE_ANDROID, // type
		nullptr, // next
		{}, // trackingState
		{}, // centerPose
		{}, // extents
		{}, // planeType
		{}, // planeLabel
		{}, // subsumedByPlane
		{}, // lastUpdatedTime
		0, // vertexCapacityInput
		&ret->vertex_count_output, // vertexCountOutput
		nullptr, // vertices
	};

	return ret;
}

Vector2 OpenXRAndroidTrackablePlaneTracker::get_extents() {
	_ensure_updated();
	return extents;
}

OpenXRAndroidTrackablePlaneTracker::PlaneType OpenXRAndroidTrackablePlaneTracker::get_plane_type() {
	_ensure_updated();
	return plane_type;
}

OpenXRAndroidTrackablePlaneTracker::PlaneLabel OpenXRAndroidTrackablePlaneTracker::get_plane_label() {
	_ensure_updated();
	return plane_label;
}

Ref<OpenXRAndroidTrackablePlaneTracker> OpenXRAndroidTrackablePlaneTracker::get_subsumed_by_plane() {
	_ensure_updated();
	return subsumed_by_plane;
}

PackedVector3Array OpenXRAndroidTrackablePlaneTracker::get_vertices() {
	_ensure_updated_impl(true);
	return vertices;
}

Ref<ArrayMesh> OpenXRAndroidTrackablePlaneTracker::get_mesh() {
	return _get_mesh_or_shape(mesh);
}

Ref<ConcavePolygonShape3D> OpenXRAndroidTrackablePlaneTracker::get_shape(float p_thickness) {
	return _get_mesh_or_shape(shape, p_thickness);
}

void OpenXRAndroidTrackablePlaneTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_extents"), &OpenXRAndroidTrackablePlaneTracker::get_extents);
	ClassDB::bind_method(D_METHOD("get_plane_type"), &OpenXRAndroidTrackablePlaneTracker::get_plane_type);
	ClassDB::bind_method(D_METHOD("get_plane_label"), &OpenXRAndroidTrackablePlaneTracker::get_plane_label);
	ClassDB::bind_method(D_METHOD("get_subsumed_by_plane"), &OpenXRAndroidTrackablePlaneTracker::get_subsumed_by_plane);
	ClassDB::bind_method(D_METHOD("get_vertices"), &OpenXRAndroidTrackablePlaneTracker::get_vertices);
	ClassDB::bind_method(D_METHOD("get_mesh"), &OpenXRAndroidTrackablePlaneTracker::get_mesh);
	ClassDB::bind_method(D_METHOD("get_shape", "thickness"), &OpenXRAndroidTrackablePlaneTracker::get_shape);

	BIND_ENUM_CONSTANT(PLANE_TYPE_HORIZONTAL_DOWNWARD_FACING);
	BIND_ENUM_CONSTANT(PLANE_TYPE_HORIZONTAL_UPWARD_FACING);
	BIND_ENUM_CONSTANT(PLANE_TYPE_VERTICAL);
	BIND_ENUM_CONSTANT(PLANE_TYPE_ARBITRARY);
	BIND_ENUM_CONSTANT(PLANE_LABEL_UNKNOWN);
	BIND_ENUM_CONSTANT(PLANE_LABEL_WALL);
	BIND_ENUM_CONSTANT(PLANE_LABEL_FLOOR);
	BIND_ENUM_CONSTANT(PLANE_LABEL_CEILING);
	BIND_ENUM_CONSTANT(PLANE_LABEL_TABLE);
}

void OpenXRAndroidTrackablePlaneTracker::_ensure_updated() {
	_ensure_updated_impl(false);
}

void OpenXRAndroidTrackablePlaneTracker::_ensure_updated_impl(bool p_query_vertices) {
	// This function can be called multiple times per frame, however it will only do work at most 2
	// times per frame if...
	// 1: the first call is when !p_query_vertices, then
	// 2: the second call is when p_query_vertices and
	//    lastUpdatedTime != last_updated_time_vertices
	// Additional calls will do zero work, until the next frame.
	// Additionally, it is normal for this function to be called recursively; the recursive
	// calls will either early-return or attempt to query and copy vertices.
	uint64_t frame = Engine::get_singleton()->get_process_frames();
	if (!p_query_vertices && last_frame == frame) {
		return;
	}

	// If this function is being called for the first time this frame, ensure vertex count is
	// retrieved. xrvertices.is_empty() == false if vertices were retrieved
	// (see _get_xrtrackable_info)
	bool called_first_time_for_this_frame = last_frame != frame;
	last_frame = frame;

	// NOTE there is likely a way to early-return here when !called_first_time_for_this_frame && don't
	// need to query vertices, though the logic seems tricky/confusing. So leave it alone since this
	// function is complex enough as it is.

	if (!_get_xrtrackable_info(called_first_time_for_this_frame, p_query_vertices)) {
		_set_tracking_state(XR_TRACKING_STATE_STOPPED_ANDROID);
		return;
	}

	// copy vertices if they were queried
	if (!xrvertices.is_empty()) {
		vertices.resize(xrvertices.size());
		Vector3 *dst = vertices.ptrw();
		for (int i = 0; i < xrvertices.size(); ++i) {
			// xrvertices are on the XZ plane
			dst[i] = Vector3(xrvertices[i].x, 0.0, xrvertices[i].y);
		}

		xrvertices.clear();

		// mesh and shape are no longer valid; regenerate if they're retrieved again later
		mesh.unref();
		shape.unref();
	}

	if (called_first_time_for_this_frame) {
		// always set pose, even when old last updated time == xr_trackable_plane.lastUpdatedTime, since
		// it could have changed due to XR changing the reference space
		extents.x = xr_trackable_plane.extents.width;
		extents.y = xr_trackable_plane.extents.height;
		_set_center_pose(xr_trackable_plane.centerPose);

		// recursively create and/or update the subsumed planes (they may need to update pose too)
		OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
		subsumed_by_plane = wrapper->get_or_create_tracker_and_update(xr_trackable_plane.subsumedByPlane, XR_TRACKABLE_TYPE_PLANE_ANDROID, true);

		if (_set_last_updated_time(xr_trackable_plane.lastUpdatedTime) == xr_trackable_plane.lastUpdatedTime) {
			return;
		}

		switch (xr_trackable_plane.planeType) {
			case XR_PLANE_TYPE_HORIZONTAL_DOWNWARD_FACING_ANDROID:
				plane_type = PLANE_TYPE_HORIZONTAL_DOWNWARD_FACING;
				break;
			case XR_PLANE_TYPE_HORIZONTAL_UPWARD_FACING_ANDROID:
				plane_type = PLANE_TYPE_HORIZONTAL_UPWARD_FACING;
				break;
			case XR_PLANE_TYPE_VERTICAL_ANDROID:
				plane_type = PLANE_TYPE_VERTICAL;
				break;
			case XR_PLANE_TYPE_ARBITRARY_ANDROID:
				plane_type = PLANE_TYPE_ARBITRARY;
				break;
			case XR_PLANE_TYPE_MAX_ENUM_ANDROID:
			default:
				UtilityFunctions::printerr("OpenXR: invalid plane type: ", xr_trackable_plane.planeType);
				_set_tracking_state(XR_TRACKING_STATE_STOPPED_ANDROID);
				return;
		}

		switch (xr_trackable_plane.planeLabel) {
			case XR_PLANE_LABEL_UNKNOWN_ANDROID:
				plane_label = PLANE_LABEL_UNKNOWN;
				break;
			case XR_PLANE_LABEL_WALL_ANDROID:
				plane_label = PLANE_LABEL_WALL;
				break;
			case XR_PLANE_LABEL_FLOOR_ANDROID:
				plane_label = PLANE_LABEL_FLOOR;
				break;
			case XR_PLANE_LABEL_CEILING_ANDROID:
				plane_label = PLANE_LABEL_CEILING;
				break;
			case XR_PLANE_LABEL_TABLE_ANDROID:
				plane_label = PLANE_LABEL_TABLE;
				break;
			case XR_PLANE_LABEL_MAX_ENUM_ANDROID:
			default:
				UtilityFunctions::printerr("OpenXR: invalid plane label: ", xr_trackable_plane.planeLabel);
				_set_tracking_state(XR_TRACKING_STATE_STOPPED_ANDROID);
				return;
		}

		_set_tracking_state(xr_trackable_plane.trackingState);

		// Any recursive calls this 'updated' signal brings will either update vertices or
		// early-return. It should never infinitely recurse.
		emit_signal(StringName("updated"));
	}
}

bool OpenXRAndroidTrackablePlaneTracker::_get_xrtrackable_info(bool p_called_first_time_for_this_frame, bool p_query_vertices) {
	if (get_trackable() == XR_NULL_TRACKABLE_ANDROID) {
		return false;
	}

	OpenXRAndroidTrackablesExtension *wrapper = OpenXRAndroidTrackablesExtension::get_singleton();
	XrTrackableTrackerANDROID trackable_tracker = wrapper->get_or_create_xrtrackable_tracker(XR_TRACKABLE_TYPE_PLANE_ANDROID);
	if (trackable_tracker == XR_NULL_HANDLE || xr_trackable_plane.vertexCountOutput == nullptr) {
		return false;
	}

	XrTrackableGetInfoANDROID get_info = {
		XR_TYPE_TRACKABLE_GET_INFO_ANDROID, // type
		nullptr, // next
		get_trackable(), // trackable
		(XrSpace)wrapper->get_openxr_api()->get_play_space(), // baseSpace
		(XrTime)wrapper->get_openxr_api()->get_predicted_display_time(), // time
	};

	if (p_called_first_time_for_this_frame) {
		vertex_count_output = 0;
		xr_trackable_plane.vertices = nullptr;
		xr_trackable_plane.vertexCapacityInput = 0;
		XrResult result = wrapper->xrGetTrackablePlaneANDROID(trackable_tracker, &get_info, &xr_trackable_plane);
		if (result != XR_SUCCESS) {
			UtilityFunctions::printerr("OpenXR: Failed to get trackable plane; ", wrapper->get_openxr_api()->get_error_string(result));
			return false;
		}
	}

	// Retrieve the vertices, if there are any to retrieve
	// NOTE: xrvertices is used to determine if we queried for vertices, so we must clear() if we
	// failed to query
	// NOTE: currently lastUpdatedTime refers to when any plane data changed, not just vertices. So we
	// may copy the exact same data as last time since we have no way of knowning otherwise.
	if (0 < vertex_count_output && p_query_vertices && xr_trackable_plane.lastUpdatedTime != last_updated_time_vertices) {
		last_updated_time_vertices = xr_trackable_plane.lastUpdatedTime;
		xrvertices.resize(vertex_count_output);
		xr_trackable_plane.vertexCapacityInput = vertex_count_output;
		xr_trackable_plane.vertices = xrvertices.ptr();
		XrResult result = wrapper->xrGetTrackablePlaneANDROID(trackable_tracker, &get_info, &xr_trackable_plane);
		if (result != XR_SUCCESS || vertex_count_output == 0) {
			WARN_PRINT("OpenXR: Failed to re-query plane vertices");
			xr_trackable_plane.vertexCapacityInput = 0;
			xr_trackable_plane.vertices = nullptr;
			xrvertices.clear();
		}
	}

	return true;
}

template <typename ResourceType>
Ref<ResourceType> OpenXRAndroidTrackablePlaneTracker::_get_mesh_or_shape(Ref<ResourceType> &p_mesh_or_shape, float p_thickness) {
	static_assert(std::is_same_v<ResourceType, ArrayMesh> || std::is_same_v<ResourceType, ConcavePolygonShape3D>, "ResourceType must be ArrayMesh or ConcavePolygonShape3D");

	// p_mesh_or_shape is invalidated if get_vertices() copied new vertices; so we don't have new
	// vertices if it's still valid (or if the thickness hasn't changed for the shape)
	PackedVector3Array vertices_plus_first = get_vertices();
	if (p_mesh_or_shape.is_valid() && (std::is_same_v<ResourceType, ArrayMesh> || p_thickness == shape_thickness)) {
		return p_mesh_or_shape;
	}

	PackedVector3Array faces;
	if constexpr (std::is_same_v<ResourceType, ArrayMesh>) {
		// See for loop below for why we need 3 * vertices.size() Vector3s
		faces.resize(3 * vertices.size());
	} else {
		// Negative thickness is okay (it'll just extend the collider up instead of down)
		if (p_thickness == 0.0) {
			WARN_PRINT("OpenXR: Not creating shape; thickness must be != 0.0");

			// don't return p_mesh_or_shape since it might be valid; we must always return null here
			return Ref<ResourceType>();
		}

		// See for loop below for why we need 12 * vertices.size() Vector3s
		faces.resize(12 * vertices.size());
	}
	Vector3 *write = faces.ptrw();

	if (vertices_plus_first.size() < 4) {
		WARN_PRINT("OpenXR: Expected plane to have at least four points");
		return p_mesh_or_shape;
	}
	vertices_plus_first.push_back(vertices_plus_first[0]);
	const Vector3 *read = vertices_plus_first.ptr();

	// The vertices outline the plane CCW, on the XZ plane
	int idx = 0;
	for (int vertices_idx = 0; vertices_idx < vertices.size(); ++vertices_idx) {
		if constexpr (std::is_same_v<ResourceType, ArrayMesh>) {
			// p0 and p1 are the next two vertices in the plane
			//
			// Draw a triangle with these vertices + Vector3(), CW
			//
			//    Vector3()
			//       /\
			//      /  \
			//     /    \
			//    /      \
			//   /        \
			// p0 -------- p1
			//
			// p0 == read[vertices_idx]
			// p1 == read[vertices_idx + 1]

			write[idx] = read[vertices_idx];
			write[idx + 1] = Vector3();
			write[idx + 2] = read[vertices_idx + 1];

			idx += 3;
		} else {
			// p0 and p1 are the next two vertices in the plane
			// p2 and p3 are p0 and p1 at p_thickness away
			// (recall the points lie on the XZ plane, so thickness extends vertically)
			//
			//    Vector3()
			//       /\
			//      /  \
			//     /top \
			//    /      \
			//   /   /\   \
			// p0 -------- p1 (p1.x, 0.0, p1.z)
			// |   /    \  |
			// |  /      \ | edge
			// | / bottom \|
			// p3 -------- p2 (p1.x, -p_thickness, p1.z)
			//
			Vector3 p0 = read[vertices_idx];
			Vector3 p1 = read[vertices_idx + 1];
			Vector3 p2 = p1 - Vector3(0.0, p_thickness, 0.0);
			Vector3 p3 = p0 - Vector3(0.0, p_thickness, 0.0);

			// top triangle (CW; this is identical to the triangle used for the mesh)
			write[idx] = p0;
			write[idx + 1] = Vector3();
			write[idx + 2] = p1;

			// bottom triangle (notice that the winding is CCW since it's on the other side)
			write[idx + 3] = p2;
			write[idx + 4] = Vector3(0.0, -p_thickness, 0.0);
			write[idx + 5] = p3;

			// edge between top and bottom (rectangle; requires 2 triangles)
			write[idx + 6] = p0;
			write[idx + 7] = p1;
			write[idx + 8] = p3;

			write[idx + 9] = p1;
			write[idx + 10] = p2;
			write[idx + 11] = p3;

			idx += 12;
		}
	}

	p_mesh_or_shape.instantiate();
	if constexpr (std::is_same_v<ResourceType, ArrayMesh>) {
		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);
		arrays[Mesh::ARRAY_VERTEX] = faces;

		p_mesh_or_shape->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	} else {
		shape_thickness = p_thickness;
		p_mesh_or_shape->set_faces(faces);
	}

	return p_mesh_or_shape;
}
