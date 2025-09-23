extends Node3D

const WIREFRAME_MATERIAL: Material = preload("res://assets/wireframe-material.tres")

@onready var static_body: StaticBody3D = $StaticBody3D

var mesh_instance: MeshInstance3D


func setup_scene(entity: OpenXRFbSpatialEntity) -> void:
	var collision_shape = entity.create_collision_shape()
	if collision_shape:
		static_body.add_child(collision_shape)

	#mesh_instance = entity.create_mesh_instance()
	#if mesh_instance:
	#	mesh_instance.set_surface_override_material(0, WIREFRAME_MATERIAL)
	#	add_child(mesh_instance)

	# Calling entity.create_mesh_instance() works, however, our wireframe shader only
	# works right if vertices aren't shared among multiple triangles, so we have to
	# process the mesh data to ensure that.
	var mesh_array: Array = entity.get_triangle_mesh()
	if not mesh_array.is_empty():
		mesh_instance = MeshInstance3D.new()

		var vertices := PackedVector3Array()
		vertices.resize(mesh_array[Mesh.ARRAY_INDEX].size())
		for i in range(mesh_array[Mesh.ARRAY_INDEX].size()):
			vertices[i] = mesh_array[Mesh.ARRAY_VERTEX][mesh_array[Mesh.ARRAY_INDEX][i]]

		mesh_array[Mesh.ARRAY_VERTEX] = vertices
		mesh_array[Mesh.ARRAY_INDEX] = null

		var mesh := ArrayMesh.new()
		mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, mesh_array)
		mesh_instance.mesh = mesh

		mesh_instance.set_surface_override_material(0, WIREFRAME_MATERIAL)
		add_child(mesh_instance)
