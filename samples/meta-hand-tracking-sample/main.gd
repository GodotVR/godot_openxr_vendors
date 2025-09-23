extends StartXR

const CAPSULE_MATERIAL = preload("res://capsule_material.tres")

@onready var left_hand_ray_cast: RayCast3D = $XROrigin3D/LeftController/RayCast3D
@onready var left_hand_tracker: XRNode3D = $XROrigin3D/LeftHandTracker
@onready var left_hand_skeleton: OpenXRFbHandTrackingMesh = $XROrigin3D/LeftHandTracker/OpenXRFbHandTrackingMesh
@onready var right_hand_ray_cast: RayCast3D = $XROrigin3D/RightController/RayCast3D
@onready var right_hand_tracker: XRNode3D = $XROrigin3D/RightHandTracker
@onready var right_hand_skeleton: OpenXRFbHandTrackingMesh = $XROrigin3D/RightHandTracker/OpenXRFbHandTrackingMesh

@onready var left_index_strength: MeshInstance3D = $LeftHandInterface/LeftIndexStrength
@onready var left_middle_strength: MeshInstance3D = $LeftHandInterface/LeftMiddleStrength
@onready var left_ring_strength: MeshInstance3D = $LeftHandInterface/LeftRingStrength
@onready var left_little_strength: MeshInstance3D = $LeftHandInterface/LeftLittleStrength
@onready var left_menu_pressed: MeshInstance3D = $LeftHandInterface/DiscreteIndicators/MenuPressed
@onready var left_menu_gesture: MeshInstance3D = $LeftHandInterface/DiscreteIndicators/MenuGesture
@onready var left_index_pinch: MeshInstance3D = $LeftHandInterface/DiscreteIndicators/IndexPinch
@onready var left_middle_pinch: MeshInstance3D = $LeftHandInterface/DiscreteIndicators/MiddlePinch
@onready var left_ring_pinch: MeshInstance3D = $LeftHandInterface/DiscreteIndicators/RingPinch
@onready var left_little_pinch: MeshInstance3D = $LeftHandInterface/DiscreteIndicators/LittlePinch

@onready var right_index_strength: MeshInstance3D = $RightHandInterface/RightIndexStrength
@onready var right_middle_strength: MeshInstance3D = $RightHandInterface/RightMiddleStrength
@onready var right_ring_strength: MeshInstance3D = $RightHandInterface/RightRingStrength
@onready var right_little_strength: MeshInstance3D = $RightHandInterface/RightLittleStrength
@onready var right_system_gesture: MeshInstance3D = $RightHandInterface/DiscreteIndicators/SystemGesture
@onready var right_index_pinch: MeshInstance3D = $RightHandInterface/DiscreteIndicators/IndexPinch
@onready var right_middle_pinch: MeshInstance3D = $RightHandInterface/DiscreteIndicators/MiddlePinch
@onready var right_ring_pinch: MeshInstance3D = $RightHandInterface/DiscreteIndicators/RingPinch
@onready var right_little_pinch: MeshInstance3D = $RightHandInterface/DiscreteIndicators/LittlePinch

var fb_capsule_ext
var countdown_to_group_hand_meshes := 3
var left_capsules_loaded := false
var right_capsules_loaded := false


func _ready() -> void:
	super._ready()
	if xr_interface and xr_interface.is_initialized():
		fb_capsule_ext = Engine.get_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper")


func _process(delta):
	if countdown_to_group_hand_meshes > 0:
		countdown_to_group_hand_meshes -= 1
		if countdown_to_group_hand_meshes == 0:
			for child in right_hand_skeleton.get_children():
				if child is MeshInstance3D:
					child.add_to_group("hand_mesh_right")
			for child in left_hand_skeleton.get_children():
				if child is MeshInstance3D:
					child.add_to_group("hand_mesh_left")

	if not left_capsules_loaded:
		var tracker: XRHandTracker = XRServer.get_tracker("/user/hand_tracker/left")
		if tracker and tracker.has_tracking_data:
			hand_capsule_setup(0, tracker)
	if not right_capsules_loaded:
		var tracker: XRHandTracker = XRServer.get_tracker("/user/hand_tracker/right")
		if tracker and tracker.has_tracking_data:
			hand_capsule_setup(1, tracker)


func hand_capsule_setup(hand_idx: int, hand_tracker: XRHandTracker) -> void:
	var skeletons := [left_hand_skeleton, right_hand_skeleton]

	for capsule_idx in fb_capsule_ext.get_hand_capsule_count():
		var capsule_mesh := CapsuleMesh.new()
		capsule_mesh.height = fb_capsule_ext.get_hand_capsule_height(hand_idx, capsule_idx)
		capsule_mesh.radius = fb_capsule_ext.get_hand_capsule_radius(hand_idx, capsule_idx)

		var mesh_instance := MeshInstance3D.new()
		mesh_instance.mesh = capsule_mesh
		mesh_instance.set_surface_override_material(0, CAPSULE_MATERIAL)
		match hand_idx:
			0:
				mesh_instance.add_to_group("hand_capsule_left")
			1:
				mesh_instance.add_to_group("hand_capsule_right")

		var bone_attachment := BoneAttachment3D.new()
		bone_attachment.bone_idx = fb_capsule_ext.get_hand_capsule_joint(hand_idx, capsule_idx)

		bone_attachment.add_child(mesh_instance)
		skeletons[hand_idx].add_child(bone_attachment)

		var capsule_transform: Transform3D = fb_capsule_ext.get_hand_capsule_transform(hand_idx, capsule_idx)
		var bone_transform: Transform3D = hand_tracker.get_hand_joint_transform(fb_capsule_ext.get_hand_capsule_joint(hand_idx, capsule_idx))
		mesh_instance.transform = bone_transform.inverse() * capsule_transform

	match hand_idx:
		0:
			left_capsules_loaded = true
		1:
			right_capsules_loaded = true


