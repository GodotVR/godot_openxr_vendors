@tool
extends Node3D

## A 3D node to render a 2D Control scene.
##
## This node is currently intended to be a more flexible version of [Label3D], since it essentially
## enables Godot's full [Control] node ui capability for 3D space.

const NO_INTERSECTION = Vector2(-1.0, -1.0)
const CURSOR_DISTANCE = 0.005
const DOUBLE_CLICK_TIME = 400
const DOUBLE_CLICK_DIST = 5.0

## The current [PackedScene] rendered by the quad.[br]
## The [PackedScene]'s root node must be a [Control] node.[br]
## Can be [code]null[/code].
@export var scene: PackedScene:
	set = set_scene

## The viewport size in pixels.
@export var viewport_size := Vector2i(1152, 648):
	set = set_viewport_size

## 1 pixel in 2D space is this size, in meters, in 3D space.[br]
## The default chosen here results in the best text rendering (though the font will need to be
## oversampled)[br]
## Example:[br]
## with a [code]viewport_size[/code] of 200 x 300 pixels when [param pixel_size] is [code]0.0001[/code],
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

var _pointer: Node3D
var _pointer_pressed := false
var _prev_intersection: Vector2 = NO_INTERSECTION
var _prev_pressed_pos: Vector2
var _prev_pressed_time: int = 0

@onready var _quad: MeshInstance3D = $Quad
@onready var _cursor: MeshInstance3D = $Cursor
@onready var _viewport: SubViewport = $SubViewport


func _ready():
	# @onready variables are not set while the setters are called, the order the setters are called
	# may be unexpected, and setters may not have been called (which happens for values not set in the
	# inspector)
	_try_to_add_scene_root_to_viewport()
	_update_sizes()
	set_render_priority(render_priority)
	set_no_depth_test(no_depth_test)
	set_modulate_color(modulate_color)


func _update_sizes() -> void:
	if _viewport and _viewport.size != viewport_size:
		_viewport.size = viewport_size
	if _quad:
		_quad.mesh.size = viewport_size * pixel_size


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
		_scene_root.queue_free()
		_scene_root = null

	scene = new_scene
	_scene_root = new_scene_root

	_try_to_add_scene_root_to_viewport()


func set_viewport_size(new_viewport_size: Vector2i) -> void:
	viewport_size = new_viewport_size
	_update_sizes()


func set_pixel_size(new_pixel_size: float):
	if new_pixel_size <= 0.0:
		printerr("pixel size must be >= 0")
		return

	pixel_size = new_pixel_size
	_update_sizes()


func set_render_priority(new_render_priority: int):
	render_priority = new_render_priority
	if not _quad:
		return

	_quad.mesh.material.render_priority = render_priority


func set_no_depth_test(new_no_depth_test: bool):
	no_depth_test = new_no_depth_test
	if not _quad:
		return

	_quad.mesh.material.no_depth_test = no_depth_test


func set_modulate_color(new_modulate_color: Color):
	modulate_color = new_modulate_color
	if not _quad:
		return

	_quad.mesh.material.albedo_color = modulate_color


## Get the root node of the 2D scene rendered in the 3D quad.[br]
## Can be [code]null[/code].
func get_scene_root() -> Control:
	return _scene_root


## Get the size of the 3D Quad, in meters, where [member Vector2.x]
## is the width and [member Vector2.y] is the height
func get_size() -> Vector2:
	if _quad:
		return _quad.mesh.size
	return Vector2.ZERO


func _try_to_add_scene_root_to_viewport():
	if !is_inside_tree() || _scene_root == null || _scene_root.get_parent() == _viewport:
		return

	_viewport.add_child(_scene_root)


func _intersect_to_global_pos(p_intersection: Vector2, p_distance: float = 0.0) -> Vector3:
	if p_intersection != NO_INTERSECTION:
		var local_pos: Vector2 = (p_intersection - Vector2(0.5, 0.5)) * get_size()
		return global_transform * Vector3(local_pos.x, -local_pos.y, p_distance)

	return Vector3()


func _intersect_to_viewport_pos(p_intersection: Vector2) -> Vector2:
	if _viewport and p_intersection != NO_INTERSECTION:
		var pos: Vector2 = p_intersection * Vector2(_viewport.size)
		return Vector2(pos)

	return NO_INTERSECTION


