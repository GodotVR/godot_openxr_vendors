extends XRAnchor3D

## Signal emitted when the OpenXRAndroidTrackablePlaneTracker is subsumed by another
## OpenXRAndroidTrackablePlaneTracker
signal subsumed

var _trackable_plane: OpenXRAndroidTrackablePlaneTracker


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
	_on_trackable_plane_vertices_updated()

	set_hit(false)


func set_hit(is_hit: bool):
	$MeshInstance3D.mesh.material.set_shader_parameter("uColor", Color.YELLOW if is_hit else Color.WHITE)


func _on_trackable_plane_vertices_updated():
	if _trackable_plane.get_subsumed_by_plane():
		subsumed.emit()
		return

	$MeshInstance3D.mesh.size = _trackable_plane.get_extents()
	$MeshInstance3D.mesh.material.set_shader_parameter("uExtents", _trackable_plane.get_extents())
