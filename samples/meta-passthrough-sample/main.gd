extends StartXR

@export var color_map: Gradient
@export var mono_map: Curve
@export var brightness_contrast_saturation: Vector3
@export var color_lut: Image
@export var color_lut2: Image
@export var edge_color: Color

var fb_passthrough
var meta_color_lut: OpenXRMetaPassthroughColorLut
var meta_color_lut2: OpenXRMetaPassthroughColorLut
var countdown_to_recenter_hmd: int = 3

@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var left_controller_ray_cast: RayCast3D = $XROrigin3D/LeftController/LeftControllerRayCast
@onready var right_controller_ray_cast: RayCast3D = $XROrigin3D/RightController/RightControllerRayCast
@onready var passthrough_geometry: OpenXRFbPassthroughGeometry = $XROrigin3D/OpenXRFbPassthroughGeometry
@onready var interfaces: Node3D = $Interfaces
@onready var interface_color_map: Node3D = $Interfaces/InterfaceColorMap
@onready var interface_mono_map: Node3D = $Interfaces/InterfaceMonoMap
@onready var interface_brightness_contrast_saturation: Node3D = $Interfaces/InterfaceBrightnessContrastSaturation
@onready var interface_color_map_lut: Node3D = $Interfaces/InterfaceColorMapLut
@onready var interface_color_map_interpolated_lut: Node3D = $Interfaces/InterfaceColorMapInterpolatedLut
@onready var interface_disabled: Node3D = $Interfaces/InterfaceDisabled
@onready var color_map_mesh: MeshInstance3D = $Interfaces/InterfaceColorMap/ColorMapMesh
@onready var mono_map_mesh: MeshInstance3D = $Interfaces/InterfaceMonoMap/MonoMapMesh


func _ready() -> void:
	super._ready()

	passthrough_geometry.hide()
	enable_mode_full()

	fb_passthrough = Engine.get_singleton("OpenXRFbPassthroughExtensionWrapper")
	fb_passthrough.set_color_map(color_map)
	fb_passthrough.set_mono_map(mono_map)
	fb_passthrough.set_brightness_contrast_saturation(brightness_contrast_saturation.x, brightness_contrast_saturation.y, brightness_contrast_saturation.z)
	meta_color_lut = OpenXRMetaPassthroughColorLut.create_from_image(color_lut, OpenXRMetaPassthroughColorLut.COLOR_LUT_CHANNELS_RGB)
	fb_passthrough.set_color_lut(0.5, meta_color_lut)
	meta_color_lut2 = OpenXRMetaPassthroughColorLut.create_from_image(color_lut2, OpenXRMetaPassthroughColorLut.COLOR_LUT_CHANNELS_RGB)
	fb_passthrough.set_interpolated_color_lut(0.5, meta_color_lut, meta_color_lut2)
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_DISABLED)

	var color_map_mat := color_map_mesh.get_surface_override_material(0)
	var color_map_gradient_texture = GradientTexture2D.new()
	color_map_gradient_texture.gradient = color_map
	color_map_mat.albedo_texture = color_map_gradient_texture

	var curve_texture := CurveTexture.new()
	curve_texture.curve = mono_map
	var mono_map_mat := mono_map_mesh.get_surface_override_material(0)
	mono_map_mat.albedo_texture = curve_texture


func _process(_delta: float) -> void:
	if countdown_to_recenter_hmd > 0:
		countdown_to_recenter_hmd -= 1
		if countdown_to_recenter_hmd == 0:
			XRServer.center_on_hmd(XRServer.RESET_BUT_KEEP_TILT, true)


func enable_mode_full() -> void:
	get_viewport().transparent_bg = true
	world_environment.environment.background_mode = Environment.BG_COLOR
	world_environment.environment.background_color = Color(0.0, 0.0, 0.0, 0.0)
	xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
	passthrough_geometry.hide()


func enable_mode_geometry() -> void:
	get_viewport().transparent_bg = true
	world_environment.environment.background_mode = Environment.BG_COLOR
	world_environment.environment.background_color = Color(0.3, 0.3, 0.3, 0.0)
	xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
	passthrough_geometry.enable_hole_punch = false
	passthrough_geometry.show()


