/**************************************************************************/
/*  openxr_fb_hand_tracking_mesh.cpp                                      */
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

#include "classes/openxr_fb_hand_tracking_mesh.h"

#include "extensions/openxr_fb_hand_tracking_mesh_extension_wrapper.h"

#include <godot_cpp/classes/xr_hand_modifier3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void OpenXRFbHandTrackingMesh::setup_hand_mesh(Hand p_hand) {
	ERR_FAIL_COND_MSG(skeleton != nullptr, "OpenXRFbHandTrackingMesh skeleton has already been set up");

	if (p_hand == hand) {
		skeleton = memnew(Skeleton3D);
		OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->construct_skeleton(p_hand, skeleton);
		add_child(skeleton);

		mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_mesh(OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->get_mesh(p_hand));
		mesh_instance->set_material_override(material);
		skeleton->add_child(mesh_instance);
	}

	XRHandModifier3D *parent = Object::cast_to<XRHandModifier3D>(get_parent());
	if (parent) {
		parent->set_target(parent->get_path_to(skeleton));
	}
}

Skeleton3D *OpenXRFbHandTrackingMesh::get_skeleton() const {
	return skeleton;
}

MeshInstance3D *OpenXRFbHandTrackingMesh::get_mesh_instance() const {
	return mesh_instance;
}

void OpenXRFbHandTrackingMesh::set_hand(Hand p_hand) {
	hand = p_hand;
}

XRHandTracker::Hand OpenXRFbHandTrackingMesh::get_hand() const {
	return hand;
}

void OpenXRFbHandTrackingMesh::set_material(const Ref<Material> &p_material) {
	material = p_material;
	if (mesh_instance) {
		mesh_instance->set_material_override(material);
	}
}

Ref<Material> OpenXRFbHandTrackingMesh::get_material() const {
	return material;
}

void OpenXRFbHandTrackingMesh::set_use_scale_override(bool p_use) {
	OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->set_use_scale_override(hand, p_use);
}

bool OpenXRFbHandTrackingMesh::get_use_scale_override() const {
	return OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->get_use_scale_override(hand);
}

void OpenXRFbHandTrackingMesh::set_scale_override(float p_scale) {
	OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->set_scale_override(hand, p_scale);
}

float OpenXRFbHandTrackingMesh::get_scale_override() const {
	return OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->get_scale_override(hand);
}

void OpenXRFbHandTrackingMesh::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->enable_fetch_hand_mesh_data();
			OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->connect("openxr_fb_hand_tracking_mesh_data_fetched", callable_mp(this, &OpenXRFbHandTrackingMesh::setup_hand_mesh));
		} break;
	}
}

void OpenXRFbHandTrackingMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_skeleton"), &OpenXRFbHandTrackingMesh::get_skeleton);
	ClassDB::bind_method(D_METHOD("get_mesh_instance"), &OpenXRFbHandTrackingMesh::get_mesh_instance);
	ClassDB::bind_method(D_METHOD("set_hand", "hand"), &OpenXRFbHandTrackingMesh::set_hand);
	ClassDB::bind_method(D_METHOD("get_hand"), &OpenXRFbHandTrackingMesh::get_hand);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &OpenXRFbHandTrackingMesh::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &OpenXRFbHandTrackingMesh::get_material);
	ClassDB::bind_method(D_METHOD("set_use_scale_override", "use_scale_override"), &OpenXRFbHandTrackingMesh::set_use_scale_override);
	ClassDB::bind_method(D_METHOD("get_use_scale_override"), &OpenXRFbHandTrackingMesh::get_use_scale_override);
	ClassDB::bind_method(D_METHOD("set_scale_override", "scale_override"), &OpenXRFbHandTrackingMesh::set_scale_override);
	ClassDB::bind_method(D_METHOD("get_scale_override"), &OpenXRFbHandTrackingMesh::get_scale_override);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "hand", PROPERTY_HINT_ENUM, "Left Hand,Right Hand"), "set_hand", "get_hand");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial", PROPERTY_USAGE_DEFAULT), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_scale_override", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_use_scale_override", "get_use_scale_override");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_override", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_scale_override", "get_scale_override");

	BIND_ENUM_CONSTANT(Hand::HAND_LEFT);
	BIND_ENUM_CONSTANT(Hand::HAND_RIGHT);
}
