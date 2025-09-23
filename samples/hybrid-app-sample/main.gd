extends StartXR

const PanelSwitcherLayerScene = preload("res://panel_switcher_layer.tscn")
const PanelSwitcherLayer = preload("res://panel_switcher_layer.gd")
const PanelSwitcherScene = preload("res://panel_switcher.tscn")
const PanelSwitcher = preload("res://panel_switcher.gd")

var panel_switcher: PanelSwitcher
var panel_switcher_layer: PanelSwitcherLayer
var pointer_pressed := false

@onready var turkey = %Turkey
@onready var right_controller_pointer: XRController3D = %RightControllerPointer


func _ready() -> void:
	var data_string: String = OpenXRHybridApp.get_launch_data()
	if data_string != "":
		var data: Dictionary = JSON.parse_string(data_string)
		print("Hybrid App Launch Data: ", data)

		# Restore the turkey's rotation.
		var turkey_rotation := Vector3(0.0, data.get("turkey_y_rotation", 0.0), 0.0)
		turkey.transform.basis = Basis.from_euler(turkey_rotation)

	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		# Only call StartXR._ready() when we know that OpenXR is initialized, because it'll
		# quit the whole app if it isn't.
		super._ready()

		# Enable passthrough
		xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
		get_viewport().transparent_bg = true

		panel_switcher_layer = PanelSwitcherLayerScene.instantiate()
		add_child(panel_switcher_layer)
		panel_switcher_layer.global_transform = %PanelSwitcherMarker.global_transform
		panel_switcher = panel_switcher_layer.get_panel_switcher()

	else:
		panel_switcher = PanelSwitcherScene.instantiate()
		add_child(panel_switcher)

	print("Is Hybrid App: ", OpenXRHybridApp.is_hybrid_app())
	print("Hybrid App Mode: ", OpenXRHybridApp.get_mode())


func _physics_process(_delta: float) -> void:
	# Make the gltf model slowly rotate
	if turkey:
		turkey.rotate_y(0.003)

		# Store the data in the panel switcher, so the turkey rotation is maintained in the other mode.
		var turkey_rotation: Vector3 = turkey.transform.basis.get_euler()
		panel_switcher.data["turkey_y_rotation"] = turkey_rotation.y


func _process(_delta: float) -> void:
	if panel_switcher_layer:
		var t: Transform3D = right_controller_pointer.global_transform
		panel_switcher_layer.update_pointer(t.origin, -t.basis.z, pointer_pressed)


func _on_right_controller_pointer_button_pressed(p_name: String) -> void:
	if p_name == "trigger_click":
		pointer_pressed = true


func _on_right_controller_pointer_button_released(p_name: String) -> void:
	if p_name == "trigger_click":
		pointer_pressed = false
