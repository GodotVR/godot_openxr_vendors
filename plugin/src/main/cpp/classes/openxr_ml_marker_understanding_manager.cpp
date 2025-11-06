/**************************************************************************/
/*  openxr_ml_marker_understanding_manager.cpp                             */
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

#include "classes/openxr_ml_marker_understanding_manager.h"

using namespace godot;

void OpenXRMlMarkerUnderstandingManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_scene", "scene"), &OpenXRMlMarkerUnderstandingManager::set_scene);
	ClassDB::bind_method(D_METHOD("get_scene"), &OpenXRMlMarkerUnderstandingManager::get_scene);

	ClassDB::bind_method(D_METHOD("set_scene_setup_method", "method_name"), &OpenXRMlMarkerUnderstandingManager::set_scene_setup_method);
	ClassDB::bind_method(D_METHOD("get_scene_setup_method"), &OpenXRMlMarkerUnderstandingManager::get_scene_setup_method);

	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &OpenXRMlMarkerUnderstandingManager::set_visible);
	ClassDB::bind_method(D_METHOD("get_visible"), &OpenXRMlMarkerUnderstandingManager::get_visible);
	ClassDB::bind_method(D_METHOD("show"), &OpenXRMlMarkerUnderstandingManager::show);
	ClassDB::bind_method(D_METHOD("hide"), &OpenXRMlMarkerUnderstandingManager::hide);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_scene", "get_scene");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "scene_setup_method", PROPERTY_HINT_NONE, ""), "set_scene_setup_method", "get_scene_setup_method");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible", PROPERTY_HINT_NONE, ""), "set_visible", "get_visible");
}

void OpenXRMlMarkerUnderstandingManager::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			xr_origin = Object::cast_to<XROrigin3D>(get_parent());
			XRServer *xr_server = XRServer::get_singleton();
			xr_server->connect("tracker_added", callable_mp(this, &OpenXRMlMarkerUnderstandingManager::_on_tracker_added));
			xr_server->connect("tracker_removed", callable_mp(this, &OpenXRMlMarkerUnderstandingManager::_on_tracker_removed));
			Array trackers = xr_server->get_trackers(XRServer::TRACKER_ANCHOR).values();
			for (int i = 0; i < trackers.size(); i++) {
				OpenXRMlMarkerTracker *tracker = Object::cast_to<OpenXRMlMarkerTracker>(trackers[i]);
				if (tracker) {
					_add_tracker(tracker);
				}
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			XRServer *xr_server = XRServer::get_singleton();
			xr_server->disconnect("tracker_added", callable_mp(this, &OpenXRMlMarkerUnderstandingManager::_on_tracker_added));
			xr_server->disconnect("tracker_removed", callable_mp(this, &OpenXRMlMarkerUnderstandingManager::_on_tracker_removed));
			_cleanup_anchors();
			xr_origin = nullptr;
		} break;
	}
}

PackedStringArray OpenXRMlMarkerUnderstandingManager::_get_configuration_warnings() const {
	PackedStringArray warnings = Node::_get_configuration_warnings();

	if (is_inside_tree()) {
		XROrigin3D *origin = Object::cast_to<XROrigin3D>(get_parent());
		if (origin == nullptr) {
			warnings.push_back("Must be a child of XROrigin3D");
		}
	}

	return warnings;
}

void OpenXRMlMarkerUnderstandingManager::set_scene(const Ref<PackedScene> &p_scene) {
	scene = p_scene;
}

Ref<PackedScene> OpenXRMlMarkerUnderstandingManager::get_scene() const {
	return scene;
}

void OpenXRMlMarkerUnderstandingManager::set_scene_setup_method(const StringName &p_method) {
	scene_setup_method = p_method;
}

StringName OpenXRMlMarkerUnderstandingManager::get_scene_setup_method() const {
	return scene_setup_method;
}

void OpenXRMlMarkerUnderstandingManager::set_visible(bool p_visible) {
	visible = p_visible;

	for (KeyValue<StringName, Anchor> &E : anchors) {
		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(E.value.node));
		ERR_CONTINUE_MSG(!node, vformat("Cannot find node for anchor %s.", E.key));
		if (node) {
			node->set_visible(p_visible);
		}
	}
}

bool OpenXRMlMarkerUnderstandingManager::get_visible() const {
	return visible;
}

void OpenXRMlMarkerUnderstandingManager::show() {
	set_visible(true);
}

void OpenXRMlMarkerUnderstandingManager::hide() {
	set_visible(false);
}

void OpenXRMlMarkerUnderstandingManager::_add_tracker(const Ref<OpenXRMlMarkerTracker> &p_tracker) {
	ERR_FAIL_COND(!xr_origin);
	StringName tracker_name = p_tracker->get_tracker_name();
	ERR_FAIL_COND(anchors.has(tracker_name));

	XRAnchor3D *node = memnew(XRAnchor3D);
	node->set_name(tracker_name);
	node->set_tracker(tracker_name);
	node->set_visible(visible);
	xr_origin->add_child(node);

	anchors[tracker_name] = Anchor(node, p_tracker);

	if (scene.is_valid()) {
		Node *scene_node = scene->instantiate();
		node->add_child(scene_node);
		scene_node->call(scene_setup_method, p_tracker);
	}
}

void OpenXRMlMarkerUnderstandingManager::_cleanup_anchors() {
	for (KeyValue<StringName, Anchor> &E : anchors) {
		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(E.value.node));
		if (node) {
			Node *parent = node->get_parent();
			if (parent) {
				parent->remove_child(node);
			}
			node->queue_free();
		}
	}
	anchors.clear();
}

void OpenXRMlMarkerUnderstandingManager::_on_tracker_added(const StringName &p_tracker_name, XRServer::TrackerType p_tracker_type) {
	Ref<OpenXRMlMarkerTracker> tracker = XRServer::get_singleton()->get_tracker(p_tracker_name);
	if (!tracker.is_valid())
		return;
	_add_tracker(tracker);
}

void OpenXRMlMarkerUnderstandingManager::_on_tracker_removed(const StringName &p_tracker_name, XRServer::TrackerType p_tracker_type) {
	if (anchors.has(p_tracker_name)) {
		Anchor anchor = anchors[p_tracker_name];
		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(anchor.node));
		if (node) {
			Node *parent = node->get_parent();
			if (parent) {
				parent->remove_child(node);
			}
			node->queue_free();
		}
		anchors.erase(p_tracker_name);
	}
}
