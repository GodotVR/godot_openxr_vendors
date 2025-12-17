/**************************************************************************/
/*  openxr_ml_marker_understanding_manager.h                              */
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

#pragma once

#include <openxr/openxr.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/xr_anchor3d.hpp>
#include <godot_cpp/classes/xr_origin3d.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "classes/openxr_ml_marker_tracker.h"

namespace godot {
class OpenXRMlMarkerUnderstandingManager : public Node {
	GDCLASS(OpenXRMlMarkerUnderstandingManager, Node);

private:
	Ref<PackedScene> scene;
	StringName scene_setup_method = "setup_scene";
	bool visible = true;

	XROrigin3D *xr_origin = nullptr;

	struct Anchor {
		ObjectID node;
		Ref<OpenXRMlMarkerTracker> entity;

		Anchor(Node *p_node, const Ref<OpenXRMlMarkerTracker> &p_entity) {
			node = p_node->get_instance_id();
			entity = p_entity;
		}
		Anchor() {}
	};
	HashMap<StringName, Anchor> anchors;

	void _cleanup_anchors();
	void _add_tracker(const Ref<OpenXRMlMarkerTracker> &p_tracker);
	void _on_tracker_added(const StringName &p_tracker_name, XRServer::TrackerType p_tracker_type);
	void _on_tracker_removed(const StringName &p_tracker_name, XRServer::TrackerType p_tracker_type);

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	PackedStringArray _get_configuration_warnings() const override;

	void set_scene(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_scene() const;

	void set_scene_setup_method(const StringName &p_method);
	StringName get_scene_setup_method() const;

	void set_visible(bool p_visible);
	bool get_visible() const;
	void show();
	void hide();
};
} // namespace godot
