/**************************************************************************/
/*  openxr_fb_render_model.cpp                                            */
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

#include "classes/openxr_fb_render_model.h"

#include "extensions/openxr_fb_render_model_extension.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gltf_document.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void OpenXRFbRenderModel::set_render_model_type(Model p_model) {
	render_model_type = p_model;
	if (is_inside_tree() && OpenXRFbRenderModelExtension::get_singleton()->is_openxr_session_active()) {
		load_render_model();
	}
}

OpenXRFbRenderModel::Model OpenXRFbRenderModel::get_render_model_type() {
	return render_model_type;
}

bool OpenXRFbRenderModel::has_render_model_node() {
	return render_model_node != nullptr;
}

void OpenXRFbRenderModel::load_render_model() {
	if (render_model_node != nullptr) {
		render_model_node->queue_free();
		render_model_node = nullptr;
	}

	String render_model_path;
	switch (render_model_type) {
		case MODEL_CONTROLLER_LEFT: {
			render_model_path = "/model_fb/controller/left";
		} break;

		case MODEL_CONTROLLER_RIGHT: {
			render_model_path = "/model_fb/controller/right";
		} break;

		default: {
			render_model_path = "";
		}
	}

	if (render_model_path.is_empty()) {
		return;
	}

	PackedByteArray render_model_buffer = OpenXRFbRenderModelExtension::get_singleton()->get_buffer(render_model_path);
	if (render_model_buffer.is_empty()) {
		UtilityFunctions::print_verbose("Failed to load render model buffer from path [", render_model_path, "] in OpenXRFbRenderModel node");
		return;
	}

	Ref<GLTFDocument> gltf_document;
	gltf_document.instantiate();
	Ref<GLTFState> gltf_state;
	gltf_state.instantiate();

	Error err = gltf_document->append_from_buffer(render_model_buffer, "", gltf_state);
	if (err != OK) {
		UtilityFunctions::print_verbose("Failed to instance render model in OpenXRFbRenderModel node");
		return;
	}

	render_model_node = Object::cast_to<Node3D>(gltf_document->generate_scene(gltf_state));
	if (render_model_node) {
		add_child(render_model_node);
		emit_signal("openxr_fb_render_model_loaded");
	}
}

Node3D *OpenXRFbRenderModel::get_render_model_node() {
	return render_model_node;
}

void OpenXRFbRenderModel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
			if (openxr_interface.is_valid()) {
				openxr_interface->connect("session_begun", callable_mp(this, &OpenXRFbRenderModel::load_render_model));
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			if (OpenXRFbRenderModelExtension::get_singleton()->is_openxr_session_active()) {
				load_render_model();
			}
			if (Engine::get_singleton()->is_editor_hint()) {
				ProjectSettings::get_singleton()->connect("settings_changed", callable_mp((Node *)this, &Node::update_configuration_warnings));
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (Engine::get_singleton()->is_editor_hint()) {
				ProjectSettings::get_singleton()->disconnect("settings_changed", callable_mp((Node *)this, &Node::update_configuration_warnings));
			}
		} break;
	}
}

PackedStringArray OpenXRFbRenderModel::_get_configuration_warnings() const {
	PackedStringArray warnings;
	if (!ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/render_model")) {
		warnings.push_back("The render model extension isn't enabled in project settings. Please enable `xr/openxr/extensions/meta/render_model` to use this node.");
	}
	return warnings;
}

void OpenXRFbRenderModel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_render_model_node"), &OpenXRFbRenderModel::has_render_model_node);
	ClassDB::bind_method(D_METHOD("get_render_model_node"), &OpenXRFbRenderModel::get_render_model_node);
	ClassDB::bind_method(D_METHOD("set_render_model_type", "render_model_type"), &OpenXRFbRenderModel::set_render_model_type);
	ClassDB::bind_method(D_METHOD("get_render_model_type"), &OpenXRFbRenderModel::get_render_model_type);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "render_model_type", PROPERTY_HINT_ENUM, "Left Controller,Right Controller"), "set_render_model_type", "get_render_model_type");

	BIND_ENUM_CONSTANT(MODEL_CONTROLLER_LEFT);
	BIND_ENUM_CONSTANT(MODEL_CONTROLLER_RIGHT);

	ADD_SIGNAL(MethodInfo("openxr_fb_render_model_loaded"));
}
