@tool
extends Node3D

## Color of this vector
@export var color: Color = Color(1.0, 1.0, 1.0, 1.0):
	set(value):
		color = value
		if is_inside_tree():
			_on_color_changed()

## If true we ignore depth and always draw
@export var no_depth_test: bool = false:
	set(value):
		no_depth_test = value
		if is_inside_tree():
			_update_ignore_depth()


func _on_color_changed():
	var material: StandardMaterial3D = $Stem.material_override
	material.albedo_color = color


func _update_ignore_depth():
	var material: StandardMaterial3D = $Stem.material_override
	material.no_depth_test = no_depth_test


func _ready():
	_on_color_changed()
	_update_ignore_depth()
