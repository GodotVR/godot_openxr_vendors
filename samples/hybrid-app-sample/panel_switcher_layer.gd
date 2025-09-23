extends OpenXRCompositionLayerQuad

const PanelSwitcher = preload("res://panel_switcher.gd")

const NO_INTERSECTION := Vector2(-1.0, -1.0)

@onready var panel_switcher: PanelSwitcher = %PanelSwitcher
@onready var cursor: MeshInstance3D = %Cursor

var previous_cursor_intersection: Vector2 = NO_INTERSECTION
var previous_cursor_pressed := false


func get_panel_switcher() -> PanelSwitcher:
	return panel_switcher


func update_pointer(p_origin: Vector3, p_direction: Vector3, p_pressed: bool) -> bool:
	var viewport_size: Vector2 = layer_viewport.size
	var previous_intersection: Vector2 = previous_cursor_intersection

	# We hang on to the cursor until the trigger is released.
	if previous_cursor_pressed:
		if not p_pressed:
			var event := InputEventMouseButton.new()
			event.button_index = MOUSE_BUTTON_LEFT
			event.pressed = false
			event.position = viewport_size * previous_cursor_intersection
			layer_viewport.push_input(event)

			previous_cursor_pressed = false
		return true

	var intersection: Vector2 = intersects_ray(p_origin, p_direction)

	# Update cursor.
	if intersection == NO_INTERSECTION:
		cursor.visible = false
	else:
		cursor.visible = true
		cursor.position.x = (quad_size.x * intersection.x) - (quad_size.x / 2.0)
		cursor.position.y = (quad_size.y * (1.0 - intersection.y)) - (quad_size.y / 2.0)

	if intersection != previous_intersection:
		var event := InputEventMouseMotion.new()
		var from := viewport_size * previous_cursor_intersection
		var to := viewport_size * intersection
		event.relative = to - from
		event.position = to
		layer_viewport.push_input(event)

		previous_cursor_intersection = intersection

	if intersection != NO_INTERSECTION and not previous_cursor_pressed and p_pressed:
		var event := InputEventMouseButton.new()
		event.button_index = MOUSE_BUTTON_LEFT
		event.button_mask = MOUSE_BUTTON_MASK_LEFT
		event.pressed = true
		event.position = viewport_size * intersection
		layer_viewport.push_input(event)

	previous_cursor_pressed = p_pressed

	return intersection != NO_INTERSECTION
