extends XRController3D

@onready var _raycast: RayCast3D = $RayCast3D
@onready var _collision_point: Node3D = $CollisionPoint


func _process(_delta: float) -> void:
	if _raycast.is_colliding() && _raycast.get_collider():
		_collision_point.visible = true
		_collision_point.global_position = _raycast.get_collision_point()
		_collision_point.global_transform.basis = _raycast.get_collider().global_transform.basis
	else:
		_collision_point.visible = false
