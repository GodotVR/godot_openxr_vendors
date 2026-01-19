extends Control

@onready var info_label: Label = %InfoLabel

signal eye_mode_changed(eye_mode: String)


func set_info_label_text(p_text: String) -> void:
	info_label.text = p_text


func _on_eye_gaze_interaction_button_pressed() -> void:
	eye_mode_changed.emit("XR_EXT_eye_gaze_interaction")


func _on_android_eye_tracking_button_pressed() -> void:
	eye_mode_changed.emit("XR_ANDROID_eye_tracking")
