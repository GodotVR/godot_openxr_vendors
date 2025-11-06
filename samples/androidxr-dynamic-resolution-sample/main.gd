extends StartXR


func _ready():
	super._ready()

	XRServer.get_tracker("head").pose_changed.connect(_wait_for_camera)


func _wait_for_camera(pose: XRPose):
	if pose.name != &"default":
		return

	if pose.transform == Transform3D.IDENTITY:
		# still waiting
		return

	# stop waiting for the camera
	XRServer.get_tracker("head").pose_changed.disconnect(_wait_for_camera)

	# Connect to the hud
	$XROrigin3D/Hud.data_source = _get_resolution_text


func _update_spheres():
	if 0 == $XROrigin3D/Spheres.get_child_count():
		_show_spheres()
	else:
		_delete_spheres()


func _show_spheres():
	# these values are arbitrary - position enough spheres in front of the camera to affect
	# performance.
	# Also, make spheres only because they have many vertices
	var maxrow := 30
	var maxcol := 30
	var topleft := Vector3(-4.0, 4.0, -5.0)

	var dims := Vector2(topleft.x * topleft.x, topleft.y * topleft.y)
	var spherescene: PackedScene = load("res://sphere.tscn")
	for row in maxrow:
		for col in maxcol:
			var sphere = spherescene.instantiate()
			$XROrigin3D/Spheres.add_child(sphere)

			var offset := Vector3(topleft.x + dims.x * float(col) / float(maxcol), topleft.y - dims.y * float(row) / float(maxrow), topleft.z)
			sphere.global_transform = $XROrigin3D/XRCamera3D.get_camera_transform().translated_local(offset)
			sphere.set_color(Color(randf_range(0.0, 1.0), randf_range(0.0, 1.0), randf_range(0.0, 1.0), 1.0))


func _delete_spheres():
	for sphere in $XROrigin3D/Spheres.get_children():
		sphere.queue_free()


func _get_resolution_text() -> String:
	var render_target_size: Vector2 = xr_interface.get_render_target_size()
	return "Recommended resolution:\n  width %s\n  height %s" % [render_target_size.x, render_target_size.y]
