extends StartXR

const TRANSFORM3D_UTILS = preload("res://transform3d_utils.gd")


func _ready():
	super._ready()

	OS.request_permissions()

	XRServer.get_tracker("head").pose_changed.connect(_wait_for_camera)


func _wait_for_camera(pose: XRPose):
	if pose.name != &"default":
		return

	if pose.transform == Transform3D.IDENTITY:
		# still waiting
		return

	# stop waiting for the camera
	XRServer.get_tracker("head").pose_changed.disconnect(_wait_for_camera)

	$XROrigin3D/Raycast.initialize()
	$XROrigin3D/Anchors.initialize()


func _process(_delta: float) -> void:
	# position the menu in front of the camera, when the camera's UP vector is Vector3.UP

	var camera_transform: Transform3D = $XROrigin3D/XRCamera3D.get_camera_transform()

	# apply an offset to the camera, when the camera is upright
	var menu_transform: Transform3D = TRANSFORM3D_UTILS.upright_then_offset(camera_transform, Vector3(0.0, -0.7, -2.0))

	# make Menu3D look at the camera
	$XROrigin3D/Menu3D.global_transform = menu_transform.looking_at(camera_transform.origin, Vector3.UP, true)
