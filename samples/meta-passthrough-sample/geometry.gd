extends OpenXRFbPassthroughGeometry


func _process(delta: float) -> void:
	if not "--xrsim-automated-tests" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, we don't want this
		# to be animated, which can lead to differences in the screenshots.
		position.x = 3 * sin(Time.get_ticks_msec() / 1000.0)