## Returns a normalized point (UV) in 2D space.
func intersects_ray(p_origin: Vector3, p_direction: Vector3) -> Vector2:
	if not visible:
		return NO_INTERSECTION

	var quad_transform: Transform3D = get_global_transform()
	var quad_normal: Vector3 = quad_transform.basis.z

	var denom: float = quad_normal.dot(p_direction)
	if denom < -0.0001:
		var vector: Vector3 = quad_transform.origin - p_origin
		var t: float = vector.dot(quad_normal) / denom
		if t < 0.0:
			return NO_INTERSECTION

		var intersection: Vector3 = p_origin + p_direction * t

		var relative_point: Vector3 = intersection - quad_transform.origin
		var projected_point := Vector2(relative_point.dot(quad_transform.basis.x), relative_point.dot(quad_transform.basis.y))

		var quad_size := get_size()
		if absf(projected_point.x) > quad_size.x / 2.0:
			return NO_INTERSECTION
		if absf(projected_point.y) > quad_size.y / 2.0:
			return NO_INTERSECTION

		var u: float = 0.5 + (projected_point.x / quad_size.x)
		var v: float = 1.0 - (0.5 + (projected_point.y / quad_size.y))

		return Vector2(u, v)

	return NO_INTERSECTION


func pointer_intersects(p_pointer: Node3D) -> bool:
	var pt := p_pointer.global_transform
	var intersection := intersects_ray(pt.origin, -pt.basis.z)
	if intersection != NO_INTERSECTION:
		# If there was no current pointer, let's take this one.
		if _pointer == null:
			_pointer = p_pointer
			_cursor.visible = true

		var cursor_position = _intersect_to_global_pos(intersection, CURSOR_DISTANCE)
		if p_pointer.has_method("_update_pointer_length_for_intersection"):
			p_pointer._update_pointer_length_for_intersection(cursor_position)

		# If this pointer is the current pointer, then the cursor and mouse events move with it.
		if p_pointer == _pointer:
			_cursor.global_position = cursor_position

			if _viewport and _prev_intersection:
				var event := InputEventMouseMotion.new()
				var from := _intersect_to_viewport_pos(_prev_intersection)
				var to := _intersect_to_viewport_pos(intersection)
				if _pointer_pressed:
					event.button_mask = MOUSE_BUTTON_MASK_LEFT
				event.relative = to - from
				event.position = to
				_viewport.push_input(event)

			_prev_intersection = intersection

		return true

	# If this pointer is the current pointer, but there was no intersection, then that pointer
	# should leave.
	if p_pointer == _pointer:
		# Except if it's pressed - we'll hang on to it until it's released.
		if _pointer_pressed:
			return true
		pointer_leave(p_pointer)

	return false


func _send_mouse_button_event(p_pressed: bool) -> void:
	if not _viewport:
		return

	var event := InputEventMouseButton.new()
	event.button_index = MOUSE_BUTTON_LEFT
	event.button_mask = MOUSE_BUTTON_MASK_LEFT
	event.pressed = p_pressed
	event.position = _intersect_to_viewport_pos(_prev_intersection)

	if p_pressed:
		var time := Time.get_ticks_msec()
		if time - _prev_pressed_time < DOUBLE_CLICK_TIME and (event.position - _prev_pressed_pos).length() < DOUBLE_CLICK_DIST:
			event.double_click = true

		_prev_pressed_time = time
		_prev_pressed_pos = event.position

	_viewport.push_input(event)


func pointer_leave(p_pointer: Node3D) -> void:
	# We only need to do anything, if the pointer leaving is the current pointer.
	if _pointer == p_pointer:
		# If the pointer was pressed, then send the mouse event to release the button.
		if _pointer_pressed and _prev_intersection != NO_INTERSECTION:
			_send_mouse_button_event(false)

		# And clear everything out.
		_pointer = null
		_pointer_pressed = false
		_cursor.visible = false
		_prev_intersection = NO_INTERSECTION
		_prev_pressed_time = 0


func pointer_set_pressed(p_pointer: Node3D, p_pressed: bool) -> void:
	if p_pointer == _pointer:
		# If this is the current pointer, then update our pressed state and send the mouse
		# events, if this is a change in state.
		if p_pressed != _pointer_pressed:
			_pointer_pressed = p_pressed
			_send_mouse_button_event(p_pressed)
	elif p_pressed:
		# If another pointer presses, then allow it to take over.
		if _pointer:
			# The current pointer leaves (this will send the mouse up event).
			pointer_leave(_pointer)
		if pointer_intersects(p_pointer):
			_pointer_pressed = true
			_prev_pressed_time = 0
			_send_mouse_button_event(true)
