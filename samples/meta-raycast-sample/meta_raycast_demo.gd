extends StartXR

# Demonstrates the XR_META_environment_raycast OpenXR extension.
#
# Every frame a ray is cast out of the tip of the right index finger (hand
# tracking), or from the right controller or the headset when no hand is
# tracked, against the physical environment. A ring reticle marks the hit point.
# An index finger pinch or the trigger / A button places a small box on the
# surface, a middle finger pinch or the B button removes all placed boxes.

@export var max_raycast_distance: float = 10.0

var raycast_ext

@onready var xr_origin: XROrigin3D = $XROrigin3D
@onready var camera: XRCamera3D = $XROrigin3D/XRCamera3D
@onready var right_controller: XRController3D = $XROrigin3D/RightController
@onready var right_hand_aim: XRController3D = $XROrigin3D/RightHandAim
@onready var reticle: Node3D = $Reticle
@onready var box_template: Node3D = $PlacedBox
@onready var placed_boxes: Node3D = $PlacedBoxes


func _ready() -> void:
	super._ready()

	raycast_ext = Engine.get_singleton("OpenXRMetaEnvironmentRaycastExtension")
	if not raycast_ext or not raycast_ext.is_environment_raycast_supported():
		push_error("Environment raycast is not supported on this device/runtime.")
		return

	raycast_ext.raycaster_error.connect(func(error_code: int) -> void: push_error("Environment raycaster error: %d" % error_code))
	right_controller.button_pressed.connect(_on_button_pressed)
	right_hand_aim.button_pressed.connect(_on_button_pressed)


func _process(_delta: float) -> void:
	reticle.visible = false
	if not raycast_ext or not raycast_ext.is_raycaster_ready():
		return

	# The extension works in the OpenXR play space (relative to XROrigin3D, plus
	# any recentering via XRServer), so the ray goes in and the hit comes out in
	# that space.
	var play_to_world: Transform3D = xr_origin.global_transform * XRServer.get_reference_frame()
	var origin: Vector3
	var direction: Vector3
	var hand := XRServer.get_tracker("/user/hand_tracker/right") as XRHandTracker
	if hand and hand.has_tracking_data:
		# Out of the index finger tip, along the last finger segment. Hand joint
		# transforms are already in play space.
		var tip: Transform3D = hand.get_hand_joint_transform(XRHandTracker.HAND_JOINT_INDEX_FINGER_TIP)
		var distal: Transform3D = hand.get_hand_joint_transform(XRHandTracker.HAND_JOINT_INDEX_FINGER_PHALANX_DISTAL)
		origin = tip.origin
		direction = (tip.origin - distal.origin).normalized()
	else:
		# Controller or headset poses are in world space, convert them.
		var source: Node3D = right_controller if right_controller.get_is_active() else camera
		var world_to_play: Transform3D = play_to_world.affine_inverse()
		origin = world_to_play * source.global_position
		direction = world_to_play.basis * -source.global_transform.basis.z

	var hit: Dictionary = raycast_ext.raycast(origin, direction, max_raycast_distance)

	# Only HIT_STATUS_HIT comes with a usable pose. Its Z axis is the surface normal.
	if hit.status == OpenXRMetaEnvironmentRaycastExtension.HIT_STATUS_HIT:
		reticle.global_transform = play_to_world * hit.transform
		reticle.visible = true


func _on_button_pressed(button_name: String) -> void:
	if button_name in ["index_pinch", "trigger_click", "ax_button"] and reticle.visible:
		var box: Node3D = box_template.duplicate()
		box.visible = true
		placed_boxes.add_child(box)
		box.global_transform = reticle.global_transform
	elif button_name in ["middle_pinch", "by_button"]:
		for box in placed_boxes.get_children():
			box.queue_free()
