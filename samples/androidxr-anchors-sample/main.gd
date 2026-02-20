extends StartXR

const ANCHOR_VISUAL = preload("res://anchor_visual/anchor_visual.tscn")
const PLANE_OUTLINE_3D = preload("res://plane_outline_3d/plane_outline_3d.tscn")
const TRANSFORM3D_UTILS = preload("res://transform3d_utils.gd")

# Maps [OpenXRAndroidAnchorTracker] -> ANCHOR_VISUAL
var _anchor_tracker_to_visual := {}

# Maps [OpenXRAndroidAnchorTracker] -> PLANE_OUTLINE_3D
var _anchor_tracker_to_plane_outline := {}


func _ready():
	super._ready()

	OS.request_permissions()

	XRServer.get_tracker("head").pose_changed.connect(_wait_for_camera)


func _wait_for_camera(pose: XRPose):
	if pose.name != &"default":
		return

	if pose.transform == Transform3D.IDENTITY:
		# still waiting
		return

	# stop waiting for the camera
	XRServer.get_tracker("head").pose_changed.disconnect(_wait_for_camera)

	# create a reference anchor visual in front of the camera,
	var camera_transform: Transform3D = $XROrigin3D/XRCamera3D.get_camera_transform()
	var visual: Node3D = ANCHOR_VISUAL.instantiate()
	add_child(visual)
	visual.global_transform = TRANSFORM3D_UTILS.upright_then_offset(camera_transform, Vector3(0.0, 0.0, -0.2))

	XRServer.tracker_added.connect(_on_tracker_added)
	XRServer.tracker_removed.connect(_on_tracker_removed)

	# this may always find zero anchors since zero anchors have been created, but try anyway
	for tracker in XRServer.get_trackers(XRServer.TRACKER_ANCHOR).values():
		_on_anchor_tracker_added(_get_tracker_as_anchor_tracker(tracker))

	$XROrigin3D/XRController3DLeftHand/HandPinchDetector.pinch_tapped.connect($XROrigin3D/Menu3D.on_pinch_tapped.bind(OpenXRHand.Hands.HAND_LEFT))
	$XROrigin3D/XRController3DRightHand/HandPinchDetector.pinch_tapped.connect($XROrigin3D/Menu3D.on_pinch_tapped.bind(OpenXRHand.Hands.HAND_RIGHT))


func _process(_delta: float) -> void:
	# position the menu in front of the camera, when the camera's UP vector is Vector3.UP

	var camera_transform: Transform3D = $XROrigin3D/XRCamera3D.get_camera_transform()

	# apply an offset to the camera, when the camera is upright
	var menu_transform: Transform3D = TRANSFORM3D_UTILS.upright_then_offset(camera_transform, Vector3(0.0, 0.0, -0.5))

	# make Menu3D look at the camera
	$XROrigin3D/Menu3D.global_transform = menu_transform.looking_at(camera_transform.origin, Vector3.UP, true)


func _on_tracker_added(tracker_name: StringName, _type: int):
	_on_anchor_tracker_added(_get_as_anchor_tracker(tracker_name))


func _on_tracker_removed(tracker_name: StringName, _type: int):
	_on_anchor_tracker_removed(tracker_name)


func _on_anchor_tracker_added(anchor_tracker: OpenXRAndroidAnchorTracker):
	if anchor_tracker == null:
		return

	# sanity; this "should never happen"
	if _anchor_tracker_to_visual.has(anchor_tracker):
		printerr("AnchorVisual already created for anchor tracker (when creating)!")
		return

	var visual: Node3D = ANCHOR_VISUAL.instantiate()
	add_child(visual)
	visual.set_anchor_tracker(anchor_tracker)

	# draw an outline of the plane if this anchor tracker is for a plane trackable
	var tracker: OpenXRAndroidTrackableTracker = anchor_tracker.get_tracker()
	if tracker is OpenXRAndroidTrackablePlaneTracker:
		var plane_outline: Node3D = PLANE_OUTLINE_3D.instantiate()
		plane_outline.set_plane_tracker(tracker)
		add_child(plane_outline)
		_anchor_tracker_to_plane_outline[anchor_tracker] = plane_outline

	_anchor_tracker_to_visual[anchor_tracker] = visual


func _on_anchor_tracker_removed(tracker_name: StringName):
	var anchor_tracker: OpenXRAndroidAnchorTracker = _get_as_anchor_tracker(tracker_name)
	if anchor_tracker == null:
		return

	# sanity; this "should never happen"
	if !_anchor_tracker_to_visual.has(anchor_tracker):
		printerr("Missing AnchorVisual for anchor tracker (when destroying)!")
		return

	_anchor_tracker_to_visual[anchor_tracker].queue_free()
	_anchor_tracker_to_visual.erase(anchor_tracker)

	if _anchor_tracker_to_plane_outline.has(anchor_tracker):
		_anchor_tracker_to_plane_outline[anchor_tracker].queue_free()
		_anchor_tracker_to_plane_outline.erase(anchor_tracker)


func _get_as_anchor_tracker(tracker_name: StringName) -> OpenXRAndroidAnchorTracker:
	return _get_tracker_as_anchor_tracker(XRServer.get_tracker(tracker_name))


func _get_tracker_as_anchor_tracker(tracker: XRTracker) -> OpenXRAndroidAnchorTracker:
	if not tracker:
		push_error("Couldn't retrieve tracker!")
		return null

	if !(tracker is OpenXRAndroidAnchorTracker):
		# this is okay
		return null

	return tracker
