extends Control

@onready var switch_button: Button = %SwitchButton

var hybrid_app_mode := OpenXRHybridApp.HYBRID_MODE_NONE
var data := {}


func _ready() -> void:
	hybrid_app_mode = OpenXRHybridApp.get_mode()
	if hybrid_app_mode == OpenXRHybridApp.HYBRID_MODE_IMMERSIVE:
		switch_button.text = "Switch To Panel"
	elif hybrid_app_mode == OpenXRHybridApp.HYBRID_MODE_PANEL:
		switch_button.text = "Switch To Immersive"
	else:
		switch_button.visible = false


func _on_switch_button_pressed() -> void:
	var data_string := JSON.stringify(data)

	var success := false

	print("Attempting to toggle hybrid app mode with data: ", data_string)

	if hybrid_app_mode == OpenXRHybridApp.HYBRID_MODE_IMMERSIVE:
		success = OpenXRHybridApp.switch_mode(OpenXRHybridApp.HYBRID_MODE_PANEL, data_string)
	elif hybrid_app_mode == OpenXRHybridApp.HYBRID_MODE_PANEL:
		success = OpenXRHybridApp.switch_mode(OpenXRHybridApp.HYBRID_MODE_IMMERSIVE, data_string)

	if not success:
		print("Unable to toggle hybrid app mode.")
