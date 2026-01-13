extends Control

@onready var label: Label = %Label


func set_label_text(p_text: String) -> void:
	label.text = p_text