func _on_left_controller_input_float_changed(name: String, value: float) -> void:
	match name:
		"index_pinch_strength":
			left_index_strength.get_surface_override_material(0).set_shader_parameter("value", value)
		"middle_pinch_strength":
			left_middle_strength.get_surface_override_material(0).set_shader_parameter("value", value)
		"ring_pinch_strength":
			left_ring_strength.get_surface_override_material(0).set_shader_parameter("value", value)
		"little_pinch_strength":
			left_little_strength.get_surface_override_material(0).set_shader_parameter("value", value)


func _on_right_controller_input_float_changed(name: String, value: float) -> void:
	match name:
		"index_pinch_strength":
			right_index_strength.get_surface_override_material(0).set_shader_parameter("value", value)
		"middle_pinch_strength":
			right_middle_strength.get_surface_override_material(0).set_shader_parameter("value", value)
		"ring_pinch_strength":
			right_ring_strength.get_surface_override_material(0).set_shader_parameter("value", value)
		"little_pinch_strength":
			right_little_strength.get_surface_override_material(0).set_shader_parameter("value", value)


func _on_left_controller_button_pressed(name: String) -> void:
	match name:
		"menu_pressed":
			left_menu_pressed.get_surface_override_material(0).albedo_color = Color.GREEN
		"menu_gesture":
			left_menu_gesture.get_surface_override_material(0).albedo_color = Color.GREEN
		"index_pinch":
			left_hand_ray_cast.is_active = true
			if left_hand_ray_cast.is_colliding():
				var collider = left_hand_ray_cast.get_collider()
				update(collider.name)
			left_index_pinch.get_surface_override_material(0).albedo_color = Color.GREEN
		"middle_pinch":
			left_middle_pinch.get_surface_override_material(0).albedo_color = Color.GREEN
		"ring_pinch":
			left_ring_pinch.get_surface_override_material(0).albedo_color = Color.GREEN
		"little_pinch":
			left_little_pinch.get_surface_override_material(0).albedo_color = Color.GREEN


func _on_left_controller_button_released(name: String) -> void:
	match name:
		"menu_pressed":
			left_menu_pressed.get_surface_override_material(0).albedo_color = Color.RED
		"menu_gesture":
			left_menu_gesture.get_surface_override_material(0).albedo_color = Color.RED
		"index_pinch":
			left_hand_ray_cast.is_active = false
			left_index_pinch.get_surface_override_material(0).albedo_color = Color.RED
		"middle_pinch":
			left_middle_pinch.get_surface_override_material(0).albedo_color = Color.RED
		"ring_pinch":
			left_ring_pinch.get_surface_override_material(0).albedo_color = Color.RED
		"little_pinch":
			left_little_pinch.get_surface_override_material(0).albedo_color = Color.RED


func _on_right_controller_button_pressed(name: String) -> void:
	match name:
		"system_gesture":
			right_system_gesture.get_surface_override_material(0).albedo_color = Color.GREEN
		"index_pinch":
			right_hand_ray_cast.is_active = true
			if right_hand_ray_cast.is_colliding():
				var collider = right_hand_ray_cast.get_collider()
				update(collider.name)
			right_index_pinch.get_surface_override_material(0).albedo_color = Color.GREEN
		"middle_pinch":
			right_middle_pinch.get_surface_override_material(0).albedo_color = Color.GREEN
		"ring_pinch":
			right_ring_pinch.get_surface_override_material(0).albedo_color = Color.GREEN
		"little_pinch":
			right_little_pinch.get_surface_override_material(0).albedo_color = Color.GREEN


func _on_right_controller_button_released(name: String) -> void:
	match name:
		"system_gesture":
			right_system_gesture.get_surface_override_material(0).albedo_color = Color.RED
		"index_pinch":
			right_hand_ray_cast.is_active = false
			right_index_pinch.get_surface_override_material(0).albedo_color = Color.RED
		"middle_pinch":
			right_middle_pinch.get_surface_override_material(0).albedo_color = Color.RED
		"ring_pinch":
			right_ring_pinch.get_surface_override_material(0).albedo_color = Color.RED
		"little_pinch":
			right_little_pinch.get_surface_override_material(0).albedo_color = Color.RED


func update(collider_name: String) -> void:
	match collider_name:
		"LeftHandMesh":
			for hand_mesh in get_tree().get_nodes_in_group("hand_mesh_left"):
				hand_mesh.visible = not hand_mesh.visible
		"LeftHandCapsules":
			for hand_capsule in get_tree().get_nodes_in_group("hand_capsule_left"):
				hand_capsule.visible = not hand_capsule.visible
		"RightHandMesh":
			for hand_mesh in get_tree().get_nodes_in_group("hand_mesh_right"):
				hand_mesh.visible = not hand_mesh.visible
		"RightHandCapsules":
			for hand_capsule in get_tree().get_nodes_in_group("hand_capsule_right"):
				hand_capsule.visible = not hand_capsule.visible
