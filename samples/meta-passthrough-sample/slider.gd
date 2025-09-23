extends StaticBody3D
class_name ValueSlider

signal new_value(value: float)

@export var min_value: float = 0.0
@export var max_value: float = 1.0

# Only used for starting display, has no impact on filter values
@export var start_value: float = 0.5

var extent: float
var value: float

@onready var collision_shape_3d: CollisionShape3D = $CollisionShape3D
@onready var value_indicator: MeshInstance3D = $ValueIndicator


func _ready() -> void:
	extent = (collision_shape_3d.shape.size.x * 0.5) * 0.9
	value_indicator.position.x = remap(start_value, 0.0, 1.0, -extent, extent)

	visibility_changed.connect(_on_visibility_changed)
	if not get_parent().visible:
		process_mode = PROCESS_MODE_DISABLED


func update_value(location_global: Vector3) -> void:
	var location_local = to_local(location_global)
	value_indicator.position.x = clamp(location_local.x, -extent, extent)
	value = remap(value_indicator.position.x, -extent, extent, min_value, max_value)

	new_value.emit(value)


func _on_visibility_changed() -> void:
	if not get_parent().visible:
		process_mode = PROCESS_MODE_DISABLED
	else:
		process_mode = PROCESS_MODE_INHERIT
