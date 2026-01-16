/**************************************************************************/
/*  openxr_android_scene_meshing.cpp                                      */
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

#include "classes/openxr_android_scene_meshing.h"
#include "classes/openxr_android_scene_submesh_data.h"
#include "extensions/openxr_android_scene_meshing_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidSceneMeshing::OpenXRAndroidSceneMeshing() {}

OpenXRAndroidSceneMeshing::~OpenXRAndroidSceneMeshing() {
	OpenXRAndroidSceneMeshingExtensionWrapper *wrapper = OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton();
	if (wrapper != nullptr && tracker != XR_NULL_HANDLE) {
		wrapper->xrDestroySceneMeshingTrackerANDROID(tracker);
		tracker = XR_NULL_HANDLE;
	}
}

bool OpenXRAndroidSceneMeshing::initialize(SemanticLabelSet p_semantic_label_set, bool p_enable_normals) {
	OpenXRAndroidSceneMeshingExtensionWrapper *wrapper = OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton();
	ERR_FAIL_NULL_V_MSG(wrapper, false, "Cannot create tracker data without an OpenXRAndroidSceneMeshingExtensionWrapper");

	if (tracker != XR_NULL_HANDLE) {
		if (p_semantic_label_set == semantic_label_set && enable_normals == p_enable_normals) {
			return true;
		}

		WARN_PRINT("Already initialized; consider creating a different instance if this instance is shared");
	}

	XrSceneMeshSemanticLabelSetANDROID xrsemantic_label_set;
	switch (p_semantic_label_set) {
		case SEMANTIC_LABEL_SET_NONE:
			xrsemantic_label_set = XR_SCENE_MESH_SEMANTIC_LABEL_SET_NONE_ANDROID;
			break;
		case SEMANTIC_LABEL_SET_DEFAULT:
			xrsemantic_label_set = XR_SCENE_MESH_SEMANTIC_LABEL_SET_DEFAULT_ANDROID;
			break;
		default:
			UtilityFunctions::printerr("OpenXR: received an invalid semantic label: ", p_semantic_label_set);
			return false;
	}

	XrSceneMeshingTrackerCreateInfoANDROID create_info = {
		XR_TYPE_SCENE_MESHING_TRACKER_CREATE_INFO_ANDROID, // type
		nullptr, // next
		xrsemantic_label_set, // semanticLabelSet
		(XrBool32)(p_enable_normals ? XR_TRUE : XR_FALSE) // enableNormals
	};
	XrResult result = wrapper->xrCreateSceneMeshingTrackerANDROID((XrSession)wrapper->get_openxr_api()->get_session(), &create_info, &tracker);
	if (result != XR_SUCCESS || tracker == XR_NULL_HANDLE) {
		UtilityFunctions::printerr("OpenXR: Failed to create scene meshing tracker; ", wrapper->get_openxr_api()->get_error_string(result));
		return false;
	}

	semantic_label_set = p_semantic_label_set;
	enable_normals = p_enable_normals;

	return true;
}

OpenXRAndroidSceneMeshing::SemanticLabelSet OpenXRAndroidSceneMeshing::get_semantic_label_set() const {
	return semantic_label_set;
}

bool OpenXRAndroidSceneMeshing::are_normals_enabled() const {
	return enable_normals;
}

Dictionary OpenXRAndroidSceneMeshing::get_submesh_data(const Transform3D &p_pose, const Vector3 &p_extents) {
	OpenXRAndroidSceneMeshingExtensionWrapper *wrapper = OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton();
	ERR_FAIL_NULL_V_MSG(wrapper, Dictionary(), "Cannot get submesh data without an OpenXRAndroidSceneMeshingExtensionWrapper");

	XrSceneMeshSnapshotANDROID snapshot = _create_snapshot(p_pose, p_extents);
	if (snapshot == XR_NULL_HANDLE) {
		return Dictionary();
	}

	LocalVector<XrSceneSubmeshStateANDROID> submesh_states = _get_all_submesh_states(snapshot);

	// run through the states and compare to what we got last time
	++update_idx;
	for (const XrSceneSubmeshStateANDROID &submesh_state : submesh_states) {
		StringName uuid = OpenXRUtilities::uuid_to_string_name(submesh_state.submeshId);
		Ref<OpenXRAndroidSceneSubmeshData> submesh;
		OpenXRAndroidSceneSubmeshData::UpdateState update_state;
		if (submeshes.has(uuid)) {
			submesh = submeshes[uuid];

			if (submesh->get_update_state() == OpenXRAndroidSceneSubmeshData::UPDATE_STATE_DELETED) {
				update_state = OpenXRAndroidSceneSubmeshData::UPDATE_STATE_CREATED;
			} else if (submesh->get_last_update_time() == submesh_state.lastUpdatedTime) {
				update_state = OpenXRAndroidSceneSubmeshData::UPDATE_STATE_UNCHANGED;
			} else {
				update_state = OpenXRAndroidSceneSubmeshData::UPDATE_STATE_UPDATED;
			}
		} else {
			submesh.instantiate();
			submeshes[uuid] = submesh;

			update_state = OpenXRAndroidSceneSubmeshData::UPDATE_STATE_CREATED;
		}

		submesh->update(update_state, update_idx, uuid, submesh_state, snapshot, enable_normals, semantic_label_set);
	}

	// update submeshes dictionary to prune or set "deleted" state
	Array keys = submeshes.keys();
	for (int i = 0; i < keys.size(); ++i) {
		Ref<OpenXRAndroidSceneSubmeshData> submesh = submeshes[keys[i]];

		// skip if this submesh was updated just now above
		if (submesh->get_update_idx() == update_idx) {
			continue;
		}

		// this submesh was deleted last time and the caller was told about it.  We don't need to hold onto
		// it any longer
		if (submesh->get_update_state() == OpenXRAndroidSceneSubmeshData::UPDATE_STATE_DELETED) {
			submeshes.erase(keys[i]);
			continue;
		}

		// this submesh was not updated by the xr runtime, so it must've been deleted
		submesh->set_is_deleted(update_idx);
	}

	wrapper->xrDestroySceneMeshSnapshotANDROID(snapshot);
	return submeshes;
}

