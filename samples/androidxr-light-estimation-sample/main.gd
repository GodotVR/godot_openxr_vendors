extends Node3D

const REFLECTIVE_GREY_MATERIAL = preload("res://reflective_grey_material.tres")
const CUSTOM_AMBIENT_MATERIAL = preload("res://custom_ambient_material.tres")

@onready var directional_light: DirectionalLight3D = $DirectionalLight3D
@onready var directional_light_orig_transform = directional_light.transform
@onready var directional_light_orig_color = directional_light.light_color
@onready var directional_light_orig_energy = directional_light.light_energy

@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var environment: Environment = world_environment.environment
@onready var ambient_light_orig_source = environment.ambient_light_source
@onready var ambient_light_orig_color = environment.ambient_light_color
@onready var ambient_light_orig_energy = environment.ambient_light_energy

var openxr_interface: OpenXRInterface
var last_material_update := 0

# Copied from ARCore's hello_ar_kotlin sample.
# See: https://github.com/google-ar/arcore-android-sdk/blob/52c722e43cd8ce546eea5dc4587e70e0c7f2c006/samples/hello_ar_kotlin/app/src/main/java/com/google/ar/core/examples/kotlin/helloar/HelloArRenderer.kt#L60
const SH_FACTORS = [
	0.282095,
	-0.325735,
	0.325735,
	-0.325735,
	0.273137,
	-0.273137,
	0.078848,
	-0.273137,
	0.136569,
]


func _ready() -> void:
	get_tree().on_request_permissions_result.connect(_on_request_permissions_result)

	OS.request_permissions()

	openxr_interface = XRServer.find_interface("OpenXR")
	openxr_interface.session_begun.connect(_on_openxr_session_begun)

	OpenXRAndroidLightEstimationExtensionWrapper.light_estimate_types = OpenXRAndroidLightEstimationExtensionWrapper.LIGHT_ESTIMATE_TYPE_ALL

	var menu: Control = %Viewport2Din3D.get_scene_root()
	menu.directional_light_mode_changed.connect(_on_directional_light_mode_changed)
	menu.ambient_light_mode_changed.connect(_on_ambient_light_mode_changed)


func _on_openxr_session_begun() -> void:
	start_light_estimation()


func start_light_estimation() -> void:
	if not OpenXRAndroidLightEstimationExtensionWrapper.is_light_estimation_supported():
		push_error("Light estimation is unsupported")
		return

	if OpenXRAndroidLightEstimationExtensionWrapper.start_light_estimation():
		print("Light estimation started")
	else:
		push_error("Unable to start light estimation")


func _on_request_permissions_result(p_permission: String, p_granted: bool) -> void:
	if p_permission == "android.permission.SCENE_UNDERSTANDING_COARSE" and p_granted:
		start_light_estimation()


func _on_directional_light_mode_changed(p_mode: int) -> void:
	# Reset the original values.
	directional_light.transform = directional_light_orig_transform
	directional_light.light_color = directional_light_orig_color
	directional_light.light_energy = directional_light_orig_energy

	# Then change the mode.
	$OpenXRAndroidLightEstimation.directional_light_mode = p_mode

	# Goes as far as disabling the directional light if we're not using it.
	%DirectionalLight3D.visible = (p_mode != OpenXRAndroidLightEstimation.DIRECTIONAL_LIGHT_MODE_DISABLED)


func _on_ambient_light_mode_changed(p_mode: int) -> void:
	# Reset the original values.
	environment.ambient_light_source = ambient_light_orig_source
	environment.ambient_light_color = ambient_light_orig_color
	environment.ambient_light_energy = ambient_light_orig_energy

	# Then change the mode.
	if p_mode == 3:
		# Our "custom" mode.
		$OpenXRAndroidLightEstimation.ambient_light_mode = OpenXRAndroidLightEstimation.AMBIENT_LIGHT_MODE_DISABLED
		get_tree().set_group("test_sphere", "surface_material_override/0", CUSTOM_AMBIENT_MATERIAL)
	else:
		$OpenXRAndroidLightEstimation.ambient_light_mode = p_mode
		get_tree().set_group("test_sphere", "surface_material_override/0", REFLECTIVE_GREY_MATERIAL)


func _process(_delta: float) -> void:
	var ale = OpenXRAndroidLightEstimationExtensionWrapper
	if ale.is_light_estimation_started():
		var next_update = ale.get_last_updated_time()
		if next_update > last_material_update and ale.is_spherical_harmonics_total_valid():
			last_material_update = next_update

			var coefficients = ale.get_spherical_harmonics_total_coefficients()
			for i in range(9):
				coefficients[i] = coefficients[i] * SH_FACTORS[i]

			CUSTOM_AMBIENT_MATERIAL.set_shader_parameter("coefficients", coefficients)
			CUSTOM_AMBIENT_MATERIAL.set_shader_parameter("rotation", XRServer.world_origin.basis)
