@tool
extends Node3D

## Text of the label we show
@export_multiline var label: String = "XRNode3D":
	set(value):
		label = value
		if is_inside_tree():
			_update_label()

## If true we ignore depth and always draw
@export var no_depth_test: bool = false:
	set(value):
		no_depth_test = value
		if is_inside_tree():
			_update_ignore_depth()


func _update_label():
	$Label3D.text = label


func _update_ignore_depth():
	$Label3D.no_depth_test = no_depth_test
	$Forward.no_depth_test = no_depth_test
	$Forward/Z.no_depth_test = no_depth_test
	$Up.no_depth_test = no_depth_test
	$Up/Y.no_depth_test = no_depth_test
	$Left.no_depth_test = no_depth_test
	$Left/X.no_depth_test = no_depth_test


# Called when the node enters the scene tree for the first time.
func _ready():
	_update_label()
	_update_ignore_depth()
