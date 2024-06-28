extends Area3D

@onready var mesh_instance: MeshInstance3D = $MeshInstance3D

var color: Color
var selected := false

func setup_scene(spatial_entity: OpenXRFbSpatialEntity) -> void:
	var data: Dictionary = spatial_entity.custom_data

	color = Color(data.get('color', '#FFFFFF'))

	var material: StandardMaterial3D = mesh_instance.get_surface_override_material(0)
	material.albedo_color = color
	mesh_instance.set_surface_override_material(0, material)

	spatial_entity.openxr_fb_spatial_entity_saved.connect(self._on_saved_to_cloud.bind(spatial_entity))
	spatial_entity.save_to_storage(OpenXRFbSpatialEntity.STORAGE_CLOUD)

func set_selected(p_selected: bool) -> void:
	selected = p_selected

	var material: StandardMaterial3D = mesh_instance.get_surface_override_material(0)
	if selected:
		material.albedo_color = Color(0.5, 0.5, 0.5)
	else:
		material.albedo_color = color
	mesh_instance.set_surface_override_material(0, material)
