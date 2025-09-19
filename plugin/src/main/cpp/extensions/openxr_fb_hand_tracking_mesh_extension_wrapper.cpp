/**************************************************************************/
/*  openxr_fb_hand_tracking_mesh_extension_wrapper.cpp                    */
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

#include "extensions/openxr_fb_hand_tracking_mesh_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_hand_tracker.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbHandTrackingMeshExtensionWrapper *OpenXRFbHandTrackingMeshExtensionWrapper::singleton = nullptr;

OpenXRFbHandTrackingMeshExtensionWrapper *OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbHandTrackingMeshExtensionWrapper());
	}
	return singleton;
}

OpenXRFbHandTrackingMeshExtensionWrapper::OpenXRFbHandTrackingMeshExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbHandTrackingMeshExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_HAND_TRACKING_MESH_EXTENSION_NAME] = &fb_hand_tracking_mesh_ext;
	singleton = this;
}

OpenXRFbHandTrackingMeshExtensionWrapper::~OpenXRFbHandTrackingMeshExtensionWrapper() {
	cleanup();
}

void OpenXRFbHandTrackingMeshExtensionWrapper::_bind_methods() {
}

void OpenXRFbHandTrackingMeshExtensionWrapper::cleanup() {
	fb_hand_tracking_mesh_ext = false;
	should_fetch_hand_mesh_data = false;

	for (int i = 0; i < Hand::HAND_MAX; i++) {
		if (hand_mesh[i].is_valid()) {
			hand_mesh[i].unref();
		}

		hand_tracking_scale[i].overrideHandScale = false;
		hand_tracking_scale[i].overrideValueInput = 1.0;
		bone_data[i].joint_poses.clear();
		bone_data[i].joint_radii.clear();
		bone_data[i].joint_parents.clear();
	}
}

godot::Dictionary OpenXRFbHandTrackingMeshExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbHandTrackingMeshExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_hand_tracking_mesh_ext) {
		bool result = initialize_fb_hand_tracking_mesh_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_hand_tracking_mesh extension");
			fb_hand_tracking_mesh_ext = false;
		}
	}
}

void OpenXRFbHandTrackingMeshExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

uint64_t OpenXRFbHandTrackingMeshExtensionWrapper::_set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) {
	if (!fb_hand_tracking_mesh_ext) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	hand_tracking_scale[p_hand_index] = {
		XR_TYPE_HAND_TRACKING_SCALE_FB, // type
		p_next_pointer, // next
		1.0, // sensorOutput
		1.0, // currentOutput
		false, // overrideHandScale
		1.0, // overrideValueInput
	};

	return reinterpret_cast<uint64_t>(&hand_tracking_scale[p_hand_index]);
}

void OpenXRFbHandTrackingMeshExtensionWrapper::_on_process() {
	if (!should_fetch_hand_mesh_data) {
		return;
	}

	if (!is_enabled() || get_openxr_api().is_null()) {
		return;
	}

	XrHandTrackerEXT hand_trackers[Hand::HAND_MAX];
	for (int i = 0; i < Hand::HAND_MAX; i++) {
		hand_trackers[i] = reinterpret_cast<XrHandTrackerEXT>(get_openxr_api()->get_hand_tracker(i));
		if (hand_trackers[i] == XR_NULL_HANDLE) {
			// If we're missing a hand tracker, we can't fetch the data.
			return;
		}
	}

	for (int i = 0; i < Hand::HAND_MAX; i++) {
		fetch_hand_mesh_data(Hand(i));
	}

	for (const FetchCallback &fetch_callback : fetch_callbacks) {
		fetch_callback.callable.call(hand_mesh[fetch_callback.hand]);
	}

	fetch_callbacks.clear();
	should_fetch_hand_mesh_data = false;
}

void OpenXRFbHandTrackingMeshExtensionWrapper::set_use_scale_override(Hand p_hand, bool p_use) {
	hand_tracking_scale[p_hand].overrideHandScale = p_use;
}

bool OpenXRFbHandTrackingMeshExtensionWrapper::get_use_scale_override(Hand p_hand) const {
	return hand_tracking_scale[p_hand].overrideHandScale;
}

