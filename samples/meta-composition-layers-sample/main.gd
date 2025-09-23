extends StartXR

const TURN_THRESHOLD := 0.8
const TURN_ANGLE := TAU / 16

@onready var xr_origin: XROrigin3D = $XROrigin3D
@onready var xr_camera: XRCamera3D = $XROrigin3D/XRCamera3D
@onready var left_controller: XRController3D = $XROrigin3D/LeftController
@onready var right_controller: XRController3D = $XROrigin3D/RightController
@onready var left_controller_ray_cast: RayCast3D = $XROrigin3D/LeftController/LeftControllerRayCast
@onready var right_controller_ray_cast: RayCast3D = $XROrigin3D/RightController/RightControllerRayCast
@onready var turn_timer: Timer = $XROrigin3D/TurnTimer

@onready var composition_layer_quad: OpenXRCompositionLayerQuad = $XROrigin3D/OpenXRCompositionLayerQuad
@onready var composition_layer_cylinder: OpenXRCompositionLayerCylinder = $XROrigin3D/OpenXRCompositionLayerCylinder
@onready var composition_layer_equirect: OpenXRCompositionLayerEquirect = $XROrigin3D/OpenXRCompositionLayerEquirect
@onready var quad_panel: Control = $QuadViewport/ExamplePanel
@onready var clylinder_panel: Control = $CylinderViewport/ExamplePanel
@onready var equirect_panel: Control = $EquirectViewport/ExamplePanel

@onready var settings_composition_layer: OpenXRCompositionLayerCylinder = $XROrigin3D/SettingsCompositionLayer
@onready var secure_content_composition_layer: OpenXRCompositionLayerCylinder = $XROrigin3D/SecureContentCompositionLayer
@onready var disabled_supersampling_mesh: MeshInstance3D = $XROrigin3D/SettingsCompositionLayer/DisabledSupersampling/MeshInstance3D
@onready var normal_supersampling_mesh: MeshInstance3D = $XROrigin3D/SettingsCompositionLayer/NormalSupersampling/MeshInstance3D
@onready var quality_supersampling_mesh: MeshInstance3D = $XROrigin3D/SettingsCompositionLayer/QualitySupersampling/MeshInstance3D
@onready var disabled_sharpening_mesh: MeshInstance3D = $XROrigin3D/SettingsCompositionLayer/DisabledSharpening/MeshInstance3D
@onready var normal_sharpening_mesh: MeshInstance3D = $XROrigin3D/SettingsCompositionLayer/NormalSharpening/MeshInstance3D
@onready var quality_sharpening_mesh: MeshInstance3D = $XROrigin3D/SettingsCompositionLayer/QualitySharpening/MeshInstance3D
@onready var display_secure_content_mesh: MeshInstance3D = $XROrigin3D/SecureContentCompositionLayer/DisplaySecureContent/MeshInstance3D
@onready var replace_secure_content_mesh: MeshInstance3D = $XROrigin3D/SecureContentCompositionLayer/ReplaceSecureContent/MeshInstance3D
@onready var exclude_secure_content_mesh: MeshInstance3D = $XROrigin3D/SecureContentCompositionLayer/ExcludeSecureContent/MeshInstance3D


func _ready():
	super._ready()

	for composition_layer in get_tree().get_nodes_in_group("composition_layer"):
		var node_3d := Node3D.new()
		node_3d.global_transform = composition_layer.global_transform
		node_3d.name = composition_layer.name
		add_child(node_3d)


func _process(_delta: float) -> void:
	var ray_intersection_left: Vector2
	var ray_intersection_right: Vector2

	ray_intersection_right = composition_layer_cylinder.intersects_ray(right_controller_ray_cast.global_transform.origin, -right_controller_ray_cast.global_transform.basis.y)
	ray_intersection_left = composition_layer_cylinder.intersects_ray(left_controller_ray_cast.global_transform.origin, -left_controller_ray_cast.global_transform.basis.y)
	clylinder_panel.update_pointers(ray_intersection_left, ray_intersection_right)

	ray_intersection_right = composition_layer_equirect.intersects_ray(right_controller_ray_cast.global_transform.origin, -right_controller_ray_cast.global_transform.basis.y)
	ray_intersection_left = composition_layer_equirect.intersects_ray(left_controller_ray_cast.global_transform.origin, -left_controller_ray_cast.global_transform.basis.y)
	equirect_panel.update_pointers(ray_intersection_left, ray_intersection_right)

	ray_intersection_right = composition_layer_quad.intersects_ray(right_controller_ray_cast.global_transform.origin, -right_controller_ray_cast.global_transform.basis.y)
	ray_intersection_left = composition_layer_quad.intersects_ray(left_controller_ray_cast.global_transform.origin, -left_controller_ray_cast.global_transform.basis.y)
	quad_panel.update_pointers(ray_intersection_left, ray_intersection_right)


