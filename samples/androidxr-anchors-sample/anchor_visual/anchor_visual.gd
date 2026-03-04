extends XRAnchor3D

var _info: Control
var _anchor_tracker: OpenXRAndroidAnchorTracker
var _location_flags: OpenXRAndroidAnchorTracker.LocationFlags = 0
var _persist_state: OpenXRAndroidAnchorTracker.PersistState = OpenXRAndroidAnchorTracker.PERSIST_STATE_NOT_REQUESTED

const XR_UTILS = preload("res://xr_utils.gd")


func _ready():
	_info = $Viewport2Din3D.get_scene_root()
	set_anchor_tracker(_anchor_tracker)

	_info.resized.connect(_on_hud_resized)
	_on_hud_resized()


func _process(_delta: float) -> void:
	# make the hud always look at the camera
	$Viewport2Din3D.look_at(get_tree().root.get_camera_3d().get_camera_transform().origin, Vector3.UP, true)


func set_anchor_tracker(anchor_tracker: OpenXRAndroidAnchorTracker):
	if _anchor_tracker:
		tracker = &""
		_anchor_tracker.location_flags_changed.disconnect(_location_flags_changed)
		_anchor_tracker.persist_state_changed.disconnect(_persist_state_changed)
		_anchor_tracker.pose_changed.disconnect(_on_pose_changed)
	_anchor_tracker = anchor_tracker

	if _anchor_tracker != null:
		tracker = _anchor_tracker.get_tracker_name()
		_anchor_tracker.location_flags_changed.connect(_location_flags_changed)
		_anchor_tracker.persist_state_changed.connect(_persist_state_changed)
		_anchor_tracker.pose_changed.connect(_on_pose_changed)
		_persist_state_changed()
		_location_flags_changed()

	if _info:
		_info.set_title(XR_UTILS.get_anchor_tracker_string(_anchor_tracker))


func _on_hud_resized():
	$Viewport2Din3D.set_viewport_size(_info.size)

	var hud_size: Vector2 = $Viewport2Din3D.get_size()
	$Viewport2Din3D.position = Vector3(0.0, hud_size.y / 2.0 + $MeshInstance3D.mesh.size.y / 2.0, 0.0)


func _persist_state_changed():
	_persist_state = OpenXRAndroidDeviceAnchorPersistenceExtension.get_anchor_tracker_persist_state(_anchor_tracker)
	_info.set_persisted_state_string(XR_UTILS.get_persist_state_string(_persist_state))
	_update_color()


func _location_flags_changed():
	if _info == null:
		return

	if _anchor_tracker == null:
		printerr("failed to get location flags")
		return

	_location_flags = _anchor_tracker.get_location_flags()
	_info.set_location_flags(_location_flags)
	_update_color()


func _update_color():
	var is_tracked := 0 != (_location_flags & OpenXRAndroidAnchorTracker.LOCATION_FLAGS_ORIENTATION_TRACKED) && 0 != (_location_flags & OpenXRAndroidAnchorTracker.LOCATION_FLAGS_POSITION_TRACKED)
	var new_color: Color
	match _persist_state:
		OpenXRAndroidAnchorTracker.PERSIST_STATE_NOT_REQUESTED:
			new_color = Color.YELLOW if is_tracked else Color.RED
		OpenXRAndroidAnchorTracker.PERSIST_STATE_PENDING:
			new_color = Color.DARK_GREEN if is_tracked else Color.DARK_ORANGE
		OpenXRAndroidAnchorTracker.PERSIST_STATE_PERSISTED:
			new_color = Color.GREEN if is_tracked else Color.ORANGE
		OpenXRAndroidAnchorTracker.PERSIST_STATE_ERROR:
			new_color = Color.DARK_RED
		_:
			new_color = Color.WHITE
	$MeshInstance3D.mesh.material.albedo_color = new_color


func _on_pose_changed(new_pose: XRPose):
	if new_pose.name != &"default":
		return

	_info.set_position_text(new_pose.transform.origin)