void OpenXRAndroidSceneMeshing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize", "semantic_label_set", "enable_normals"), &OpenXRAndroidSceneMeshing::initialize);
	ClassDB::bind_method(D_METHOD("get_semantic_label_set"), &OpenXRAndroidSceneMeshing::get_semantic_label_set);
	ClassDB::bind_method(D_METHOD("are_normals_enabled"), &OpenXRAndroidSceneMeshing::are_normals_enabled);
	ClassDB::bind_method(D_METHOD("get_submesh_data", "pose", "extents"), &OpenXRAndroidSceneMeshing::get_submesh_data);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_SET_NONE);
	BIND_ENUM_CONSTANT(SEMANTIC_LABEL_SET_DEFAULT);
}

XrSceneMeshSnapshotANDROID OpenXRAndroidSceneMeshing::_create_snapshot(const Transform3D &p_pose, const Vector3 &p_extents) {
	OpenXRAndroidSceneMeshingExtensionWrapper *wrapper = OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton();
	ERR_FAIL_NULL_V_MSG(wrapper, XR_NULL_HANDLE, "Cannot create snapshot without an OpenXRAndroidSceneMeshingExtensionWrapper");

	Quaternion quat = p_pose.basis.get_quaternion();
	XrSceneMeshSnapshotCreateInfoANDROID create_info{
		XR_TYPE_SCENE_MESH_SNAPSHOT_CREATE_INFO_ANDROID, // type
		nullptr, // next
		(XrSpace)wrapper->get_openxr_api()->get_play_space(), // baseSpace
		(XrTime)wrapper->get_openxr_api()->get_predicted_display_time(), // time
		XrBoxf{
				XrPosef{ XrQuaternionf{ quat.x, quat.y, quat.z, quat.w }, XrVector3f{ p_pose.origin.x, p_pose.origin.y, p_pose.origin.z } },
				XrExtent3Df{ p_extents.x, p_extents.y, p_extents.z } } // boundingBox
	};
	XrSceneMeshSnapshotCreationResultANDROID snapshot_creation_result{
		XR_TYPE_SCENE_MESH_SNAPSHOT_CREATION_RESULT_ANDROID, // type
		nullptr, // next
		XR_NULL_HANDLE, // snapshot
		XR_SCENE_MESH_TRACKING_STATE_MAX_ENUM_ANDROID // trackingState
	};
	XrResult result = wrapper->xrCreateSceneMeshSnapshotANDROID(tracker, &create_info, &snapshot_creation_result);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to create scene mesh snapshot; ", wrapper->get_openxr_api()->get_error_string(result));
		return XR_NULL_HANDLE;
	}

	if (snapshot_creation_result.trackingState != XR_SCENE_MESH_TRACKING_STATE_TRACKING_ANDROID) {
		wrapper->xrDestroySceneMeshSnapshotANDROID(snapshot_creation_result.snapshot);
		return XR_NULL_HANDLE;
	}

	return snapshot_creation_result.snapshot;
}

LocalVector<XrSceneSubmeshStateANDROID> OpenXRAndroidSceneMeshing::_get_all_submesh_states(XrSceneMeshSnapshotANDROID snapshot) {
	LocalVector<XrSceneSubmeshStateANDROID> ret;
	OpenXRAndroidSceneMeshingExtensionWrapper *wrapper = OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton();
	ERR_FAIL_NULL_V_MSG(wrapper, ret, "Cannot create a snapshot without an OpenXRAndroidSceneMeshingExtensionWrapper");

	if (snapshot == XR_NULL_HANDLE) {
		UtilityFunctions::printerr("OpenXR: Failed to get all submesh states; snapshot is null");
		return ret;
	}

	uint32_t submesh_state_count_output = 0;
	XrResult result = wrapper->xrGetAllSubmeshStatesANDROID(snapshot, 0, &submesh_state_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get all submesh states; ", wrapper->get_openxr_api()->get_error_string(result));
		return ret;
	}

	if (submesh_state_count_output == 0) {
		// This is okay, we may get mesh states next time.
		return ret;
	}

	ret.resize(submesh_state_count_output);
	for (XrSceneSubmeshStateANDROID &submesh_state : ret) {
		submesh_state = {
			XR_TYPE_SCENE_SUBMESH_STATE_ANDROID, //type
			nullptr, // next
			0, // submeshId
			0, // lastUpdatedTime
			{}, // submeshPoseInBaseSpace
			{}, // bounds
		};
	}

	result = wrapper->xrGetAllSubmeshStatesANDROID(snapshot, submesh_state_count_output, &submesh_state_count_output, ret.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to get all submesh states; ", wrapper->get_openxr_api()->get_error_string(result));
		return ret;
	}

	if (ret.size() != submesh_state_count_output) {
		WARN_PRINT("OpenXR: get all submesh states returned a different count");
		if (ret.size() < submesh_state_count_output) {
			UtilityFunctions::printerr("OpenXR: somehow received more submesh states on the second query; ", wrapper->get_openxr_api()->get_error_string(result));
			return ret;
		}

		if (submesh_state_count_output == 0) {
			UtilityFunctions::printerr("OpenXR: second get all submesh states returned zero");
			return ret;
		}

		ret.resize(submesh_state_count_output);
	}

	return ret;
}