func _on_left_controller_button_pressed(name: String) -> void:
	if name == "trigger_click" and left_controller_ray_cast.is_colliding():
		var collider = left_controller_ray_cast.get_collider()
		update(collider.name)


func _on_right_hand_button_pressed(name: String) -> void:
	if name == "trigger_click" and right_controller_ray_cast.is_colliding():
		var collider = right_controller_ray_cast.get_collider()
		update(collider.name)


func _on_left_controller_input_vector_2_changed(name: String, value: Vector2) -> void:
	check_turn(name, value)


func _on_right_controller_input_vector_2_changed(name: String, value: Vector2) -> void:
	check_turn(name, value)


func update(button_name: String) -> void:
	if button_name.contains("Supersampling"):
		disabled_supersampling_mesh.get_surface_override_material(0).albedo_color = Color.RED
		normal_supersampling_mesh.get_surface_override_material(0).albedo_color = Color.RED
		quality_supersampling_mesh.get_surface_override_material(0).albedo_color = Color.RED
	elif button_name.contains("Sharpening"):
		disabled_sharpening_mesh.get_surface_override_material(0).albedo_color = Color.RED
		normal_sharpening_mesh.get_surface_override_material(0).albedo_color = Color.RED
		quality_sharpening_mesh.get_surface_override_material(0).albedo_color = Color.RED
	elif button_name.contains("SecureContent"):
		display_secure_content_mesh.get_surface_override_material(0).albedo_color = Color.RED
		replace_secure_content_mesh.get_surface_override_material(0).albedo_color = Color.RED
		exclude_secure_content_mesh.get_surface_override_material(0).albedo_color = Color.RED

	match button_name:
		"DisabledSupersampling":
			settings_composition_layer.set("XR_FB_composition_layer_settings/manual/supersampling_mode", OpenXRFbCompositionLayerSettingsExtensionWrapper.SUPERSAMPLING_MODE_DISABLED)
			disabled_supersampling_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"NormalSupersampling":
			settings_composition_layer.set("XR_FB_composition_layer_settings/manual/supersampling_mode", OpenXRFbCompositionLayerSettingsExtensionWrapper.SUPERSAMPLING_MODE_NORMAL)
			normal_supersampling_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"QualitySupersampling":
			settings_composition_layer.set("XR_FB_composition_layer_settings/manual/supersampling_mode", OpenXRFbCompositionLayerSettingsExtensionWrapper.SUPERSAMPLING_MODE_QUALITY)
			quality_supersampling_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"DisabledSharpening":
			settings_composition_layer.set("XR_FB_composition_layer_settings/manual/sharpening_mode", OpenXRFbCompositionLayerSettingsExtensionWrapper.SHARPENING_MODE_DISABLED)
			disabled_sharpening_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"NormalSharpening":
			settings_composition_layer.set("XR_FB_composition_layer_settings/manual/sharpening_mode", OpenXRFbCompositionLayerSettingsExtensionWrapper.SHARPENING_MODE_NORMAL)
			normal_sharpening_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"QualitySharpening":
			settings_composition_layer.set("XR_FB_composition_layer_settings/manual/sharpening_mode", OpenXRFbCompositionLayerSettingsExtensionWrapper.SHARPENING_MODE_QUALITY)
			quality_sharpening_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"DisplaySecureContent":
			secure_content_composition_layer.set("XR_FB_composition_layer_secure_content/external_output", 0)
			display_secure_content_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"ReplaceSecureContent":
			secure_content_composition_layer.set("XR_FB_composition_layer_secure_content/external_output", 2)
			replace_secure_content_mesh.get_surface_override_material(0).albedo_color = Color.GREEN
		"ExcludeSecureContent":
			secure_content_composition_layer.set("XR_FB_composition_layer_secure_content/external_output", 1)
			exclude_secure_content_mesh.get_surface_override_material(0).albedo_color = Color.GREEN


func check_turn(name: String, value: Vector2) -> void:
	if not turn_timer.is_stopped():
		return

	if name == "primary":
		if value.x > TURN_THRESHOLD:
			rotate_player(TURN_ANGLE)
			turn_timer.start()
		if value.x < -TURN_THRESHOLD:
			rotate_player(-TURN_ANGLE)
			turn_timer.start()


func rotate_player(angle: float):
	var t1 := Transform3D()
	var t2 := Transform3D()
	var rot := Transform3D()

	t1.origin = -xr_camera.transform.origin
	t2.origin = xr_camera.transform.origin
	rot = rot.rotated(Vector3.DOWN, angle)
	xr_origin.transform = (xr_origin.transform * t2 * rot * t1).orthonormalized()

	for composition_layer in get_tree().get_nodes_in_group("composition_layer"):
		composition_layer.global_transform = get_node(NodePath(composition_layer.name)).global_transform