void OpenXRFbHandTrackingMeshExtensionWrapper::set_scale_override(Hand p_hand, float p_scale) {
	hand_tracking_scale[p_hand].overrideValueInput = p_scale;
}

float OpenXRFbHandTrackingMeshExtensionWrapper::get_scale_override(Hand p_hand) const {
	return hand_tracking_scale[p_hand].overrideValueInput;
}

bool OpenXRFbHandTrackingMeshExtensionWrapper::fetch_hand_mesh_data(Hand p_hand) {
	ERR_FAIL_COND_V_MSG(!is_enabled(), false, "OpenXR extension XR_FB_hand_tracking_mesh is not available");

	XrHandTrackingMeshFB xr_hand_mesh;
	xr_hand_mesh.type = XR_TYPE_HAND_TRACKING_MESH_FB;
	xr_hand_mesh.next = nullptr;
	xr_hand_mesh.jointCapacityInput = 0;
	xr_hand_mesh.vertexCapacityInput = 0;
	xr_hand_mesh.indexCapacityInput = 0;
	XrHandTrackerEXT hand_tracker = reinterpret_cast<XrHandTrackerEXT>(get_openxr_api()->get_hand_tracker(p_hand));

	XrResult result = xrGetHandMeshFB(hand_tracker, &xr_hand_mesh);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to retrieve capacity inputs for OpenXR XR_FB_hand_tracking_mesh extension, error code: ", result);

		if (result == XR_ERROR_FUNCTION_UNSUPPORTED || result == XR_ERROR_FEATURE_UNSUPPORTED) {
			should_fetch_hand_mesh_data = false;
		}

		return false;
	}

	if (xr_hand_mesh.jointCountOutput == 0 || xr_hand_mesh.vertexCountOutput == 0 || xr_hand_mesh.indexCountOutput == 0) {
		WARN_PRINT("Invalid mesh data returned from xrGetHandMeshFB...");
		return false;
	}

	xr_hand_mesh.jointCapacityInput = xr_hand_mesh.jointCountOutput;
	xr_hand_mesh.vertexCapacityInput = xr_hand_mesh.vertexCountOutput;
	xr_hand_mesh.indexCapacityInput = xr_hand_mesh.indexCountOutput;

	// skeleton data persists in BoneData structs
	bone_data[p_hand].joint_poses.resize(xr_hand_mesh.jointCapacityInput);
	xr_hand_mesh.jointBindPoses = bone_data[p_hand].joint_poses.ptr();
	bone_data[p_hand].joint_radii.resize(xr_hand_mesh.jointCapacityInput);
	xr_hand_mesh.jointRadii = bone_data[p_hand].joint_radii.ptr();
	bone_data[p_hand].joint_parents.resize(xr_hand_mesh.jointCapacityInput);
	xr_hand_mesh.jointParents = bone_data[p_hand].joint_parents.ptr();

	// mesh data will be used to construct ArrayMeshes and then be discarded
	LocalVector<XrVector3f> vertex_positions;
	vertex_positions.resize(xr_hand_mesh.vertexCapacityInput);
	xr_hand_mesh.vertexPositions = vertex_positions.ptr();
	LocalVector<XrVector3f> vertex_normals;
	vertex_normals.resize(xr_hand_mesh.vertexCapacityInput);
	xr_hand_mesh.vertexNormals = vertex_normals.ptr();
	LocalVector<XrVector2f> vertex_uvs;
	vertex_uvs.resize(xr_hand_mesh.vertexCapacityInput);
	xr_hand_mesh.vertexUVs = vertex_uvs.ptr();
	LocalVector<XrVector4sFB> vertex_blend_indices;
	vertex_blend_indices.resize(xr_hand_mesh.vertexCapacityInput);
	xr_hand_mesh.vertexBlendIndices = vertex_blend_indices.ptr();
	LocalVector<XrVector4f> vertex_blend_weights;
	vertex_blend_weights.resize(xr_hand_mesh.vertexCapacityInput);
	xr_hand_mesh.vertexBlendWeights = vertex_blend_weights.ptr();
	LocalVector<int16_t> indices;
	indices.resize(xr_hand_mesh.indexCapacityInput);
	xr_hand_mesh.indices = indices.ptr();

	result = xrGetHandMeshFB(hand_tracker, &xr_hand_mesh);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to retrieve hand mesh data for OpenXR XR_FB_hand_tracking_mesh extension, error code: ", result);

		if (result == XR_ERROR_FUNCTION_UNSUPPORTED || result == XR_ERROR_FEATURE_UNSUPPORTED) {
			should_fetch_hand_mesh_data = false;
		}

		return false;
	}

	// convert to clockwise winding order to cull correct face side
	const int VERTICES_PER_TRIANGLE = 3;
	for (int i = 0; i < xr_hand_mesh.indexCapacityInput; i += VERTICES_PER_TRIANGLE) {
		SWAP(xr_hand_mesh.indices[i], xr_hand_mesh.indices[i + VERTICES_PER_TRIANGLE - 1]);
	}

	PackedVector3Array godot_vertex_positions = PackedVector3Array();
	PackedVector3Array godot_vertex_normals = PackedVector3Array();
	PackedVector2Array godot_vertex_uvs = PackedVector2Array();
	PackedInt32Array godot_bone_indices = PackedInt32Array();
	PackedFloat32Array godot_bone_weights = PackedFloat32Array();
	for (int i = 0; i < xr_hand_mesh.vertexCapacityInput; i++) {
		godot_vertex_positions.push_back(Vector3(xr_hand_mesh.vertexPositions[i].x, xr_hand_mesh.vertexPositions[i].y, xr_hand_mesh.vertexPositions[i].z));
		godot_vertex_normals.push_back(Vector3(xr_hand_mesh.vertexNormals[i].x, xr_hand_mesh.vertexNormals[i].y, xr_hand_mesh.vertexNormals[i].z));
		godot_vertex_uvs.push_back(Vector2(xr_hand_mesh.vertexUVs[i].x, xr_hand_mesh.vertexUVs[i].y));

		godot_bone_indices.push_back(xr_hand_mesh.vertexBlendIndices[i].x);
		godot_bone_indices.push_back(xr_hand_mesh.vertexBlendIndices[i].y);
		godot_bone_indices.push_back(xr_hand_mesh.vertexBlendIndices[i].z);
		godot_bone_indices.push_back(xr_hand_mesh.vertexBlendIndices[i].w);

		godot_bone_weights.push_back(xr_hand_mesh.vertexBlendWeights[i].x);
		godot_bone_weights.push_back(xr_hand_mesh.vertexBlendWeights[i].y);
		godot_bone_weights.push_back(xr_hand_mesh.vertexBlendWeights[i].z);
		godot_bone_weights.push_back(xr_hand_mesh.vertexBlendWeights[i].w);
	}

	PackedInt32Array godot_indices = PackedInt32Array();
	for (int i = 0; i < xr_hand_mesh.indexCapacityInput; i++) {
		godot_indices.push_back(xr_hand_mesh.indices[i]);
	}

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = godot_vertex_positions;
	arrays[Mesh::ARRAY_NORMAL] = godot_vertex_normals;
	arrays[Mesh::ARRAY_TEX_UV] = godot_vertex_uvs;
	arrays[Mesh::ARRAY_BONES] = godot_bone_indices;
	arrays[Mesh::ARRAY_WEIGHTS] = godot_bone_weights;
	arrays[Mesh::ARRAY_INDEX] = godot_indices;

	Ref<ArrayMesh> array_mesh;
	array_mesh.instantiate();
	array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

	hand_mesh[p_hand] = array_mesh;

	return true;
}

