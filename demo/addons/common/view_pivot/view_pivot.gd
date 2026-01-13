extends Node3D

## The minimum threshold that the dot product between the camera direction
## and desired direction must fall below before the pivot is rotated.
## Higher values will rotate more often, and lower values will be looser.
@export var direction_threshold := 0.9

## The minimum threshold that the distance between the camera position and
## desired position must go above, before the pivot is moved.
## Lower values will move more often, and higher values will be looser.
@export var distance_threshold := 0.5

## The delay (in seconds) after the camera has moved past one of the thresholds
## before the pivot will move.
@export var movement_delay := 1.0:
	set = set_movement_delay

@onready var _timer: Timer = %Timer

var _current_direction: Vector3 = Vector3.FORWARD
var _current_position: Vector3
var _pivot_tween: Tween


func set_movement_delay(p_delay: float) -> void:
	movement_delay = p_delay
	if _timer != null:
		_timer.wait_time = movement_delay


func _ready() -> void:
	_timer.wait_time = movement_delay
	update_transform_immediately()
	set_process(is_visible_in_tree())


func _get_camera() -> XRCamera3D:
	var vp: Viewport = get_viewport()
	if vp == null:
		return null

	var camera: Camera3D = vp.get_camera_3d()
	if camera == null or not camera is XRCamera3D:
		return null

	return camera as XRCamera3D


func _get_camera_direction() -> Vector3:
	var camera: XRCamera3D = _get_camera()
	if camera == null:
		return _current_direction

	var v: Vector3 = -camera.global_transform.basis.z

	# If the player is looking straight up or down, use the current direction.
	if is_equal_approx(abs(v.y), 1.0):
		return _current_direction

	v.y = 0
	v = v.normalized()

	return v


func _get_camera_position() -> Vector3:
	var camera: XRCamera3D = _get_camera()
	if camera == null:
		return _current_direction

	return camera.global_position


func update_transform_immediately() -> void:
	if _pivot_tween:
		_pivot_tween.kill()
		_pivot_tween = null

	_current_direction = _get_camera_direction()
	_current_position = _get_camera_position()
	global_transform.origin = Vector3(_current_position.x, global_transform.origin.y, _current_position.z)
	global_transform.basis = Basis.looking_at(_current_direction)


func _process(_delta: float) -> void:
	if visible:
		var camera_direction := _get_camera_direction()
		var camera_distance := (_current_position - _get_camera_position()).length()
		if _current_direction.dot(camera_direction) < direction_threshold or camera_distance > distance_threshold:
			if _timer.is_stopped():
				_timer.start()
		else:
			_timer.stop()


func _notification(p_what: int) -> void:
	match p_what:
		NOTIFICATION_VISIBILITY_CHANGED:
			if is_visible_in_tree():
				update_transform_immediately()
				set_process(true)
			else:
				set_process(false)


func _on_timer_timeout() -> void:
	_current_position = _get_camera_position()
	_current_direction = _get_camera_direction()
	var new_basis = Basis.looking_at(_current_direction)

	if _pivot_tween:
		_pivot_tween.kill()
		_pivot_tween = null

	var gt := global_transform

	_pivot_tween = get_tree().create_tween()
	_pivot_tween.tween_method(self._tween_global_quaternion, gt.basis.get_rotation_quaternion(), new_basis.get_rotation_quaternion(), movement_delay).set_trans(Tween.TRANS_EXPO).set_ease(Tween.EASE_OUT)
	_pivot_tween.parallel().tween_property(self, "global_position", Vector3(_current_position.x, gt.origin.y, _current_position.z), movement_delay).set_trans(Tween.TRANS_EXPO).set_ease(Tween.EASE_OUT)


func _tween_global_quaternion(p_rotation: Quaternion) -> void:
	global_transform.basis = Basis(p_rotation)
