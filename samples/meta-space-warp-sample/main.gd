extends StartXR

const SNAP_TURN_THRESHOLD := 0.8
const SNAP_TURN_ANGLE := TAU / 16
const SMOOTH_TURN_SPEED := 0.05
const MOVE_SPEED := 2.0

var movement_input := Vector2.ZERO
var smooth_turn_input := 0.0
var snap_turn_enabled := false
var countdown_to_check_space_warp_enabled: int = 3

@onready var xr_origin: XROrigin3D = $XROrigin3D
@onready var xr_camera: XRCamera3D = $XROrigin3D/XRCamera3D
@onready var right_controller_label: Label3D = $XROrigin3D/RightHand/Label3D
@onready var turn_timer: Timer = $XROrigin3D/TurnTimer

func _process(delta: float) -> void:
	if countdown_to_check_space_warp_enabled > 0:
		countdown_to_check_space_warp_enabled -= 1
		if countdown_to_check_space_warp_enabled == 0:
			var fb_space_warp = Engine.get_singleton("OpenXRFbSpaceWarpExtensionWrapper")
			if not fb_space_warp.is_enabled():
				right_controller_label.text = right_controller_label.text.replace("ENABLED", "DISABLED")


func _physics_process(delta: float) -> void:
	if movement_input != Vector2.ZERO:
		xr_origin.position.z += -movement_input.y * delta * MOVE_SPEED
		xr_origin.position.x += movement_input.x * delta * MOVE_SPEED
	if smooth_turn_input != 0.0:
		rotate_player(smooth_turn_input * SMOOTH_TURN_SPEED)


func _on_right_hand_button_pressed(name: String) -> void:
	if name == "ax_button":
		var fb_space_warp = Engine.get_singleton("OpenXRFbSpaceWarpExtensionWrapper")
		fb_space_warp.set_space_warp_enabled(!fb_space_warp.is_enabled())

		if fb_space_warp.is_enabled():
			right_controller_label.text = right_controller_label.text.replace("DISABLED", "ENABLED")
		else:
			right_controller_label.text = right_controller_label.text.replace("ENABLED", "DISABLED")
	elif name == "by_button":
		if not snap_turn_enabled:
			smooth_turn_input = 0.0
		snap_turn_enabled = !snap_turn_enabled

		if snap_turn_enabled:
			right_controller_label.text = right_controller_label.text.replace("SMOOTH", "SNAP")
		else:
			right_controller_label.text = right_controller_label.text.replace("SNAP", "SMOOTH")


func check_turn(name: String, value: Vector2) -> void:
	if snap_turn_enabled:
		if not turn_timer.is_stopped():
			return

		if name == "primary":
			if abs(value.x) > SNAP_TURN_THRESHOLD:
				rotate_player(sign(value.x) * SNAP_TURN_ANGLE)
				turn_timer.start()
				var fb_space_warp = Engine.get_singleton("OpenXRFbSpaceWarpExtensionWrapper")
				fb_space_warp.skip_space_warp_frame()
	else:
		smooth_turn_input = value.x


func rotate_player(angle: float):
	var t1 := Transform3D()
	var t2 := Transform3D()
	var rot := Transform3D()

	t1.origin = -xr_camera.transform.origin
	t2.origin = xr_camera.transform.origin
	rot = rot.rotated(Vector3.DOWN, angle)
	xr_origin.transform = (xr_origin.transform * t2 * rot * t1).orthonormalized()

	for composition_layer in get_tree().get_nodes_in_group("composition_layer"):
		composition_layer.global_transform = get_node(NodePath(composition_layer.name)).global_transform


func _on_left_hand_input_vector_2_changed(name: String, value: Vector2) -> void:
	if name == "primary":
		movement_input = value
