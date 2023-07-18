extends Node3D


# Called when the node enters the scene tree for the first time.
func _ready():
	var xr_interface : XRInterface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
