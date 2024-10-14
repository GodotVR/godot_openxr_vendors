extends MultiMeshInstance3D

func _process(delta: float) -> void:
	var origin := Vector3(sin(Time.get_ticks_msec() / 500.0), 0, 0)
	var new_transform := Transform3D(basis, origin)
	multimesh.set_instance_transform(0, new_transform)
