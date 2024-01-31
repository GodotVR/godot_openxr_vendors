/**************************************************************************/
/*  openxr_fb_hand_tracking_capsules.cpp                                  */
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

#include "classes/openxr_fb_hand_tracking_capsules.h"

#include "extensions/openxr_fb_hand_tracking_capsules_extension_wrapper.h"

#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/xr_origin3d.hpp>

using namespace godot;

void OpenXRFbHandTrackingCapsules::_ready() {
	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		AnimatableBody3D *animatable_body = memnew(AnimatableBody3D);
		animatable_bodies[i] = animatable_body;

		animatable_body->set_sync_to_physics(sync_to_physics);
		animatable_body->set_physics_material_override(physics_material_override);
		animatable_body->set_constant_linear_velocity(constant_linear_velocity);
		animatable_body->set_constant_angular_velocity(constant_angular_velocity);
		animatable_body->set_disable_mode(disable_mode);
		animatable_body->set_collision_layer(collision_layer);
		animatable_body->set_collision_mask(collision_mask);

		CollisionShape3D *collision_shape = memnew(CollisionShape3D);
		Ref<CapsuleShape3D> shape;
		shape.instantiate();
		collision_shape->set_shape(shape);
		animatable_body->add_child(collision_shape);
	}
}

void OpenXRFbHandTrackingCapsules::_physics_process(double delta) {
	if (!Engine::get_singleton()->is_editor_hint()) {
		place_capsules();
	}
}

void OpenXRFbHandTrackingCapsules::place_capsules() {
	XrHandCapsuleFB *capsules = OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton()->get_hand_capsules(hand);

	// points will be NaN until an instance of valid hand tracking takes place for corresponding hand
	if (Math::is_nan(capsules[0].points[0].x)) {
		return;
	}

	// on first instance of valid hand tracking, add the animatable bodies to the scene tree
	if (!are_bodies_in_scene_tree) {
		for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
			add_child(animatable_bodies[i]);
		}
		are_bodies_in_scene_tree = true;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		XrHandCapsuleFB capsule = capsules[i];

		XrVector3f xr_vec1 = capsules[i].points[0];
		XrVector3f xr_vec2 = capsules[i].points[1];
		Vector3 vec1 = Vector3(xr_vec1.x, xr_vec1.y, xr_vec1.z);
		Vector3 vec2 = Vector3(xr_vec2.x, xr_vec2.y, xr_vec2.z);

		AnimatableBody3D *animatable_body = animatable_bodies[i];
		CollisionShape3D *collision_shape;
		for (int i = 0; i < animatable_body->get_child_count(); i++) {
			collision_shape = Object::cast_to<CollisionShape3D>(animatable_body->get_child(i));
			if (collision_shape) {
				break;
			}
		}

		if (collision_shape == nullptr) {
			continue;
		}

		float height = vec1.distance_to(vec2);
		Ref<CapsuleShape3D> shape = collision_shape->get_shape();
		shape->set_height(height);
		shape->set_radius(capsule.radius);

		Vector3 y_dir = (vec2 - vec1).normalized();
		Vector3 x_dir;
		if (y_dir.is_equal_approx(Vector3(0, 1, 0))) {
			x_dir = y_dir.cross(Vector3(1, 0, 0)).normalized();
		} else {
			x_dir = y_dir.cross(Vector3(0, 1, 0)).normalized();
		}
		Vector3 z_dir = y_dir.cross(x_dir).normalized();
		Basis basis = Basis(x_dir, y_dir, z_dir);
		Vector3 center = (vec1 + vec2) / 2.0;

		if (basis.determinant() == 0.0) {
			continue;
		}

		animatable_body->set_transform(Transform3D(basis, center));
	}
}

void OpenXRFbHandTrackingCapsules::set_hand(Hand p_hand) {
	hand = p_hand;
}

OpenXRFbHandTrackingCapsules::Hand OpenXRFbHandTrackingCapsules::get_hand() const {
	return hand;
}

void OpenXRFbHandTrackingCapsules::set_sync_to_physics(bool p_enable) {
	sync_to_physics = p_enable;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_sync_to_physics(sync_to_physics);
	}
}

bool OpenXRFbHandTrackingCapsules::is_sync_to_physics_enabled() const {
	return sync_to_physics;
}

void OpenXRFbHandTrackingCapsules::set_physics_material_override(const Ref<PhysicsMaterial> &p_physics_material_override) {
	physics_material_override = p_physics_material_override;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_physics_material_override(physics_material_override);
	}
}

Ref<PhysicsMaterial> OpenXRFbHandTrackingCapsules::get_physics_material_override() const {
	return physics_material_override;
}

void OpenXRFbHandTrackingCapsules::set_constant_linear_velocity(const Vector3 &p_vel) {
	constant_linear_velocity = p_vel;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_constant_linear_velocity(constant_linear_velocity);
	}
}

