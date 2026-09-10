extends StartXR

const BOUNDS_MODES_SWITCH_TIME = 10.0  # seconds

var supported_bounds_modes: Array
var current_bounds_mode_index := 0

@onready var godot_robot: Node3D = $GodotRobot

var initial_godot_robot_scale: Vector3
var bounds_to_scale_ratio: Vector3


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	super()

	initial_godot_robot_scale = godot_robot.scale
	# Update the bounds_to_scale_ratio value
	var initial_spatial_container_bounds = ProjectSettings.get_setting_with_override("xr/openxr/extensions/spatial_container/bounds")
	bounds_to_scale_ratio = initial_godot_robot_scale / initial_spatial_container_bounds
	print("Bounds to scale ratio: " + str(bounds_to_scale_ratio))

	OpenXRSpatialContainerExtension.spatial_container_bounds_changed.connect(_on_spatial_container_bounds_changed)
	OpenXRSpatialContainerExtension.spatial_container_bounds_mode_request_denied.connect(_on_spatial_container_bounds_mode_request_denied)


func _on_bounds_modes_timer_timeout() -> void:
	# Select the next bounds mode to switch to.
	var next_bounds_mode_index := (current_bounds_mode_index + 1) % supported_bounds_modes.size()
	var next_bounds_mode = supported_bounds_modes[next_bounds_mode_index]
	print("Switching to bounds mode: " + str(next_bounds_mode))
	OpenXRSpatialContainerExtension.request_spatial_container_bounds_mode(next_bounds_mode)

	current_bounds_mode_index = next_bounds_mode_index

	# Restart the timer to transition back and forth between bounded and immersive mode.
	get_tree().create_timer(BOUNDS_MODES_SWITCH_TIME).timeout.connect(_on_bounds_modes_timer_timeout)


func _on_spatial_container_bounds_changed(_spatial_container_rid: RID, infinite_bounds: bool, bounds_mode: OpenXRSpatialContainerState.BoundsMode, bounds: Vector3) -> void:
	print("Spatial container bounds changed:")
	print("Bounds mode: " + str(bounds_mode))
	if not infinite_bounds:
		print("Bounds: " + str(bounds))
		# Update the scale based on the bounds.
		godot_robot.scale = bounds * bounds_to_scale_ratio
	else:
		# Update the scale for immersive mode.
		godot_robot.scale = Vector3.ONE


func _on_spatial_container_bounds_mode_request_denied(_spatial_container_rid: RID) -> void:
	print("Spatial container bounds mode request denied!")


func _on_openxr_session_begun() -> void:
	super()
	_start_bounds_modes_timer()


func _start_bounds_modes_timer() -> void:
	supported_bounds_modes = OpenXRSpatialContainerExtension.get_supported_bounds_modes()
	if not supported_bounds_modes.is_empty():
		print("Supported bounds modes: " + str(supported_bounds_modes))

		# Get the current bounds mode.
		var spatial_container_state := OpenXRSpatialContainerExtension.get_spatial_container_state()
		var current_bounds_mode := spatial_container_state.get_bounds_mode()
		current_bounds_mode_index = supported_bounds_modes.find(current_bounds_mode)
		print("Current bounds mode: " + str(current_bounds_mode))
		print("Current bounds mode index: " + str(current_bounds_mode_index))

		# Start a timer to transition back and forth between bounded and immersive mode.
		get_tree().create_timer(BOUNDS_MODES_SWITCH_TIME).timeout.connect(_on_bounds_modes_timer_timeout)
	else:
		print("No supported bounds modes!")
