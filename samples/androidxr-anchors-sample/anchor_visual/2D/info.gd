extends PanelContainer


func set_title(title: String):
	$VBoxContainer/Title.text = title


func set_persisted_state_string(persisted_state_text: String):
	$VBoxContainer/Persisted.text = persisted_state_text


func set_location_flags(location_flags: OpenXRAndroidAnchorTracker.LocationFlags):
	var append := ""
	var flag := 1
	while flag <= location_flags:
		if 0 != (flag & location_flags):
			if !append.is_empty():
				append += "|"
			append += _get_location_flag_as_string(flag)
		flag = flag << 1

	if append.is_empty():
		append = "(none)"

	$VBoxContainer/LocationFlags.text = "LocationFlags=" + append


func set_position_text(new_position: Vector3):
	$VBoxContainer/Position.text = "Position: " + str(new_position)


func _get_location_flag_as_string(location_flag: OpenXRAndroidAnchorTracker.LocationFlags) -> String:
	match location_flag:
		OpenXRAndroidAnchorTracker.LOCATION_FLAGS_ORIENTATION_VALID:
			return "ORIENTATION_VALID"
		OpenXRAndroidAnchorTracker.LOCATION_FLAGS_POSITION_VALID:
			return "POSITION_VALID"
		OpenXRAndroidAnchorTracker.LOCATION_FLAGS_ORIENTATION_TRACKED:
			return "ORIENTATION_TRACKED"
		OpenXRAndroidAnchorTracker.LOCATION_FLAGS_POSITION_TRACKED:
			return "POSITION_TRACKED"

	return "(unknown flag)"
