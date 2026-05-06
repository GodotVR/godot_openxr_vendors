extends XRAnchor3D

## Signal emitted when the OpenXRAndroidTrackablePlaneTracker is subsumed by another
## OpenXRAndroidTrackablePlaneTracker
signal subsumed

var _plane_tracker: OpenXRAndroidTrackablePlaneTracker
var _mesh_instance: MeshInstance3D
var _collision_shape: CollisionShape3D
var _is_hit := false


func set_plane_tracker(new_plane_tracker: OpenXRAndroidTrackablePlaneTracker):
	# currently only handle one plane tracker
	if _plane_tracker:
		push_error("Expect exactly one call to each set_plane_tracker; why was this called twice?")
		return

	_plane_tracker = new_plane_tracker
	_plane_tracker.updated.connect(_on_trackable_plane_vertices_updated)

	# Set this XRAnchor3D's tracker to the XRTracker's name to receive automatic pose updates
	# (the XRTracker updates the "default" pose, which happens to be our default pose name too)
	tracker = _plane_tracker.get_tracker_name()

	# the trackable is not updated before we receive it, so we have to call this to see the first mesh
	_mesh_instance = $StaticBody3D/MeshInstance3D
	_collision_shape = $StaticBody3D/CollisionShape3D
	_on_trackable_plane_vertices_updated()

	add_to_group("planes_raycast%s" % _plane_tracker.get_instance_id())


# Group callback from "planes_raycast%s" group
func set_plane_hit(is_hit: bool):
	_is_hit = is_hit
	_set_color()


func _on_trackable_plane_vertices_updated():
	if _plane_tracker.get_subsumed_by_plane():
		subsumed.emit()
		return

	_mesh_instance.mesh = _plane_tracker.get_mesh()
	_collision_shape.shape = _plane_tracker.get_shape(0.1)
	_set_color()


func _set_color():
	var color: Color = Color.WHITE
	match _plane_tracker.get_plane_label():
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_UNKNOWN:
			color = Color.RED
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_WALL:
			color = Color.GREEN
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_FLOOR:
			color = Color.BLUE
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_CEILING:
			color = Color.YELLOW
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_TABLE:
			# Violet (Color.VIOLET is something else)
			color = Color(1, 0, 1, 1)

	# reduce the saturation for hit planes (i.e make them a lighter color)
	if _is_hit:
		color.s = 0.4

	_mesh_instance.material_override.set_shader_parameter("color", color)
