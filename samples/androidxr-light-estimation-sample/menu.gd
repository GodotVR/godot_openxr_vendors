extends Control

signal directional_light_mode_changed(mode: int)
signal ambient_light_mode_changed(mode: int)
signal spherical_harmonics_degree_changed(degree: int)


func _ready() -> void:
	for i in range(%DirectionalLightContainer.get_child_count()):
		var btn: Button = %DirectionalLightContainer.get_child(i)
		btn.pressed.connect(emit_signal.bind("directional_light_mode_changed", i))

	for i in range(%AmbientLightContainer.get_child_count()):
		var btn: Button = %AmbientLightContainer.get_child(i)
		btn.pressed.connect(emit_signal.bind("ambient_light_mode_changed", i))

	for i in range(%SphericalHarmonicsContainer.get_child_count()):
		var btn: Button = %SphericalHarmonicsContainer.get_child(i)
		btn.pressed.connect(emit_signal.bind("spherical_harmonics_degree_changed", i))
