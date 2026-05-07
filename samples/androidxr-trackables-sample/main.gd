extends StartXR


func _ready():
	super._ready()

	OS.request_permissions()