func enable_mode_geometry_hp() -> void:
	get_viewport().transparent_bg = false
	world_environment.environment.background_mode = Environment.BG_SKY
	xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
	passthrough_geometry.enable_hole_punch = true
	passthrough_geometry.show()


func enable_filter_color_map() -> void:
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_COLOR_MAP)
	interface_color_map.show()


func enable_filter_mono_map() -> void:
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_MONO_MAP)
	interface_mono_map.show()


func enable_filter_brightness_contrast_saturation() -> void:
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION)
	interface_brightness_contrast_saturation.show()


func enable_filter_color_map_lut() -> void:
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_COLOR_MAP_LUT)
	interface_color_map_lut.show()


func enable_filter_color_map_interpolated_lut() -> void:
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT)
	interface_color_map_interpolated_lut.show()


func disable_filters() -> void:
	fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_DISABLED)
	interface_disabled.show()


func _on_left_controller_button_pressed(name: String) -> void:
	if name == "trigger_click" and left_controller_ray_cast.is_colliding():
		var collider = left_controller_ray_cast.get_collider()
		update(collider.name)

		if collider is ValueSlider:
			collider.update_value(left_controller_ray_cast.get_collision_point())
		else:
			update(collider.name)


func _on_right_controller_button_pressed(name: String) -> void:
	if name == "trigger_click" and right_controller_ray_cast.is_colliding():
		var collider = right_controller_ray_cast.get_collider()

		if collider is ValueSlider:
			collider.update_value(right_controller_ray_cast.get_collision_point())
		else:
			update(collider.name)


func update(to_update: String) -> void:
	if to_update.contains("Filter"):
		for child in interfaces.get_children():
			child.hide()

	match to_update:
		"ModeFull":
			enable_mode_full()
		"ModeGeometry":
			enable_mode_geometry()
		"ModeGeometryHP":
			enable_mode_geometry_hp()
		"FilterColorMap":
			enable_filter_color_map()
		"FilterMonoMap":
			enable_filter_mono_map()
		"FilterBrightnessContrastSaturation":
			enable_filter_brightness_contrast_saturation()
		"FilterColorMapLUT":
			enable_filter_color_map_lut()
		"FilterColorMapInterpolatedLUT":
			enable_filter_color_map_interpolated_lut()
		"FilterDisabled":
			disable_filters()


func _on_brightness_new_value(value: float) -> void:
	brightness_contrast_saturation.x = value
	fb_passthrough.set_brightness_contrast_saturation(brightness_contrast_saturation.x, brightness_contrast_saturation.y, brightness_contrast_saturation.z)


func _on_contrast_new_value(value: float) -> void:
	brightness_contrast_saturation.y = value
	fb_passthrough.set_brightness_contrast_saturation(brightness_contrast_saturation.x, brightness_contrast_saturation.y, brightness_contrast_saturation.z)


func _on_saturation_new_value(value: float) -> void:
	brightness_contrast_saturation.z = value
	fb_passthrough.set_brightness_contrast_saturation(brightness_contrast_saturation.x, brightness_contrast_saturation.y, brightness_contrast_saturation.z)


func _on_color_map_lut_weight_new_value(value: float) -> void:
	fb_passthrough.set_color_lut(value, meta_color_lut)


func _on_color_map_interpolated_lut_weight_new_value(value: float) -> void:
	fb_passthrough.set_interpolated_color_lut(value, meta_color_lut, meta_color_lut2)


func _on_edge_r_new_value(value: float) -> void:
	edge_color.r = value
	fb_passthrough.set_edge_color(edge_color)


func _on_edge_g_new_value(value: float) -> void:
	edge_color.g = value
	fb_passthrough.set_edge_color(edge_color)


func _on_edge_b_new_value(value: float) -> void:
	edge_color.b = value
	fb_passthrough.set_edge_color(edge_color)


func _on_edge_a_new_value(value: float) -> void:
	edge_color.a = value
	fb_passthrough.set_edge_color(edge_color)


func _on_opacity_new_value(value: float) -> void:
	fb_passthrough.set_texture_opacity_factor(value)
