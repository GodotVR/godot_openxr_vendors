/**************************************************************************/
/*  openxr_fb_spatial_anchor_manager.h                                    */
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

#ifndef OPENXR_FB_SPATIAL_ANCHOR_MANAGER_H
#define OPENXR_FB_SPATIAL_ANCHOR_MANAGER_H

#include <openxr/openxr.h>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "classes/openxr_fb_spatial_entity.h"

namespace godot {
class PackedScene;
class XROrigin3D;
class XRAnchor3D;

class OpenXRFbSpatialAnchorManager : public Node {
	GDCLASS(OpenXRFbSpatialAnchorManager, Node);

	Ref<PackedScene> scene;
	StringName scene_setup_method = "setup_scene";
	bool visible = true;

	XROrigin3D *xr_origin = nullptr;

	struct Anchor {
		ObjectID node;
		Ref<OpenXRFbSpatialEntity> entity;

		Anchor(Node *p_node, const Ref<OpenXRFbSpatialEntity> &p_entity) {
			node = p_node->get_instance_id();
			entity = p_entity;
		}
		Anchor() { }
	};
	HashMap<StringName, Anchor> anchors;

	void _cleanup_anchors();

	void _track_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_new_anchor);

	void _on_anchor_created(bool p_success, const Transform3D &p_transform, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity);
	void _on_anchor_load_query_completed(const Array &p_results, const Dictionary &p_anchors_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location, bool p_erase_unknown_anchors);
	void _on_anchor_track_enable_locatable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_entity, bool p_new_anchor);
	void _on_anchor_track_enable_storable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_entity, bool p_new_anchor);
	void _on_anchor_saved(bool p_succeeded, OpenXRFbSpatialEntity::StorageLocation p_location, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_new_anchor);
	void _complete_anchor_setup(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_new_anchor);

	void _untrack_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity);

	void _on_anchor_untrack_enable_storable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_entity);
	void _on_anchor_erase_completed(bool p_succeeded, OpenXRFbSpatialEntity::StorageLocation p_location, const Ref<OpenXRFbSpatialEntity> &p_entity);

protected:
	void _notification(int p_what);

	void _on_openxr_session_stopping();

	static void _bind_methods();

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

	void create_anchor(const Transform3D &p_transform, const Dictionary &p_custom_data);
	void load_anchor(const StringName &p_uuid, const Dictionary &p_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location);
	void load_anchors(const TypedArray<StringName> &p_uuids, const Dictionary &p_all_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location, bool p_erase_unknown_anchors = false);
	void track_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity);
	void untrack_anchor(const Variant &p_spatial_entity_or_uuid);

	Array get_anchor_uuids() const;
	XRAnchor3D *get_anchor_node(const StringName &p_uuid) const;
	Ref<OpenXRFbSpatialEntity> get_spatial_entity(const StringName &p_uuids) const;
};
} // namespace godot

#endif
