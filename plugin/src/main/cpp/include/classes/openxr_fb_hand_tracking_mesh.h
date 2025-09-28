/**************************************************************************/
/*  openxr_fb_hand_tracking_mesh.h                                        */
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

#ifndef OPENXR_FB_HAND_TRACKING_MESH_H
#define OPENXR_FB_HAND_TRACKING_MESH_H

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>

namespace godot {
class OpenXRFbHandTrackingMesh : public Skeleton3D {
	GDCLASS(OpenXRFbHandTrackingMesh, Skeleton3D)
public:
	enum Hand {
		HAND_LEFT,
		HAND_RIGHT,
		HAND_MAX
	};

private:
	Hand hand = Hand::HAND_LEFT;
	Ref<Material> material;

	MeshInstance3D *mesh_instance = nullptr;

	void setup_hand_mesh(const Ref<Mesh> &p_hand_mesh);

protected:
	void _notification(int p_what);

	static void _bind_methods();

	void construct_skeleton();

public:
	MeshInstance3D *get_mesh_instance() const;

	void set_hand(Hand p_hand);
	Hand get_hand() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	void set_use_scale_override(bool p_use);
	bool get_use_scale_override() const;

	void set_scale_override(float p_scale);
	float get_scale_override() const;

	virtual PackedStringArray _get_configuration_warnings() const override;
};
} //namespace godot

VARIANT_ENUM_CAST(OpenXRFbHandTrackingMesh::Hand);

#endif
