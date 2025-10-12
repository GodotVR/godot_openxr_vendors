extends XRNode3D


func _process(_delta: float) -> void:
	var hand_tracker: XRHandTracker = XRServer.get_tracker(tracker)
	if hand_tracker and hand_tracker.has_tracking_data:
		show()
	else:
		hide()
