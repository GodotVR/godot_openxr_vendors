extends OpenXRFbPassthroughGeometry


func _process(delta: float) -> void:
	position.x = 3 * sin(Time.get_ticks_msec() / 1000.0)
