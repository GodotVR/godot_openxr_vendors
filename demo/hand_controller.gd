extends XRController3D

@onready var hand_cube: MeshInstance3D = $HandCube
@onready var render_model: OpenXRFbRenderModel = $ControllerFbRenderModel


func _process(_delta: float) -> void:
	var hand_tracker_name = "/user/hand_tracker/left" if tracker == "left_hand" else "/user/hand_tracker/right"
	var hand_tracker_hand: OpenXRInterface.Hand = OpenXRInterface.HAND_LEFT if tracker == "left_hand" else OpenXRInterface.HAND_RIGHT

	var interface: OpenXRInterface = XRServer.find_interface("OpenXR")

	var hand_tracker: XRHandTracker = XRServer.get_tracker(hand_tracker_name)
	if hand_tracker and hand_tracker.has_tracking_data:
		# If hand tracking data is provided by controller,
		# and controller model is available, show it with hand mesh.
		if hand_tracker.hand_tracking_source == XRHandTracker.HAND_TRACKING_SOURCE_CONTROLLER and render_model.has_render_model_node():
			render_model.show()
			interface.set_motion_range(hand_tracker_hand, OpenXRInterface.HAND_MOTION_RANGE_CONFORM_TO_CONTROLLER)
		# If hand tracking data is not provided by controller,
		# and controller model is available, hide it.
		elif render_model.has_render_model_node():
			render_model.hide()
			interface.set_motion_range(hand_tracker_hand, OpenXRInterface.HAND_MOTION_RANGE_UNOBSTRUCTED)

		# Always hide placeholder hand cubes if we have hand tracking data.
		hand_cube.hide()
	else:
		# Show render model if availabe, otherwise show hand cubes.
		if render_model.has_render_model_node():
			render_model.show()
			hand_cube.hide()
		else:
			hand_cube.show()
