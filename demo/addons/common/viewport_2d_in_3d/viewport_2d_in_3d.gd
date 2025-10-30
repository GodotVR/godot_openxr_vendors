@tool
extends Node3D

## A 3D node to render a 2D Control scene.
##
## This node is currently intended to be a more flexible version of [Label3D], since it essentially
## enables Godot's full [Control] node ui capability for 3D space.
##
## Note that input is ignored (and a lot of work is required if we want input)

## Signal called when the underlying quad mesh's size changes.[br]
## Call [method get_size] to get the new size.
signal resized

## The current [PackedScene] rendered by the quad.[br]
## The [PackedScene]'s root node must be a [Control] node.[br]
## Can be [code]null[/code].
@export var scene: PackedScene:
	set = set_scene

## 1 pixel in 2D space is this size, in meters, in 3D space.[br]
## The default chosen here results in the best text rendering (though the font will need to be
## oversampled)[br]
## Example:[br]
## a [code]200x300pixel[/code] scene in 2D, when [param pixel_size] is [code]0.0001[/code],
## is 0.02x0.03meters in 3D space.
@export var pixel_size := 0.0001:
	set = set_pixel_size

## The underlying material's [member Material.render_priority]
@export var render_priority := 0:
	set = set_render_priority

## The color to set to the underlying material's [member BaseMaterial3D.no_depth_test]
@export var no_depth_test := false:
	set = set_no_depth_test

## The color to set to the underlying material's [member BaseMaterial3D.albedo_color]
@export var modulate_color := Color.WHITE:
	set = set_modulate_color

var _scene_root: Control

@onready var _viewport: SubViewport = $SubViewport


func _ready():
	# @onready variables are not set while the setters are called, the order the setters are called
	# may be unexpected, and setters may not have been called (which happens for values not set in the
	# inspector)
	_try_to_add_scene_root_to_viewport()
	set_pixel_size(pixel_size)
	set_render_priority(render_priority)
	set_no_depth_test(no_depth_test)
	set_modulate_color(modulate_color)


## Set a new 2D scene resource as the scene to render.[br]
## The new scene's root node must be a [Control] node.
func set_scene(new_scene: PackedScene):
	var new_scene_root
	if new_scene != null:
		new_scene_root = new_scene.instantiate()
		if !(new_scene_root is Control):
			printerr("Scene root must be a Control node!")
			return

	if _scene_root != null:
		_scene_root.resized.disconnect(set_pixel_size.bind(pixel_size))
		_scene_root.queue_free()
		_scene_root = null

	scene = new_scene
	_scene_root = new_scene_root

	_try_to_add_scene_root_to_viewport()
	set_pixel_size(pixel_size)


func set_pixel_size(new_pixel_size: float):
	if new_pixel_size <= 0.0:
		printerr("pixel size must be >= 0")
		return

	# always set the pixel size, and always attempt to set the size, since maybe
	# _scene_root wasn't ready the first time set_pixel_size() was called during initialization
	pixel_size = new_pixel_size

	if is_inside_tree() && _scene_root != null:
		$MeshInstance3D.mesh.size = _scene_root.size * pixel_size
		resized.emit()


func set_render_priority(new_render_priority: int):
	render_priority = new_render_priority
	if !is_inside_tree():
		return

	$MeshInstance3D.mesh.material.render_priority = render_priority


func set_no_depth_test(new_no_depth_test: bool):
	no_depth_test = new_no_depth_test
	if !is_inside_tree():
		return

	$MeshInstance3D.mesh.material.no_depth_test = no_depth_test


func set_modulate_color(new_modulate_color: Color):
	modulate_color = new_modulate_color
	if !is_inside_tree():
		return

	$MeshInstance3D.mesh.material.albedo_color = modulate_color


## Get the root node of the 2D scene rendered in the 3D quad.[br]
## Can be [code]null[/code].
func get_scene_root() -> Control:
	return _scene_root


## Get the size of the 3D Quad, in meters, where [member Vector2.x]
## is the width and [member Vector2.y] is the height
func get_size() -> Vector2:
	return $MeshInstance3D.mesh.size if is_inside_tree() && _scene_root != null else Vector2.ZERO


func _try_to_add_scene_root_to_viewport():
	if !is_inside_tree() || _scene_root == null || _scene_root.get_parent() == _viewport:
		return

	_viewport.add_child(_scene_root)
	_viewport.size = _scene_root.size

	# resize the mesh any time the root node resizes (so that the mesh grows/shrinks as needed)
	_scene_root.resized.connect(_on_scene_root_resized)


func _on_scene_root_resized():
	# _scene_root should be non-null since _on_scene_root_resized is only called when we receive a
	# signal from it
	if _scene_root == null:
		printerr("Unexpected; how did we get a signal from _scene_root when it is null?")
		return

	_viewport.size = _scene_root.size
	set_pixel_size(pixel_size)
