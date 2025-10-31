extends StartXR


func _ready():
	super._ready()
	$XROrigin3D/Hud.data_source = _get_passthrough_camera_state


func _get_passthrough_camera_state() -> String:
	var text := "Passthrough Camera State: "
	match OpenXRAndroidPassthroughCameraStateExtensionWrapper.get_passthrough_camera_state():
		OpenXRAndroidPassthroughCameraStateExtensionWrapper.PASSTHROUGH_CAMERA_STATE_DISABLED:
			text += "DISABLED"
		OpenXRAndroidPassthroughCameraStateExtensionWrapper.PASSTHROUGH_CAMERA_STATE_INITIALIZING:
			text += "INITIALIZING"
		OpenXRAndroidPassthroughCameraStateExtensionWrapper.PASSTHROUGH_CAMERA_STATE_READY:
			text += "READY"
		OpenXRAndroidPassthroughCameraStateExtensionWrapper.PASSTHROUGH_CAMERA_STATE_ERROR:
			text += "ERROR"
	return text
