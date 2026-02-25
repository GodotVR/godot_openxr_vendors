extends StartXR

enum RaycastFlags { PLANE = 1, DEPTH = 2 }
@export_flags("Plane", "Depth") var raycast_flags: int = RaycastFlags.PLANE:
	set = set_raycast_flags

@export_multiline var hud_instructions := "":
	set = set_hud_instructions
@export_file_path("*.tscn") var next_scene: String

const RECTANGULAR_PLANE_TRACKABLE_VISUAL = preload("res://scenes/rectangular_plane_trackable_visual/rectangular_plane_trackable_visual.tscn")
const HITPOSE_NODE_VISUAL = preload("res://addons/common/axis3d/axis3d.tscn")

# Maps tracker names (StringName) -> rectangular plane trackable visual (XRAnchor3D)
var _rectangular_plane_trackable_visuals := {}

# Array of the XRTracker names (StringName) for the OpenXRAndroidTrackablePlaneTrackers that were hit by a
# raycast last frame
var _last_hit_planes := []

var _first_valid_camera_transform := Transform3D.IDENTITY


func _ready() -> void:
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

	set_first_valid_camera_transform(pose.transform)


func _process(_delta) -> void:
	# hide old hitpose node(s)
	for hitpose_node in $HitPoseNodes.get_children():
		hitpose_node.visible = false

	# un-hit planes that were hit last frame
	for last_hit_plane in _last_hit_planes:
		_set_hit_plane(last_hit_plane, false)
	_last_hit_planes.clear()

	# find out what we're raycasting
	var raycast_plane := 0 != (raycast_flags & RaycastFlags.PLANE)
	var raycast_depth := 0 != (raycast_flags & RaycastFlags.DEPTH)

	# Raycast against all planes, from the camera
	# Draw a X/Y/Z axis ("HIT_NODE") at the position and orientation of the first hit plane (hit
	# planes should be from nearest to furthest)
	var trackable_types: Array
	if raycast_plane:
		trackable_types.push_back(OpenXRAndroidRaycastExtension.TRACKABLE_TYPE_PLANE)
	if raycast_depth:
		trackable_types.push_back(OpenXRAndroidRaycastExtension.TRACKABLE_TYPE_DEPTH)

	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()
	var hitresults: Array[OpenXRAndroidHitResult] = OpenXRAndroidRaycastExtension.raycast(trackable_types, camera_transform.origin, -camera_transform.basis.z, 30)

	var next_hitpose_node_idx := 0
	if !hitresults.is_empty():
		for hitresult in hitresults:
			var tracker: OpenXRAndroidTrackableTracker = hitresult.get_tracker()
			if !raycast_depth && !(tracker is OpenXRAndroidTrackablePlaneTracker):
				printerr("Expected only PLANE trackables")
				continue

			# Show a HITPOSE_NODE_VISUAL at the raycast hit pose
			# (use cached hit pose if one is available)
			var hitpose_node: Node3D
			if next_hitpose_node_idx < $HitPoseNodes.get_child_count():
				hitpose_node = $HitPoseNodes.get_children()[next_hitpose_node_idx]
				hitpose_node.visible = true
			else:
				hitpose_node = HITPOSE_NODE_VISUAL.instantiate()
				$HitPoseNodes.add_child(hitpose_node)
			hitpose_node.global_transform = hitresult.get_pose()
			next_hitpose_node_idx += 1

			# Handle hit plane
			if tracker is OpenXRAndroidTrackablePlaneTracker:
				_last_hit_planes.push_back(tracker.get_tracker_name())
				_set_hit_plane(tracker.get_tracker_name(), true)

			# When raycasting depth, break after the first one.
			# When raycasting planes (and/or depth), keep going to collect all intersected planes
			if raycast_depth && !raycast_plane:
				break


func _set_hit_plane(hit_plane_tracker_name: StringName, is_hit: bool):
	if _rectangular_plane_trackable_visuals.has(hit_plane_tracker_name):
		_rectangular_plane_trackable_visuals[hit_plane_tracker_name].set_hit(is_hit)


