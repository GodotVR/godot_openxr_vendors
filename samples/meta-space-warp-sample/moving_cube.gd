extends MeshInstance3D

func _process(delta: float) -> void:
	position.z = sin(Time.get_ticks_msec() / 500.0)
