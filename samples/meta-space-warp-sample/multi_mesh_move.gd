extends MultiMeshInstance3D

@export var direction: Vector3 = Vector3(1, 0, 0)


func _process(delta: float) -> void:
	if not "--xrsim-automated-tests" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, we don't want this
		# to be animated, which can lead to differences in the screenshots.
		var displacement = sin(Time.get_ticks_msec() / 300.0)
		var origin := Vector3(displacement, displacement, displacement)
		origin = origin * direction
		var new_transform := Transform3D(basis, origin)
		multimesh.set_instance_transform(0, new_transform)
