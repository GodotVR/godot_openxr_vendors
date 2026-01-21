extends StartXR

const MeshManager = preload("uid://4xtek5r6bs74")

@onready var xr_origin: XROrigin3D = $XROrigin3D


func _ready():
	super._ready()

	# Check if the permission is already granted.
	var granted_permissions: PackedStringArray = OS.get_granted_permissions()
	for granted_permission in granted_permissions:
		if _permission_granted_main(granted_permission, true):
			return

	# Otherwise, request all permissions and wait until they're granted.
	OS.request_permissions()
	get_tree().on_request_permissions_result.connect(_permission_granted)


func _permission_granted(permission: String, granted: bool):
	# This callback is called on the process's main thread, not Godot's main thread.  Ensure that
	# we do work on Godot's main thread.
	_permission_granted_main.bind(permission, granted).call_deferred()


func _permission_granted_main(permission: String, granted: bool):
	# NOTE: This code intentionally does not handle switching from granted==true, false, true because
	# Android terminates the app when a runtime privilege is revoked.  So it's pointless to add code
	# to handle that scenario.
	if permission != "android.permission.SCENE_UNDERSTANDING_FINE" || !granted:
		return

	# Stop waiting for permissions.
	if get_tree().on_request_permissions_result.is_connected(_permission_granted):
		get_tree().on_request_permissions_result.disconnect(_permission_granted)

	xr_origin.add_child(MeshManager.instantiate())
