extends Node3D


func _ready():
	var xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface.is_initialized():
		var vp = get_viewport()
		vp.use_xr = true