void OpenXRFbHandTrackingMeshExtensionWrapper::request_hand_mesh_data(Hand p_hand, const Callable &p_callback) {
	if (!is_enabled()) {
		p_callback.call(Ref<Mesh>());
		return;
	}

	if (hand_mesh[p_hand].is_null()) {
		should_fetch_hand_mesh_data = true;
		fetch_callbacks.push_back({ p_hand, p_callback });
	} else {
		p_callback.call(hand_mesh[p_hand]);
	}
}

void OpenXRFbHandTrackingMeshExtensionWrapper::construct_skeleton(Skeleton3D *r_skeleton) {
	r_skeleton->clear_bones();

	const String bone_names[XRHandTracker::HAND_JOINT_MAX] = {
		"LeftPalm",
		"LeftHand",
		"LeftThumbMetacarpal",
		"LeftThumbProximal",
		"LeftThumbDistal",
		"LeftThumbTip",
		"LeftIndexMetacarpal",
		"LeftIndexProximal",
		"LeftIndexIntermediate",
		"LeftIndexDistal",
		"LeftIndexTip",
		"LeftMiddleMetacarpal",
		"LeftMiddleProximal",
		"LeftMiddleIntermediate",
		"LeftMiddleDistal",
		"LeftMiddleTip",
		"LeftRingMetacarpal",
		"LeftRingProximal",
		"LeftRingIntermediate",
		"LeftRingDistal",
		"LeftRingTip",
		"LeftLittleMetacarpal",
		"LeftLittleProximal",
		"LeftLittleIntermediate",
		"LeftLittleDistal",
		"LeftLittleTip",
	};

	const int bone_parents[XRHandTracker::HAND_JOINT_MAX] = {
		1, -1, 1, 2, 3, 4, 1, 6, 7, 8, 9, 1, 11, 12, 13, 14, 1, 16, 17, 18, 19, 1, 21, 22, 23, 24
	};

	for (int i = 0; i < XRHandTracker::HAND_JOINT_MAX; i++) {
		r_skeleton->add_bone(bone_names[i]);

		if (i > 1) {
			r_skeleton->set_bone_parent(i, bone_parents[i]);
		}
	}

	// Palm bone is added before wrist, so parent it here
	r_skeleton->set_bone_parent(0, 1);
}

