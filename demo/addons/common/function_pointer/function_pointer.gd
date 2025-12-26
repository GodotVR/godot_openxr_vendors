extends Node3D

const Viewport2Din3D = preload("res://addons/common/viewport_2d_in_3d/viewport_2d_in_3d.gd")

@export var enabled := true:
	set = set_enabled

@export var select_action := "trigger_click"
@export var select_pressed_threshold := 0.9
@export var select_release_threshold := 0.6

@export var show_when_intersecting := true:
	set(v):
		show_when_intersecting = v
		_update_pointer_visibility()

@export var released_material: Material:
	set(v):
		released_material = v
		_update_pointer_material()

@export var pressed_material: Material:
	set(v):
		pressed_material = v
		_update_pointer_material()

@export var pointer_length := 5.0
@export var pointer_offset := 0.07

@onready var _mesh_instance: MeshInstance3D = %MeshInstance3D

var _controller: XRController3D
var _cur_layer: Viewport2Din3D = null
var _pressed := false


func set_enabled(p_enabled: bool) -> void:
	enabled = p_enabled
	if not enabled and _cur_layer:
		_cur_layer.pointer_leave(self)
		_cur_layer = null
	_update_pointer_visibility()
	_update_pointer_material()


func _ready() -> void:
	_update_pointer_visibility()
	_update_pointer_length(pointer_length)


func _enter_tree() -> void:
	var parent = get_parent()
	if parent is XRController3D:
		_controller = parent
		_controller.input_float_changed.connect(_on_controller_input_float_changed)
		_controller.button_pressed.connect(_on_controller_button_pressed)
		_controller.button_released.connect(_on_controller_button_released)
	else:
		_controller = null


func _exit_tree() -> void:
	if _controller:
		_controller.input_float_changed.disconnect(_on_controller_input_float_changed)
		_controller.button_pressed.disconnect(_on_controller_button_pressed)
		_controller.button_released.disconnect(_on_controller_button_released)


func _process(_delta: float) -> void:
	if enabled:
		for layer in _get_ui_layers():
			if not layer is Viewport2Din3D or not layer.visible:
				continue
			if layer.pointer_intersects(self):
				if layer != _cur_layer:
					if _cur_layer:
						_cur_layer.pointer_leave(self)
					_cur_layer = layer
					_update_pointer_visibility()
				break
			elif _cur_layer == layer:
				_cur_layer.pointer_leave(self)
				_cur_layer = null
				_update_pointer_visibility()
				_update_pointer_length(pointer_length)


func _get_ui_layers() -> Array:
	if not is_inside_tree():
		return []

	var nodes := get_tree().get_nodes_in_group("ui_layer")

	# Sort them closest to furthest.
	if nodes.size() > 1:
		var gp := global_position
		nodes.sort_custom(func(a, b): return gp.distance_squared_to(a.global_position) < gp.distance_squared_to(b.global_position))

	return nodes


func _update_pointer_visibility() -> void:
	if _mesh_instance:
		_mesh_instance.visible = _check_pointer_visibility()


func _check_pointer_visibility() -> bool:
	if not enabled:
		return false

	if show_when_intersecting and not _cur_layer:
		return false

	return true


func _update_pointer_length(p_length: float) -> void:
	if _mesh_instance:
		_mesh_instance.mesh.size.z = p_length
		_mesh_instance.position.z = -(p_length / 2.0) - pointer_offset


func _update_pointer_length_for_intersection(p_intersection: Vector3) -> void:
	var length := (p_intersection - global_position).length() - pointer_offset
	if length <= 0 or length > pointer_length:
		length = pointer_length
	_update_pointer_length(length)


func _update_pointer_material() -> void:
	if _mesh_instance:
		_mesh_instance.set_surface_override_material(0, pressed_material if _pressed else released_material)


func _on_controller_input_float_changed(p_name: String, p_value: float) -> void:
	if p_name == select_action:
		if not _pressed and p_value > select_pressed_threshold:
			_pressed = true
			_update_pointer_material()
			if _cur_layer:
				_cur_layer.pointer_set_pressed(self, true)
		elif _pressed and p_value < select_release_threshold:
			_pressed = false
			_update_pointer_material()
			if _cur_layer:
				_cur_layer.pointer_set_pressed(self, false)


func _do_select_press() -> void:
	_pressed = true
	_update_pointer_material()
	if _cur_layer:
		_cur_layer.pointer_set_pressed(self, true)


func _do_select_release() -> void:
	_pressed = false
	_update_pointer_material()
	if _cur_layer:
		_cur_layer.pointer_set_pressed(self, false)


func _on_controller_button_pressed(p_name: String) -> void:
	if p_name == select_action:
		_do_select_press()


func _on_controller_button_released(p_name: String) -> void:
	if p_name == select_action:
		_do_select_release()


func _get_hand_index() -> int:
	if _controller:
		match _controller.tracker:
			"left_hand":
				return 0
			"right_hand":
				return 1
	return -1
