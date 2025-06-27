/**************************************************************************/
/*  openxr_fb_scene_manager.cpp                                           */
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

#include "classes/openxr_fb_scene_manager.h"

#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/xr_anchor3d.hpp>
#include <godot_cpp/classes/xr_origin3d.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "classes/openxr_fb_spatial_entity_query.h"
#include "extensions/openxr_fb_scene_capture_extension_wrapper.h"
#include "extensions/openxr_fb_scene_extension_wrapper.h"

using namespace godot;

void OpenXRFbSceneManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_default_scene", "packed_scene"), &OpenXRFbSceneManager::set_default_scene);
	ClassDB::bind_method(D_METHOD("get_default_scene"), &OpenXRFbSceneManager::get_default_scene);

	ClassDB::bind_method(D_METHOD("set_scene_setup_method", "method_name"), &OpenXRFbSceneManager::set_scene_setup_method);
	ClassDB::bind_method(D_METHOD("get_scene_setup_method"), &OpenXRFbSceneManager::get_scene_setup_method);

	ClassDB::bind_method(D_METHOD("set_auto_create", "enable"), &OpenXRFbSceneManager::set_auto_create);
	ClassDB::bind_method(D_METHOD("get_auto_create"), &OpenXRFbSceneManager::get_auto_create);

	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &OpenXRFbSceneManager::set_visible);
	ClassDB::bind_method(D_METHOD("get_visible"), &OpenXRFbSceneManager::get_visible);
	ClassDB::bind_method(D_METHOD("show"), &OpenXRFbSceneManager::show);
	ClassDB::bind_method(D_METHOD("hide"), &OpenXRFbSceneManager::hide);

	ClassDB::bind_method(D_METHOD("create_scene_anchors"), &OpenXRFbSceneManager::create_scene_anchors);
	ClassDB::bind_method(D_METHOD("remove_scene_anchors"), &OpenXRFbSceneManager::remove_scene_anchors);
	ClassDB::bind_method(D_METHOD("are_scene_anchors_created"), &OpenXRFbSceneManager::are_scene_anchors_created);

	ClassDB::bind_method(D_METHOD("request_scene_capture", "request"), &OpenXRFbSceneManager::request_scene_capture, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("is_scene_capture_enabled"), &OpenXRFbSceneManager::is_scene_capture_enabled);
	ClassDB::bind_method(D_METHOD("is_scene_capture_supported"), &OpenXRFbSceneManager::is_scene_capture_supported);

	ClassDB::bind_method(D_METHOD("get_anchor_uuids"), &OpenXRFbSceneManager::get_anchor_uuids);
	ClassDB::bind_method(D_METHOD("get_anchor_node", "uuid"), &OpenXRFbSceneManager::get_anchor_node);
	ClassDB::bind_method(D_METHOD("get_spatial_entity", "uuid"), &OpenXRFbSceneManager::get_spatial_entity);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_default_scene", "get_default_scene");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "scene_setup_method", PROPERTY_HINT_NONE, ""), "set_scene_setup_method", "get_scene_setup_method");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_create", PROPERTY_HINT_NONE, ""), "set_auto_create", "get_auto_create");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible", PROPERTY_HINT_NONE, ""), "set_visible", "get_visible");

	ADD_SIGNAL(MethodInfo("openxr_fb_scene_anchor_created", PropertyInfo(Variant::Type::OBJECT, "scene_node"), PropertyInfo(Variant::Type::OBJECT, "spatial_entity")));
	ADD_SIGNAL(MethodInfo("openxr_fb_scene_data_missing"));
	ADD_SIGNAL(MethodInfo("openxr_fb_scene_capture_completed", PropertyInfo(Variant::Type::BOOL, "success")));
}

bool OpenXRFbSceneManager::_set(const StringName &p_name, const Variant &p_value) {
	PackedStringArray parts = p_name.split("/", true, 2);
	if (parts.size() == 2 && parts[0] == "scenes") {
		const PackedStringArray &semantic_labels = OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels();
		if (semantic_labels.has(parts[1])) {
			scenes[parts[1]] = p_value;
			return true;
		}
	}
	return false;
}

bool OpenXRFbSceneManager::_get(const StringName &p_name, Variant &r_ret) const {
	PackedStringArray parts = p_name.split("/", true, 2);
	if (parts.size() == 2 && parts[0] == "scenes") {
		const PackedStringArray &semantic_labels = OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels();
		if (semantic_labels.has(parts[1])) {
			const Ref<PackedScene> *scene = scenes.getptr(parts[1]);
			r_ret = scene ? Variant(*scene) : Variant();
			return true;
		}
	}
	return false;
}

void OpenXRFbSceneManager::_get_property_list(List<PropertyInfo> *p_list) const {
	const PackedStringArray &semantic_labels = OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels();
	for (int i = 0; i < semantic_labels.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::Type::OBJECT, "scenes/" + semantic_labels[i], PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"));
	}
}

