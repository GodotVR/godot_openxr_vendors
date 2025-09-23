extends Node3D

@onready var skeleton := $Armature/Skeleton3D


func _physics_process(delta: float) -> void:
	if not "--xrsim-automated-tests" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, we don't want this
		# to be animated, which can lead to differences in the screenshots.
		var new_basis := basis.rotated(Vector3.FORWARD, TAU * (Time.get_ticks_msec() / 2000.0))
		var quat := Quaternion(new_basis)
		skeleton.set_bone_pose_rotation(0, quat)
