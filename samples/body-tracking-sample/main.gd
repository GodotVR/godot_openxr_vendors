extends StartXR

@onready var viewport_2d_in_3d = %Viewport2Din3D
@onready var eye_gaze: XRController3D = %EyeGaze
@onready var android_xr_left_eye: XRController3D = %AndroidXRLeftEye
@onready var android_xr_right_eye: XRController3D = %AndroidXRRightEye

var eye_mode: String = "XR_EXT_eye_gaze_interaction"


func _ready() -> void:
	super._ready()

	viewport_2d_in_3d.get_scene_root().eye_mode_changed.connect(_on_eye_mode_changed)


func _process(_delta: float) -> void:
	if eye_mode == "XR_EXT_eye_gaze_interaction":
		var text: String = "Has tracking data!" if eye_gaze.get_has_tracking_data() else "No tracking data."
		viewport_2d_in_3d.get_scene_root().set_info_label_text(text)

	elif eye_mode == "XR_ANDROID_eye_tracking":
		var lines: PackedStringArray
		lines.push_back("Left eye: " + _get_android_xr_eye_info(android_xr_left_eye))
		lines.push_back("Right eye: " + _get_android_xr_eye_info(android_xr_right_eye))
		var text: String = "\n\n".join(lines)
		viewport_2d_in_3d.get_scene_root().set_info_label_text(text)


func _get_android_xr_eye_info(p_eye: XRController3D) -> String:
	var text: String = "Has tracking data" if p_eye.get_has_tracking_data() else "No tracking data"
	text += "\nBlinking: "
	text += "yes" if p_eye.is_button_pressed("blink") else "no"
	return text


func _on_eye_mode_changed(p_mode: String) -> void:
	eye_mode = p_mode

	if eye_mode == "XR_EXT_eye_gaze_interaction":
		eye_gaze.show_when_tracked = true
		android_xr_left_eye.visible = false
		android_xr_left_eye.show_when_tracked = false
		android_xr_right_eye.visible = false
		android_xr_right_eye.show_when_tracked = false

	elif eye_mode == "XR_ANDROID_eye_tracking":
		eye_gaze.show_when_tracked = false
		eye_gaze.visible = false
		android_xr_left_eye.show_when_tracked = true
		android_xr_right_eye.show_when_tracked = true