void OpenXRFbSceneManager::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
			if (openxr_interface.is_valid()) {
				openxr_interface->connect("session_begun", callable_mp(this, &OpenXRFbSceneManager::_on_openxr_session_begun));
				openxr_interface->connect("session_stopping", callable_mp(this, &OpenXRFbSceneManager::_on_openxr_session_stopping));
			}

			xr_origin = Object::cast_to<XROrigin3D>(get_parent());
			if (xr_origin && auto_create && openxr_interface.is_valid() && openxr_interface->is_initialized()) {
				create_scene_anchors();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
			if (openxr_interface.is_valid()) {
				openxr_interface->disconnect("session_begun", callable_mp(this, &OpenXRFbSceneManager::_on_openxr_session_begun));
				openxr_interface->disconnect("session_stopping", callable_mp(this, &OpenXRFbSceneManager::_on_openxr_session_stopping));
			}

			if (xr_origin && anchors_created) {
				remove_scene_anchors();
			}
			xr_origin = nullptr;
		} break;
	}
}

void OpenXRFbSceneManager::_on_openxr_session_begun() {
	if (xr_origin && auto_create && !anchors_created) {
		create_scene_anchors();
	}
}

void OpenXRFbSceneManager::_on_openxr_session_stopping() {
	if (anchors_created) {
		remove_scene_anchors();
	}
}

PackedStringArray OpenXRFbSceneManager::_get_configuration_warnings() const {
	PackedStringArray warnings = Node::_get_configuration_warnings();

	if (is_inside_tree()) {
		XROrigin3D *origin = Object::cast_to<XROrigin3D>(get_parent());
		if (origin == nullptr) {
			warnings.push_back("Must be a child of XROrigin3D");
		}
	}

	return warnings;
}

void OpenXRFbSceneManager::set_default_scene(const Ref<PackedScene> &p_default_scene) {
	default_scene = p_default_scene;
}

Ref<PackedScene> OpenXRFbSceneManager::get_default_scene() const {
	return default_scene;
}

void OpenXRFbSceneManager::set_scene_setup_method(const StringName &p_method) {
	scene_setup_method = p_method;
}

StringName OpenXRFbSceneManager::get_scene_setup_method() const {
	return scene_setup_method;
}

void OpenXRFbSceneManager::set_auto_create(bool p_auto_create) {
	auto_create = p_auto_create;
}

bool OpenXRFbSceneManager::get_auto_create() const {
	return auto_create;
}

void OpenXRFbSceneManager::set_visible(bool p_visible) {
	visible = p_visible;

	for (KeyValue<StringName, Anchor> &E : anchors) {
		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(E.value.node));
		ERR_CONTINUE_MSG(!node, vformat("Cannot find node for anchor %s.", E.key));
		if (node) {
			node->set_visible(p_visible);
		}
	}
}

bool OpenXRFbSceneManager::get_visible() const {
	return visible;
}

void OpenXRFbSceneManager::show() {
	set_visible(true);
}

void OpenXRFbSceneManager::hide() {
	set_visible(false);
}

Error OpenXRFbSceneManager::create_scene_anchors() {
	ERR_FAIL_COND_V(anchors_created, ERR_ALREADY_EXISTS);
	ERR_FAIL_COND_V(!xr_origin, FAILED);

	// Find the room layout entity.
	Ref<OpenXRFbSpatialEntityQuery> query;
	query.instantiate();
	query->query_by_component(OpenXRFbSpatialEntity::COMPONENT_TYPE_ROOM_LAYOUT);
	query->connect("openxr_fb_spatial_entity_query_completed", callable_mp(this, &OpenXRFbSceneManager::_on_room_layout_query_completed));

	Error ret = query->execute();

	if (ret == OK) {
		// Count as created right away so we don't double create the anchors.
		anchors_created = true;
	} else {
		ERR_PRINT("OpenXRFbSceneManager: Unable to query room layout.");
	}

	return ret;
}

void OpenXRFbSceneManager::_on_room_layout_query_completed(Array p_results) {
	Array anchor_uuids;

	for (int i = 0; i < p_results.size(); i++) {
		Ref<OpenXRFbSpatialEntity> room_layout = p_results[i];
		ERR_CONTINUE(room_layout.is_null());
		anchor_uuids.append_array(room_layout->get_contained_uuids());
	}

	if (anchor_uuids.size() == 0) {
		anchors_created = false;
		emit_signal("openxr_fb_scene_data_missing");
		return;
	}

	// Find all the anchors that are part of the room layout.
	Ref<OpenXRFbSpatialEntityQuery> query;
	query.instantiate();
	query->query_by_uuid(anchor_uuids);
	query->connect("openxr_fb_spatial_entity_query_completed", callable_mp(this, &OpenXRFbSceneManager::_on_anchor_query_completed));
	ERR_FAIL_COND(query->execute() != OK);
}

