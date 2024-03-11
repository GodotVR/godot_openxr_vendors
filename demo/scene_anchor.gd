extends Node3D

const GRID_MATERIAL: StandardMaterial3D = preload("res://assets/cross-grid-material.tres")

@onready var label: Label3D = $Label3D
@onready var static_body: StaticBody3D = $StaticBody3D

var mesh_instance: MeshInstance3D

func setup_scene(entity: OpenXRFbSpatialEntity) -> void:
	var semantic_labels: PackedStringArray = entity.get_semantic_labels()

	label.text = ", ".join(semantic_labels)

	var collision_shape = entity.create_collision_shape()
	if collision_shape:
		static_body.add_child(collision_shape)

	mesh_instance = entity.create_mesh_instance()
	if not mesh_instance:
		mesh_instance = MeshInstance3D.new()
		var box_mesh := BoxMesh.new()
		box_mesh.size = Vector3(0.1, 0.1, 0.1)
		mesh_instance.mesh = box_mesh

	# Adjust the material for the entity type.
	var material: StandardMaterial3D = GRID_MATERIAL.duplicate()
	if semantic_labels.size() > 0:
		material.albedo_color = _get_color_for_label(semantic_labels[0])
	if mesh_instance.mesh is BoxMesh:
		material.uv1_scale = Vector3(3, 2, 1)
	mesh_instance.set_surface_override_material(0, material)

	add_child(mesh_instance)

func _get_color_for_label(semantic_label) -> Color:
	match semantic_label:
		"CEILING","FLOOR":
			return Color(0.0, 0.0, 0.0, 1.0)
		"WALL_FACE","INVISIBLE_WALL_FACE":
			return Color(0.0, 0.0, 1.0, 1.0)
		"WINDOW_FRAME","DOOR_FRAME":
			return Color(1.0, 0.0, 0.0, 1.0)
		"COUCH","TABLE","BED","LAMP","PLANT","SCREEN","STORAGE":
			return Color(0.0, 1.0, 0.0, 1.0)

	return Color(1.0, 1.0, 1.0, 1.0)
