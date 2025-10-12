extends Control

@export var display_string: String = ""

@onready var pointer_left: ColorRect = $PointerLeft
@onready var pointer_right: ColorRect = $PointerRight

var start_pos: Vector2


func _ready() -> void:
	start_pos = $Label.position
	$Label.text = display_string


func _process(delta: float) -> void:
	if not "--xrsim-automated-tests" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, we don't want the text
		# to be animated, which can lead to differences in the screenshots.
		$Label.position = Vector2(start_pos.x + (100.0 * sin(Time.get_ticks_msec() * 0.001 * 2)), start_pos.y)


func update_pointers(p_position_left: Vector2, p_position_right: Vector2) -> void:
	pointer_left.position = size * p_position_left - (0.5 * pointer_left.size)
	pointer_right.position = size * p_position_right - (0.5 * pointer_left.size)
