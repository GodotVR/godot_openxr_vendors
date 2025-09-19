/**************************************************************************/
/*  openxr_fb_hand_tracking_mesh_extension_wrapper.h                      */
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

#ifndef OPENXR_FB_HAND_TRACKING_MESH_EXTENSION_WRAPPER_H
#define OPENXR_FB_HAND_TRACKING_MESH_EXTENSION_WRAPPER_H

#include "classes/openxr_fb_hand_tracking_mesh.h"

#include <openxr/openxr.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook XR hand tracking mesh extension.
class OpenXRFbHandTrackingMeshExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbHandTrackingMeshExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	struct BoneData {
		LocalVector<XrPosef> joint_poses;
		LocalVector<float> joint_radii;
		LocalVector<XrHandJointEXT> joint_parents;
	};

	using Hand = OpenXRFbHandTrackingMesh::Hand;

	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;

	uint64_t _set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) override;

	void _on_process() override;

	bool is_enabled() { return fb_hand_tracking_mesh_ext; }

	static OpenXRFbHandTrackingMeshExtensionWrapper *get_singleton();

	void set_use_scale_override(Hand p_hand, bool p_use);
	bool get_use_scale_override(Hand p_hand) const;

	void set_scale_override(Hand p_hand, float p_scale);
	float get_scale_override(Hand p_hand) const;

	void request_hand_mesh_data(Hand p_hand, const Callable &p_callback);
	void construct_skeleton(Skeleton3D *r_skeleton);
	void reset_skeleton_pose(Hand p_hand, Skeleton3D *r_skeleton);

	OpenXRFbHandTrackingMeshExtensionWrapper();
	~OpenXRFbHandTrackingMeshExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC2(xrGetHandMeshFB,
			(XrHandTrackerEXT), handTracker,
			(XrHandTrackingMeshFB *), mesh);

	bool initialize_fb_hand_tracking_mesh_extension(const XrInstance instance);

	void cleanup();

	static OpenXRFbHandTrackingMeshExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;

	bool fb_hand_tracking_mesh_ext = false;

	struct FetchCallback {
		Hand hand;
		Callable callable;
	};
	LocalVector<FetchCallback> fetch_callbacks;

	bool should_fetch_hand_mesh_data = false;
	Ref<ArrayMesh> hand_mesh[Hand::HAND_MAX];
	BoneData bone_data[Hand::HAND_MAX];
	XrHandTrackingScaleFB hand_tracking_scale[Hand::HAND_MAX];

	bool fetch_hand_mesh_data(Hand p_hand);
};

#endif // OPENXR_FB_HAND_TRACKING_MESH_EXTENSION_WRAPPER_H