void OpenXRFbSceneManager::_on_anchor_query_completed(const Array &p_results) {
	for (int i = 0; i < p_results.size(); i++) {
		Ref<OpenXRFbSpatialEntity> entity = p_results[i];
		ERR_CONTINUE(entity.is_null());

		Ref<PackedScene> packed_scene = get_scene_for_entity(entity);
		if (packed_scene.is_null()) {
			// If the developer doesn't give a default or a specific scene, that's fine, just skip it.
			continue;
		}

		// Ensure that the spatial entity is locatable before creating the anchor.
		if (entity->is_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE)) {
			_create_scene_anchor(entity, packed_scene);
		} else if (entity->is_component_supported(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE)) {
			entity->connect("openxr_fb_spatial_entity_set_component_enabled_completed", callable_mp(this, &OpenXRFbSceneManager::_on_anchor_enable_locatable_completed).bind(entity, packed_scene), CONNECT_ONE_SHOT);
			entity->set_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE, true);
		}
	}
}

void OpenXRFbSceneManager::_on_anchor_enable_locatable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_entity, const Ref<PackedScene> &p_packed_scene) {
	ERR_FAIL_COND_MSG(!p_succeeded, vformat("Unable to make scene anchor %s locatable.", p_entity->get_uuid()));
	_create_scene_anchor(p_entity, p_packed_scene);
}

void OpenXRFbSceneManager::_create_scene_anchor(const Ref<OpenXRFbSpatialEntity> &p_entity, const Ref<PackedScene> &p_packed_scene) {
	p_entity->track();

	XRAnchor3D *node = memnew(XRAnchor3D);
	node->set_name(p_entity->get_uuid());
	node->set_tracker(p_entity->get_uuid());
	node->set_visible(visible);
	xr_origin->add_child(node);

	Node *scene = p_packed_scene->instantiate();
	node->add_child(scene);

	anchors[p_entity->get_uuid()] = Anchor(node, p_entity);

	scene->call(scene_setup_method, p_entity);
	emit_signal("openxr_fb_scene_anchor_created", scene, p_entity);
}

Ref<PackedScene> OpenXRFbSceneManager::get_scene_for_entity(const Ref<OpenXRFbSpatialEntity> &p_entity) const {
	PackedStringArray semantic_labels = p_entity->get_semantic_labels();

	// @todo Allow developers to override which label is selected.
	String selected_label = semantic_labels.size() > 0 ? semantic_labels[0] : String();

	if (!selected_label.is_empty() && scenes.has(selected_label)) {
		return scenes[selected_label];
	}

	return default_scene;
}

void OpenXRFbSceneManager::remove_scene_anchors() {
	ERR_FAIL_COND(!anchors_created);

	for (KeyValue<StringName, Anchor> &E : anchors) {
		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(E.value.node));
		if (node) {
			Node *parent = node->get_parent();
			if (parent) {
				parent->remove_child(node);
			}
			node->queue_free();
		}

		E.value.entity->untrack();
	}
	anchors.clear();

	anchors_created = false;
}

bool OpenXRFbSceneManager::are_scene_anchors_created() const {
	return anchors_created;
}

bool OpenXRFbSceneManager::request_scene_capture(const String &p_request) const {
	ObjectID *userdata = memnew(ObjectID(get_instance_id()));
	return OpenXRFbSceneCaptureExtensionWrapper::get_singleton()->request_scene_capture(p_request, &OpenXRFbSceneManager::_scene_capture_callback, userdata);
}

bool OpenXRFbSceneManager::is_scene_capture_enabled() const {
	return OpenXRFbSceneCaptureExtensionWrapper::get_singleton()->is_scene_capture_enabled();
}

bool OpenXRFbSceneManager::is_scene_capture_supported() const {
	return OpenXRFbSceneCaptureExtensionWrapper::get_singleton()->is_scene_capture_supported();
}

void OpenXRFbSceneManager::_scene_capture_callback(XrResult p_result, void *p_userdata) {
	ObjectID *userdata = (ObjectID *)p_userdata;

	OpenXRFbSceneManager *self = Object::cast_to<OpenXRFbSceneManager>(ObjectDB::get_instance(*userdata));
	if (self) {
		self->emit_signal("openxr_fb_scene_capture_completed", XR_SUCCEEDED(p_result));
	}

	memdelete(userdata);
}

Array OpenXRFbSceneManager::get_anchor_uuids() const {
	ERR_FAIL_COND_V(!anchors_created, Array());

	Array ret;
	ret.resize(anchors.size());
	int i = 0;
	for (const KeyValue<StringName, Anchor> &E : anchors) {
		ret[i++] = E.key;
	}
	return ret;
}

XRAnchor3D *OpenXRFbSceneManager::get_anchor_node(const StringName &p_uuid) const {
	ERR_FAIL_COND_V(!anchors_created, nullptr);

	const Anchor *anchor = anchors.getptr(p_uuid);
	if (anchor) {
		return Object::cast_to<XRAnchor3D>(ObjectDB::get_instance(anchor->node));
	}

	return nullptr;
}

Ref<OpenXRFbSpatialEntity> OpenXRFbSceneManager::get_spatial_entity(const StringName &p_uuid) const {
	ERR_FAIL_COND_V(!anchors_created, nullptr);

	const Anchor *anchor = anchors.getptr(p_uuid);
	if (anchor) {
		return anchor->entity;
	}

	return Ref<OpenXRFbSpatialEntity>();
}
