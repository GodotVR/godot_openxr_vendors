extends XRController3D

@onready var hand_cube: MeshInstance3D = $HandCube
@onready var render_model: OpenXRFbRenderModel = $ControllerFbRenderModel

func _process(_delta: float) -> void:
	var hand_tracker_name = "/user/hand_tracker/left" if tracker == "left_hand" \
			else "/user/hand_tracker/right"

	var hand_tracker: XRHandTracker = XRServer.get_tracker(hand_tracker_name)
	if hand_tracker and hand_tracker.has_tracking_data:
		if hand_tracker.hand_tracking_source == XRHandTracker.HAND_TRACKING_SOURCE_CONTROLLER \
				and render_model.has_render_model_node():
			render_model.show()
		elif render_model.has_render_model_node():
			render_model.hide()
		else:
			hand_cube.hide()
	else:
		if render_model.has_render_model_node():
			render_model.show()
		else:
			hand_cube.show()