func set_first_valid_camera_transform(first_valid_camera_transform: Transform3D):
	# Only set the first valid camera (and complete initialization) if we haven't already.
	# This code uses $Viewport2Din3D's visibility to determine if this function has been called before, for the
	# current instance.
	if $Viewport2Din3D.visible:
		return
	$Viewport2Din3D.visible = true

	_first_valid_camera_transform = first_valid_camera_transform

	# reposition the hud
	var camera_transform: Transform3D = first_valid_camera_transform
	var forward_xz := Vector3(camera_transform.basis.z.x, 0.0, camera_transform.basis.z.z).normalized()
	var right := Vector3.UP.cross(forward_xz)
	var xform := Transform3D(right, Vector3.UP, forward_xz, camera_transform.origin)

	# put it to the left
	xform.origin = xform * Vector3(-1.5, 0.0, -0.3)

	# and point it towards the camera
	$Viewport2Din3D.global_transform = xform.looking_at(first_valid_camera_transform.origin, Vector3.UP, true)

	# detect left/right hand pinches
	$XRController3DLeftHand/HandPinchDetector.pinch_tapped.connect(_on_pinch_tapped)
	$XRController3DRightHand/HandPinchDetector.pinch_tapped.connect(_on_pinch_tapped)

	set_hud_instructions(hud_instructions)

	if 0 == (raycast_flags & RaycastFlags.PLANE):
		return

	# be notified of new trackers added by OpenXRAndroidTrackablesExtension
	XRServer.tracker_added.connect(_on_tracker_added)
	XRServer.tracker_removed.connect(_on_tracker_removed)

	# process existing trackers
	var trackers: Dictionary = XRServer.get_trackers(XRServer.TRACKER_ANCHOR)
	for tracker_name in trackers:
		_on_tracker_added(tracker_name, 0)


func set_raycast_flags(new_raycast_flags: RaycastFlags):
	raycast_flags = new_raycast_flags


func set_hud_instructions(new_hud_instructions: String):
	hud_instructions = new_hud_instructions

	if $Viewport2Din3D != null:
		$Viewport2Din3D.get_scene_root().text = hud_instructions


func _on_pinch_tapped():
	var treeroot: Viewport = get_tree().root
	var this_scene_root = treeroot.get_children()[treeroot.get_child_count() - 1]
	var next_scene_root = load(next_scene).instantiate()
	if next_scene_root == null:
		push_error("Failed to switch to next scene")
		return

	treeroot.add_child(next_scene_root)
	next_scene_root.set_first_valid_camera_transform(_first_valid_camera_transform)
	this_scene_root.queue_free()


func _on_tracker_added(tracker_name: StringName, _type: int):
	var tracker: XRTracker = XRServer.get_tracker(tracker_name)
	if not tracker:
		push_error("Couldn't retrieve tracker!")
		return

	if !(tracker is OpenXRAndroidTrackablePlaneTracker):
		# this is okay
		return

	if _rectangular_plane_trackable_visuals.size() == 30:
		print("WARNING: only showing at most 30 planes")
		return

	if _rectangular_plane_trackable_visuals.has(tracker_name):
		return

	# don't visualize planes that are subsumed by another plane, since they are identical (xform,
	# extents, and vertices) and usually produce confusing/congested visuals.
	if tracker.get_subsumed_by_plane():
		return

	var rectangular_plane_trackable_visual: XRAnchor3D = RECTANGULAR_PLANE_TRACKABLE_VISUAL.instantiate()
	rectangular_plane_trackable_visual.set_trackable_plane(tracker)
	rectangular_plane_trackable_visual.subsumed.connect(_on_tracker_removed_impl.bind(tracker_name))
	add_child(rectangular_plane_trackable_visual)
	_rectangular_plane_trackable_visuals[tracker_name] = rectangular_plane_trackable_visual


func _on_tracker_removed(tracker_name: StringName, _type: int):
	_on_tracker_removed_impl(tracker_name)


func _on_tracker_removed_impl(tracker_name: StringName):
	if !_rectangular_plane_trackable_visuals.has(tracker_name):
		# this is okay; we don't keep trackable planes that are subsumed by another plane
		return

	_rectangular_plane_trackable_visuals[tracker_name].queue_free()
	_rectangular_plane_trackable_visuals.erase(tracker_name)
