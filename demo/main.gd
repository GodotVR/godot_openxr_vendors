extends Node3D

var xr_interface: XRInterface = null


# Called when the node enters the scene tree for the first time.
func _ready():
	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.initialize():
		xr_interface.session_stopping.connect(self._on_session_stopping)
		var vp: Viewport = get_viewport()
		vp.use_xr = true


func _on_session_stopping() -> void:
	if "--xrsim-automated-tests" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, it will end the OpenXR
		# session automatically, and in that case, we want to quit.
		get_tree().quit()