void OpenXRFbHandTrackingMeshExtensionWrapper::reset_skeleton_pose(Hand p_hand, Skeleton3D *r_skeleton) {
	ERR_FAIL_COND_MSG(!is_enabled(), "OpenXR extension XR_FB_hand_tracking_mesh is not available");
	ERR_FAIL_COND_MSG(hand_mesh[p_hand].is_null(), "OpenXR extension XR_FB_hand_tracking_mesh has not populated mesh data");
	ERR_FAIL_NULL_MSG(r_skeleton, "Skeleton3D r_skeleton not valid");

	for (int i = 0; i < XRHandTracker::HAND_JOINT_MAX; i++) {
		int parent_index = r_skeleton->get_bone_parent(i);

		// rotation adjustment to conform with SKELETON_RIG_HUMANOID
		const Quaternion rot_adjustment(0.0, -Math_SQRT12, Math_SQRT12, 0.0);

		XrQuaternionf rot = bone_data[p_hand].joint_poses[i].orientation;
		XrVector3f pos = bone_data[p_hand].joint_poses[i].position;
		Transform3D transform = Transform3D(Quaternion(rot.x, rot.y, rot.z, rot.w) * rot_adjustment, Vector3(pos.x, pos.y, pos.z));

		if (i == 1) {
			r_skeleton->set_bone_rest(i, transform);
		} else {
			XrQuaternionf parent_rot = bone_data[p_hand].joint_poses[parent_index].orientation;
			XrVector3f parent_pos = bone_data[p_hand].joint_poses[parent_index].position;
			Transform3D parent_transform = Transform3D(Quaternion(parent_rot.x, parent_rot.y, parent_rot.z, parent_rot.w) * rot_adjustment, Vector3(parent_pos.x, parent_pos.y, parent_pos.z));
			r_skeleton->set_bone_rest(i, parent_transform.inverse() * transform);
		}
	}

	r_skeleton->reset_bone_poses();
}

bool OpenXRFbHandTrackingMeshExtensionWrapper::initialize_fb_hand_tracking_mesh_extension(const XrInstance instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetHandMeshFB);

	return true;
}
