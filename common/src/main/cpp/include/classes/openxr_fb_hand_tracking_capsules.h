/**************************************************************************/
/*  openxr_fb_hand_tracking_capsules.h                                    */
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

#ifndef OPENXR_FB_HAND_TRACKING_CAPSULES_H
#define OPENXR_FB_HAND_TRACKING_CAPSULES_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/animatable_body3d.hpp>
#include <godot_cpp/classes/physics_material.hpp>

namespace godot {

class OpenXRFbHandTrackingCapsules : public Node3D {
	GDCLASS(OpenXRFbHandTrackingCapsules, Node3D)

public:
	enum Hand {
		LEFT_HAND,
		RIGHT_HAND
	};

private:
	AnimatableBody3D *animatable_bodies[XR_HAND_TRACKING_CAPSULE_COUNT_FB];
	bool are_bodies_in_scene_tree = false;

	Hand hand = LEFT_HAND;
	bool sync_to_physics = true;
	Ref<PhysicsMaterial> physics_material_override;
	Vector3 constant_linear_velocity;
	Vector3 constant_angular_velocity;
	CollisionObject3D::DisableMode disable_mode = CollisionObject3D::DisableMode::DISABLE_MODE_REMOVE;
	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;

protected:
	static void _bind_methods();

public:
	void _ready() override;

	void _physics_process(double delta) override;

	void place_capsules();

	void set_hand(Hand p_hand);
	Hand get_hand() const;

	void set_sync_to_physics(bool p_enable);
	bool is_sync_to_physics_enabled() const;

	void set_physics_material_override(const Ref<PhysicsMaterial> &p_physics_material_override);
	Ref<PhysicsMaterial> get_physics_material_override() const;

	void set_constant_linear_velocity(const Vector3 &p_vel);
	Vector3 get_constant_linear_velocity() const;

	void set_constant_angular_velocity(const Vector3 &p_vel);
	Vector3 get_constant_angular_velocity() const;

	void set_disable_mode(CollisionObject3D::DisableMode p_mode);
	CollisionObject3D::DisableMode get_disable_mode() const;

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	PackedStringArray _get_configuration_warnings() const override;
};

} //namespace godot

VARIANT_ENUM_CAST(OpenXRFbHandTrackingCapsules::Hand);

#endif
