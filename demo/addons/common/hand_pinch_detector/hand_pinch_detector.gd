@tool
extends Node3D

## Signal emitted when pinch was tapped[br]
## This signal is not emitted if [signal pinch_held] was emitted some time after pinch started
signal pinch_tapped

## Signal emitted when pinch was held[br]
## This signal is not emitted if [signal pinch_tapped] was emitted some time after pinch started
signal pinch_held

## Signal emitted when pinch is released[br]
## This signal is always emitted some time after pinch started, and always after [signal
## pinch_tapped] or [signal pinch_held] if they were emitted too.[br]
signal pinch_released

## The maximum time, in milliseconds, to detect pinch-then-release (or "pinch tap")
@export var pinch_tap_duration := 300:
	set = set_pinch_tap_duration

## The minimum time, in seconds, to detect pinch-then-hold; must be > pinch_tap_duration
@export var pinch_held_duration := 0.8:
	set = set_pinch_held_duration

# true when fingers are pinched
var _pinching := false

# true when fingers have been pinched for a long time
var _pinching_held := false

# the timestamp when [param _pinching] changed from false->true
var _timestamp_when_pinch_detected := 0

# the parent controller
var _controller: XRController3D

# This Timer is started when _pinching changes from false->true, to detect if fingers have been
# pinched for a long time.
# Must use a Timer because pinch values are not very constant, as in, if you hold pinch for a
# while, you'll only get one value (1.0), rather than 1.0 every frame.
@onready var _pinching_held_timer: Timer = $Timer


func _enter_tree() -> void:
	# NOTE: as of this writing, assume the immediate parent is the XRController3D
	# Ideally this would be XRHelpers.get_xr_controller(self) like XRTools's
	# function_pointer.gd (see
	# https://github.com/GodotVR/godot-xr-tools/blob/master/addons/godot-xr-tools/functions/function_pointer.gd)
	var parent_node = get_parent()
	if !(parent_node is XRController3D):
		push_error("Unable to find XRController3D; it must be the immediate parent of this node!")
		return

	_controller = parent_node
	_controller.input_float_changed.connect(_on_input_float_changed)


func _exit_tree() -> void:
	_pinching_held_timer.stop()

	# the _controller is still valid, even if the _controller is in the process of being deleted
	# (because child nodes are free'd before parent nodes)
	if _controller:
		_controller.input_float_changed.disconnect(_on_input_float_changed)
		_controller = null


func set_pinch_tap_duration(new_pinch_tap_duration: int) -> void:
	if (pinch_held_duration * 1000.0) <= float(new_pinch_tap_duration):
		push_error("Pinch tap duration must be < pinch held duration")
		return

	pinch_tap_duration = new_pinch_tap_duration


func set_pinch_held_duration(new_pinch_held_duration: float) -> void:
	if (new_pinch_held_duration * 1000.0) <= float(pinch_tap_duration):
		push_error("Pinch held duration must be > pinch tap duration")
		return

	pinch_held_duration = new_pinch_held_duration


func _on_input_float_changed(action_name: String, value: float) -> void:
	if action_name != "pinch_value":
		return

	if !_pinching:
		# started pinching?
		if 0.9 < value:
			_pinching = true
			_timestamp_when_pinch_detected = Time.get_ticks_msec()
			_pinching_held_timer.start(pinch_held_duration)

		return

	# stopped pinching?
	if value < 0.5:
		_pinching = false
		_pinching_held_timer.stop()

		# don't emit pinch-tap if pinch-held was already emitted
		if _pinching_held:
			_pinching_held = false
		# emit pinch-tap if was released fast enough
		elif Time.get_ticks_msec() - _timestamp_when_pinch_detected < pinch_tap_duration:
			pinch_tapped.emit()

		pinch_released.emit()


func _on_timer_timeout() -> void:
	_pinching_held = true
	pinch_held.emit()
