extends StartXR

@onready var viewport_2d_in_3d = %Viewport2Din3D
@onready var left_hand = %LeftHand
@onready var right_hand = %RightHand

enum DepthMode {
	DISABLED,
	SMOOTH,
	RAW,
	MAX,
}

var depth_mode: DepthMode = DepthMode.SMOOTH


func _ready():
	super._ready()

	OS.request_permissions()

	xr_interface.session_begun.connect(_update)


func _process(_delta: float) -> void:
	# Can't use "Show When Tracked" because we only want to show the hands in SMOOTH mode.
	left_hand.visible = (left_hand.get_has_tracking_data() and depth_mode == DepthMode.SMOOTH)
	right_hand.visible = (right_hand.get_has_tracking_data() and depth_mode == DepthMode.SMOOTH)


func _update() -> void:
	if depth_mode == DepthMode.DISABLED:
		if OpenXRAndroidEnvironmentDepthExtension.is_environment_depth_started():
			OpenXRAndroidEnvironmentDepthExtension.stop_environment_depth()
		viewport_2d_in_3d.get_scene_root().set_label_text("Depth disabled")
		return

	if not OpenXRAndroidEnvironmentDepthExtension.is_environment_depth_started():
		if not OpenXRAndroidEnvironmentDepthExtension.start_environment_depth():
			viewport_2d_in_3d.get_scene_root().set_label_text("Error starting depth")
			return

	OpenXRAndroidEnvironmentDepthExtension.set_smooth(depth_mode == DepthMode.SMOOTH)
	viewport_2d_in_3d.get_scene_root().set_label_text("Smooth" if depth_mode == DepthMode.SMOOTH else "Not smooth")


func _on_hand_pinch_detector_pinch_tapped() -> void:
	depth_mode += 1
	if depth_mode >= DepthMode.MAX:
		depth_mode = DepthMode.DISABLED
	_update()
