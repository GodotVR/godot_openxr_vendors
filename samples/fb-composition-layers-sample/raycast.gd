extends RayCast3D

@onready var mesh_instance_3d: MeshInstance3D = $MeshInstance3D

func _process(delta: float) -> void:
	var mat = mesh_instance_3d.get_surface_override_material(0)

	if not is_colliding():
		mesh_instance_3d.mesh.height = 10
		mesh_instance_3d.position.y = -5
		mat.albedo_color = Color.RED
	else:
		var height = (get_collision_point() - global_position).length()
		mesh_instance_3d.mesh.height = height
		mesh_instance_3d.position.y = -(height / 2)
		mat.albedo_color = Color.GREEN
