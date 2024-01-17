extends Node3D

var xr_interface : XRInterface = null

# Called when the node enters the scene tree for the first time.
func _ready():
	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true

	var scene_capture := Engine.get_singleton("OpenXRFbSceneCaptureExtensionWrapper")
	if scene_capture:
		scene_capture.connect("scene_capture_completed", _on_scene_capture_completed)


func _on_scene_capture_completed():
	print("Scene Capture completed")


func _on_left_hand_button_pressed(name):
	var scene_capture := Engine.get_singleton("OpenXRFbSceneCaptureExtensionWrapper")
	if name == "menu_button" and scene_capture:
		print("Triggering scene capture")
		scene_capture.request_scene_capture()
