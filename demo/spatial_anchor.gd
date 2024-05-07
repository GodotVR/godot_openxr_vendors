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

func _on_saved_to_cloud(p_success: bool, p_location: OpenXRFbSpatialEntity.StorageLocation, p_spatial_entity: OpenXRFbSpatialEntity) -> void:
	print("Saved ", p_spatial_entity.uuid, " to cloud: ", p_success)

	if p_success:
		p_spatial_entity.openxr_fb_spatial_entity_set_component_enabled_completed.connect(self._on_enable_sharable.bind(p_spatial_entity))
		p_spatial_entity.set_component_enabled(OpenXRFbSpatialEntity.COMPONENT_TYPE_SHARABLE, true)

func _on_enable_sharable(p_success: bool, p_component: OpenXRFbSpatialEntity.StorageLocation, p_enabled: bool, p_spatial_entity: OpenXRFbSpatialEntity) -> void:
	print("Enable ", p_spatial_entity.uuid, " as sharable: ", p_success)

	if p_success:
		# Note: For this to really work, we'd need a real user ID from the platform SDK.
		#       This is outside the scope of what we can do in the demo here.
		var spatial_entity_user = OpenXRFbSpatialEntityUser.create_user(1234)

		p_spatial_entity.openxr_fb_spatial_entity_shared.connect(self._on_shared.bind(p_spatial_entity))
		# Note: Uncomment to test sharing.
		#p_spatial_entity.share_with_users([ spatial_entity_user ])

func _on_shared(p_success: bool, p_spatial_entity: OpenXRFbSpatialEntity) -> void:
	print("Share ", p_spatial_entity.uuid, " is success: ", p_success)
