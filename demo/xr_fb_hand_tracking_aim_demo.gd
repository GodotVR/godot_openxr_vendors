extends Node3D

@onready var little_left: MeshInstance3D = $LittleLeft
@onready var ring_left: MeshInstance3D = $RingLeft
@onready var middle_left: MeshInstance3D = $MiddleLeft
@onready var index_left: MeshInstance3D = $IndexLeft
@onready var little_right: MeshInstance3D = $LittleRight
@onready var ring_right: MeshInstance3D = $RingRight
@onready var middle_right: MeshInstance3D = $MiddleRight
@onready var index_right: MeshInstance3D = $IndexRight
@onready var system_gesture_left: MeshInstance3D = $SystemGestureLeft
@onready var system_gesture_right: MeshInstance3D = $SystemGestureRight

func _on_right_hand_button_pressed(name: String) -> void:
	match name:
		"index_pinch":
			index_right.material_override.albedo_color = Color.GREEN
		"middle_pinch":
			middle_right.material_override.albedo_color = Color.GREEN
		"ring_pinch":
			ring_right.material_override.albedo_color = Color.GREEN
		"little_pinch":
			little_right.material_override.albedo_color = Color.GREEN
		"system_gesture":
			system_gesture_right.material_override.albedo_color = Color.GREEN


func _on_right_hand_button_released(name: String) -> void:
	match name:
		"index_pinch":
			index_right.material_override.albedo_color = Color.RED
		"middle_pinch":
			middle_right.material_override.albedo_color = Color.RED
		"ring_pinch":
			ring_right.material_override.albedo_color = Color.RED
		"little_pinch":
			little_right.material_override.albedo_color = Color.RED
		"system_gesture":
			system_gesture_right.material_override.albedo_color = Color.RED


func _on_left_hand_button_pressed(name: String) -> void:
	match name:
		"index_pinch":
			index_left.material_override.albedo_color = Color.GREEN
		"middle_pinch":
			middle_left.material_override.albedo_color = Color.GREEN
		"ring_pinch":
			ring_left.material_override.albedo_color = Color.GREEN
		"little_pinch":
			little_left.material_override.albedo_color = Color.GREEN
		"system_gesture":
			system_gesture_left.material_override.albedo_color = Color.GREEN


func _on_left_hand_button_released(name: String) -> void:
	match name:
		"index_pinch":
			index_left.material_override.albedo_color = Color.RED
		"middle_pinch":
			middle_left.material_override.albedo_color = Color.RED
		"ring_pinch":
			ring_left.material_override.albedo_color = Color.RED
		"little_pinch":
			little_left.material_override.albedo_color = Color.RED
		"system_gesture":
			system_gesture_left.material_override.albedo_color = Color.RED
