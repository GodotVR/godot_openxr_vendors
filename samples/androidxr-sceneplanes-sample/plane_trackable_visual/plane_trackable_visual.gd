extends XRAnchor3D

## Signal emitted when the OpenXRAndroidTrackablePlaneTracker is subsumed by another
## OpenXRAndroidTrackablePlaneTracker
signal subsumed

var _trackable_plane: OpenXRAndroidTrackablePlaneTracker
var _mesh_instance: MeshInstance3D
var _collision_shape: CollisionShape3D


func set_trackable_plane(new_trackable_plane: OpenXRAndroidTrackablePlaneTracker):
	# currently only handle one trackable plane
	if _trackable_plane:
		push_error("Expect exactly one call to each set_trackable_plane; why was this called twice?")
		return

	_trackable_plane = new_trackable_plane
	_trackable_plane.updated.connect(_on_trackable_plane_vertices_updated)

	# Set this XRAnchor3D's tracker to the XRTracker's name to receive automatic pose updates
	# (the XRTracker updates the "default" pose, which happens to be our default pose name too)
	tracker = _trackable_plane.get_tracker_name()

	# the trackable is not updated before we receive it, so we have to call this to see the first mesh
	_mesh_instance = $StaticBody3D/MeshInstance3D
	_collision_shape = $StaticBody3D/CollisionShape3D
	_on_trackable_plane_vertices_updated()


func _on_trackable_plane_vertices_updated():
	if _trackable_plane.get_subsumed_by_plane():
		subsumed.emit()
		return

	_mesh_instance.mesh = _trackable_plane.get_mesh()
	_mesh_instance.material_override.set_shader_parameter("color", _get_color_from_plane_label(_trackable_plane.get_plane_label()))
	_collision_shape.shape = _trackable_plane.get_shape(0.1)


func _get_color_from_plane_label(plane_label: OpenXRAndroidTrackablePlaneTracker.PlaneLabel) -> Color:
	match plane_label:
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_UNKNOWN:
			return Color.RED
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_WALL:
			return Color.GREEN
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_FLOOR:
			return Color.BLUE
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_CEILING:
			return Color.YELLOW
		OpenXRAndroidTrackablePlaneTracker.PLANE_LABEL_TABLE:
			# Violet (Color.VIOLET is something else)
			return Color(1, 0, 1, 1)
	return Color.WHITE