Vector3 OpenXRFbHandTrackingCapsules::get_constant_linear_velocity() const {
	return constant_linear_velocity;
}

void OpenXRFbHandTrackingCapsules::set_constant_angular_velocity(const Vector3 &p_vel) {
	constant_angular_velocity = p_vel;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_constant_angular_velocity(constant_angular_velocity);
	}
}

Vector3 OpenXRFbHandTrackingCapsules::get_constant_angular_velocity() const {
	return constant_angular_velocity;
}

void OpenXRFbHandTrackingCapsules::set_disable_mode(CollisionObject3D::DisableMode p_mode) {
	disable_mode = p_mode;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_disable_mode(disable_mode);
	}
}

CollisionObject3D::DisableMode OpenXRFbHandTrackingCapsules::get_disable_mode() const {
	return disable_mode;
}

void OpenXRFbHandTrackingCapsules::set_collision_layer(uint32_t p_layer) {
	collision_layer = p_layer;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_collision_layer(collision_layer);
	}
}

uint32_t OpenXRFbHandTrackingCapsules::get_collision_layer() const {
	return collision_layer;
}

void OpenXRFbHandTrackingCapsules::set_collision_mask(uint32_t p_mask) {
	collision_mask = p_mask;

	if (!is_node_ready()) {
		return;
	}

	for (int i = 0; i < XR_HAND_TRACKING_CAPSULE_COUNT_FB; i++) {
		animatable_bodies[i]->set_collision_mask(collision_mask);
	}
}

uint32_t OpenXRFbHandTrackingCapsules::get_collision_mask() const {
	return collision_mask;
}

PackedStringArray OpenXRFbHandTrackingCapsules::_get_configuration_warnings() const {
	PackedStringArray warnings = Node::_get_configuration_warnings();

	XROrigin3D *origin = Object::cast_to<XROrigin3D>(get_parent());
	if (origin == nullptr) {
		warnings.push_back("OpenXRFbHandTrackingCapsules must have an XROrigin3D node as its parent.");
	}

	return warnings;
}

void OpenXRFbHandTrackingCapsules::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_hand", "hand"), &OpenXRFbHandTrackingCapsules::set_hand);
	ClassDB::bind_method(D_METHOD("get_hand"), &OpenXRFbHandTrackingCapsules::get_hand);
	ClassDB::bind_method(D_METHOD("set_sync_to_physics", "enable"), &OpenXRFbHandTrackingCapsules::set_sync_to_physics);
	ClassDB::bind_method(D_METHOD("is_sync_to_physics_enabled"), &OpenXRFbHandTrackingCapsules::is_sync_to_physics_enabled);
	ClassDB::bind_method(D_METHOD("set_physics_material_override", "physics_material_override"), &OpenXRFbHandTrackingCapsules::set_physics_material_override);
	ClassDB::bind_method(D_METHOD("get_physics_material_override"), &OpenXRFbHandTrackingCapsules::get_physics_material_override);
	ClassDB::bind_method(D_METHOD("set_constant_linear_velocity", "vel"), &OpenXRFbHandTrackingCapsules::set_constant_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_constant_linear_velocity"), &OpenXRFbHandTrackingCapsules::get_constant_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_constant_angular_velocity", "vel"), &OpenXRFbHandTrackingCapsules::set_constant_angular_velocity);
	ClassDB::bind_method(D_METHOD("get_constant_angular_velocity"), &OpenXRFbHandTrackingCapsules::get_constant_angular_velocity);
	ClassDB::bind_method(D_METHOD("set_disable_mode", "mode"), &OpenXRFbHandTrackingCapsules::set_disable_mode);
	ClassDB::bind_method(D_METHOD("get_disable_mode"), &OpenXRFbHandTrackingCapsules::get_disable_mode);
	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &OpenXRFbHandTrackingCapsules::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &OpenXRFbHandTrackingCapsules::get_collision_layer);
	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &OpenXRFbHandTrackingCapsules::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &OpenXRFbHandTrackingCapsules::get_collision_mask);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "hand", PROPERTY_HINT_ENUM, "Left,Right"), "set_hand", "get_hand");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_to_physics"), "set_sync_to_physics", "is_sync_to_physics_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "physics_material_override", PROPERTY_HINT_RESOURCE_TYPE, "PhysicsMaterial"), "set_physics_material_override", "get_physics_material_override");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "constant_linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_constant_linear_velocity", "get_constant_linear_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "constant_angular_velocity", PROPERTY_HINT_NONE, U"radians_as_degrees,suffix:\u00B0/s"), "set_constant_angular_velocity", "get_constant_angular_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "disable_mode", PROPERTY_HINT_ENUM, "Remove,Make Static,Keep Active"), "set_disable_mode", "get_disable_mode");

	ADD_GROUP("Collision", "collision_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");

	BIND_ENUM_CONSTANT(LEFT_HAND);
	BIND_ENUM_CONSTANT(RIGHT_HAND);
}
