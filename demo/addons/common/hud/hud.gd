extends Label3D
class_name Hud

## The distance to place this hud from the camera, in meters
@export var distance_to_camera := 5.0

## Number of seconds to wait before calling [param data_source]
@export var update_delay := 1.0

## The function to call after [param update_delay] seconds[br]
## The function takes zero args and should return a [String]
var data_source: Callable


func _ready() -> void:
	_call_data_source()

	if update_delay <= 0.0:
		print("WARNING: cannot set a delay <= 0.0; using 0.1 instead")
		update_delay = 0.1

	var timer := Timer.new()
	timer.autostart = true
	timer.wait_time = update_delay
	timer.timeout.connect(_call_data_source)
	add_child(timer)


func _process(_delta: float) -> void:
	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()

	var forward := Vector3(camera_transform.basis.z.x, 0.0, camera_transform.basis.z.z).normalized()

	# Vector3.UP.cross(forward)
	var right := Vector3(forward.z, 0.0, -forward.x)

	var camera_transform_upright := Transform3D(right, Vector3.UP, forward, camera_transform.origin)

	# keep the label centered, regardless of horizontal alignment
	var xcoord := 0.0
	match horizontal_alignment:
		HORIZONTAL_ALIGNMENT_LEFT:
			xcoord = -get_aabb().size.x / 2.0
		HORIZONTAL_ALIGNMENT_RIGHT:
			xcoord = get_aabb().size.x / 2.0

	var target_transform: Transform3D = camera_transform_upright.translated_local(Vector3(xcoord, 0.0, -distance_to_camera))
	global_transform = global_transform.interpolate_with(target_transform.looking_at(camera_transform_upright.origin, Vector3.UP, true), 0.1)


func _call_data_source() -> void:
	if data_source:
		var value = data_source.call()
		if value is String:
			text = value
