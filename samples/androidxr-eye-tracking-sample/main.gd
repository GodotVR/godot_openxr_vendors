extends StartXR

var hud_content: PackedStringArray


func _ready():
	super._ready()
	hud_content.resize(7)

	_on_left_tracking_changed(false)
	_on_right_tracking_changed(false)

	$XROrigin3D/LeftEyePose.button_pressed.connect(_on_left_button_pressed)
	$XROrigin3D/LeftEyePose.button_released.connect(_on_left_button_released)
	$XROrigin3D/LeftEyePose.tracking_changed.connect(_on_left_tracking_changed)

	$XROrigin3D/RightEyePose.button_pressed.connect(_on_right_button_pressed)
	$XROrigin3D/RightEyePose.button_released.connect(_on_right_button_released)
	$XROrigin3D/RightEyePose.tracking_changed.connect(_on_right_tracking_changed)


func _on_left_tracking_changed(state: bool):
	hud_content[0] = "Left eye: " + ("tracked with valid pose" if state else "not tracked or invalid pose")
	_update_hud()


func _on_left_button_pressed(input: String):
	match input:
		"blink":
			hud_content[1] = "Blinking: yes"
		"_":
			printerr("Unhandled event for LeftEyePose.button_pressed: " + input)
	_update_hud()


func _on_left_button_released(input: String):
	match input:
		"blink":
			hud_content[1] = "Blinking: no"
		"_":
			printerr("Unhandled event for LeftEyePose.button_released: " + input)
	_update_hud()


func _on_right_tracking_changed(state: bool):
	hud_content[4] = "Right eye: " + ("tracked with valid pose" if state else "not tracked or invalid pose")
	_update_hud()


func _on_right_button_pressed(input: String):
	match input:
		"blink":
			hud_content[5] = "Blinking: yes"
		"_":
			printerr("Unhandled event for RightEyePose.button_pressed: " + input)
	_update_hud()


func _on_right_button_released(input: String):
	match input:
		"blink":
			hud_content[5] = "Blinking: no"
		"_":
			printerr("Unhandled event for RightEyePose.button_released: " + input)
	_update_hud()


func _update_hud():
	$XROrigin3D/Hud.text = "\n".join(hud_content)
