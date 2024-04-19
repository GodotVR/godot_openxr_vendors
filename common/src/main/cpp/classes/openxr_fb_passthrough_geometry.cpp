/**************************************************************************/
/*  openxr_fb_passthrough_geometry.cpp                                    */
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

#include "classes/openxr_fb_passthrough_geometry.h"

#include "extensions/openxr_fb_passthrough_extension_wrapper.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void OpenXRFbPassthroughGeometry::set_mesh(const Ref<Mesh> &p_mesh) {
	if (p_mesh == mesh) {
		return;
	}

	if (geometry_instance) {
		destroy_passthrough_geometry();
	}

	mesh = p_mesh;

	if (mesh.is_null()) {
		if (preview_mesh != nullptr) {
			preview_mesh->queue_free();
			preview_mesh = nullptr;
		}
		return;
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		if (preview_mesh == nullptr) {
			const Color PREVIEW_COLOR = Color(1.0, 0.0, 1.0);

			Ref<StandardMaterial3D> material;
			material.instantiate();
			material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
			material->set_albedo(PREVIEW_COLOR);

			preview_mesh = memnew(MeshInstance3D);
			preview_mesh->set_material_override(material);
			add_child(preview_mesh, false, Node::INTERNAL_MODE_BACK);
		}
		preview_mesh->set_mesh(mesh);
	}

	if (OpenXRFbPassthroughExtensionWrapper::get_singleton()->is_passthrough_started()) {
		create_passthrough_geometry();
	}
}

Ref<Mesh> OpenXRFbPassthroughGeometry::get_mesh() const {
	return mesh;
}

void OpenXRFbPassthroughGeometry::create_passthrough_geometry() {
	geometry_instance = OpenXRFbPassthroughExtensionWrapper::get_singleton()->create_geometry_instance(mesh, get_transform());
	set_notify_local_transform(true);
}

void OpenXRFbPassthroughGeometry::destroy_passthrough_geometry() {
	if (geometry_instance != XR_NULL_HANDLE) {
		OpenXRFbPassthroughExtensionWrapper::get_singleton()->destroy_geometry_instance(geometry_instance);
		geometry_instance = XR_NULL_HANDLE;
	}
}

void OpenXRFbPassthroughGeometry::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			OpenXRFbPassthroughExtensionWrapper::get_singleton()->connect("openxr_fb_projected_passthrough_layer_created", callable_mp(this, &OpenXRFbPassthroughGeometry::create_passthrough_geometry));
			OpenXRFbPassthroughExtensionWrapper::get_singleton()->connect("openxr_fb_passthrough_stopped", callable_mp(this, &OpenXRFbPassthroughGeometry::destroy_passthrough_geometry));
		} break;
		case NOTIFICATION_ENTER_TREE: {
			if (is_visible()) {
				OpenXRFbPassthroughExtensionWrapper::get_singleton()->register_geometry_node(this);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			OpenXRFbPassthroughExtensionWrapper::get_singleton()->unregister_geometry_node(this);

			if (geometry_instance) {
				destroy_passthrough_geometry();
			}
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (is_visible()) {
				OpenXRFbPassthroughExtensionWrapper::get_singleton()->register_geometry_node(this);
			} else {
				OpenXRFbPassthroughExtensionWrapper::get_singleton()->unregister_geometry_node(this);
			}
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			if (geometry_instance) {
				OpenXRFbPassthroughExtensionWrapper::get_singleton()->set_geometry_instance_transform(geometry_instance, get_transform());
			}
		} break;
	}
}

void OpenXRFbPassthroughGeometry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh"), &OpenXRFbPassthroughGeometry::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &OpenXRFbPassthroughGeometry::get_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
}
