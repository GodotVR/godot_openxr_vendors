class_name XRMovement
extends Node3D

@export var action_name: String = "primary"

@export var movement_speed: Vector2 = Vector2(5.0, 5.0)


func _process(delta):
	var xr_controller: XRController3D = get_parent()
	if not xr_controller:
		return

	var xr_origin: XROrigin3D = xr_controller.get_parent()
	if not xr_origin:
		return

	var input: Vector2 = xr_controller.get_vector2("action_name")
	var movement: Vector3 = xr_origin.basis.x * input.x * delta * movement_speed.x
	movement += xr_origin.basis.z * input.y * delta * movement_speed.y
	xr_origin.position += movement
