extends XRAnchor3D

var _plane_tracker: OpenXRAndroidTrackablePlaneTracker
var _color: Color


func _ready():
	set_plane_tracker(_plane_tracker)

	# choose a random color to (likely) not conflict with other plane outlines
	_color = Color(randf_range(0.0, 1.0), randf_range(0.0, 1.0), randf_range(0.0, 1.0), 1.0)


func set_plane_tracker(plane_tracker: OpenXRAndroidTrackablePlaneTracker):
	if _plane_tracker:
		tracker = ""
		_plane_tracker.updated.disconnect(_on_trackable_plane_vertices_updated)

	_plane_tracker = plane_tracker

	if _plane_tracker:
		tracker = _plane_tracker.get_tracker_name()
		_plane_tracker.updated.connect(_on_trackable_plane_vertices_updated)
		_on_trackable_plane_vertices_updated()


func _on_trackable_plane_vertices_updated():
	if $MeshInstance3D == null:
		return

	var plane_vertices: PackedVector3Array = _plane_tracker.get_vertices()
	if plane_vertices.is_empty():
		return

	# draw the vertices
	var immediate_mesh: ImmediateMesh = $MeshInstance3D.mesh
	immediate_mesh.clear_surfaces()
	immediate_mesh.surface_begin(Mesh.PRIMITIVE_LINE_STRIP)
	immediate_mesh.surface_set_color(_color)

	# Plane vertices are Vector3 and are CCW
	for vertex in plane_vertices:
		immediate_mesh.surface_add_vertex(vertex)

	# connect the last vertex to the first
	immediate_mesh.surface_add_vertex(plane_vertices[0])

	immediate_mesh.surface_end()
