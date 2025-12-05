extends StartXR

var smooth: bool = false


func _ready():
	super._ready()

	OS.request_permissions()

	_update()


func _on_timer_timeout() -> void:
	_update()


func _update() -> void:
	if !OpenXRAndroidEnvironmentDepthExtensionWrapper.is_environment_depth_started():
		if !OpenXRAndroidEnvironmentDepthExtensionWrapper.start_environment_depth():
			push_error("Unable to start environment depth")
			$Timer.queue_free()
		smooth = false
	elif smooth:
		OpenXRAndroidEnvironmentDepthExtensionWrapper.stop_environment_depth()
	else:
		smooth = true

	if !OpenXRAndroidEnvironmentDepthExtensionWrapper.is_environment_depth_started():
		$XROrigin3D/XRCamera3D/Viewport2Din3D.get_scene_root().text = "Depth disabled"
		return

	var label_text: String = "Depth enabled"
	if OpenXRAndroidEnvironmentDepthExtensionWrapper.set_smooth(smooth):
		if smooth:
			label_text += "\nSmooth"
		else:
			label_text += "\nNot smooth"
	else:
		# NOTE: this label may be misleading if we've never changed smoothness (so 'smooth' is out of
		# sync with the actual smoothness setting)
		smooth = !smooth
		label_text += "\nUnable to change smooth; smooth is still %s" % smooth
	$XROrigin3D/XRCamera3D/Viewport2Din3D.get_scene_root().text = label_text
