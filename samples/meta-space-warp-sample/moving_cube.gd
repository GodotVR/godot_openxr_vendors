extends MeshInstance3D


func _process(delta: float) -> void:
	if not "--xrsim-automated-tests" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, we don't want this
		# to be animated, which can lead to differences in the screenshots.
		position.z = sin(Time.get_ticks_msec() / 300.0)
		position.y = cos(Time.get_ticks_msec() / 300.0)
