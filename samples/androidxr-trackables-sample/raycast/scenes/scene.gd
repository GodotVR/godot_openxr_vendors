extends Node3D

enum RaycastFlags { PLANE = 1, DEPTH = 2 }
@export_flags("Plane", "Depth") var raycast_flags: int = RaycastFlags.PLANE

@export_multiline var hud_instructions := ""

const HITPOSE_NODE_VISUAL = preload("res://addons/common/axis3d/axis3d.tscn")

# Array of OpenXRAndroidTrackablePlaneTrackers that were hit by a raycast last frame
var _last_hit_plane_trackers := []


func _process(_delta) -> void:
	# hide old hitpose node(s)
	for hitpose_node in get_children():
		hitpose_node.visible = false

	# un-hit planes that were hit last frame
	for last_hit_plane in _last_hit_plane_trackers:
		_set_hit_plane(last_hit_plane, false)
	_last_hit_plane_trackers.clear()

	# find out what we're raycasting
	var raycast_plane := 0 != (raycast_flags & RaycastFlags.PLANE)
	var raycast_depth := 0 != (raycast_flags & RaycastFlags.DEPTH)

	# Raycast against all planes, from the camera
	# Draw a X/Y/Z axis ("HIT_NODE") at the position and orientation of the hit plane(s) (hit
	# planes should be from nearest to furthest)
	var trackable_types: Array
	if raycast_plane:
		trackable_types.push_back(OpenXRAndroidRaycastExtension.TRACKABLE_TYPE_PLANE)
	if raycast_depth:
		trackable_types.push_back(OpenXRAndroidRaycastExtension.TRACKABLE_TYPE_DEPTH)

	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()
	var hitresults: Array[OpenXRAndroidHitResult] = OpenXRAndroidRaycastExtension.raycast(trackable_types, camera_transform.origin, -camera_transform.basis.z, 30)

	var next_hitpose_node_idx := 0
	for hitresult in hitresults:
		var tracker: OpenXRAndroidTrackableTracker = hitresult.get_tracker()
		if !raycast_depth && !(tracker is OpenXRAndroidTrackablePlaneTracker):
			printerr("Expected only PLANE trackables")
			continue

		# Show a HITPOSE_NODE_VISUAL at the raycast hit pose
		# (use cached hit pose if one is available)
		var hitpose_node: Node3D
		if next_hitpose_node_idx < get_child_count():
			hitpose_node = get_children()[next_hitpose_node_idx]
			hitpose_node.visible = true
		else:
			hitpose_node = HITPOSE_NODE_VISUAL.instantiate()
			add_child(hitpose_node)
		hitpose_node.global_transform = hitresult.get_pose()
		next_hitpose_node_idx += 1

		# Handle hit plane
		if tracker is OpenXRAndroidTrackablePlaneTracker:
			_last_hit_plane_trackers.push_back(tracker)
			_set_hit_plane(tracker, true)

		# When raycasting depth, break after the first one.
		# When raycasting planes (with/without depth), keep going to collect all intersected planes
		if raycast_depth && !raycast_plane:
			break


func _set_hit_plane(hit_plane_tracker: OpenXRAndroidTrackablePlaneTracker, is_hit: bool):
	get_tree().call_group("planes_raycast%s" % hit_plane_tracker.get_instance_id(), "set_plane_hit", is_hit)
