extends OpenXRMlMarkerDetector

@export var marker_display_scene: PackedScene = null
var marker_displays: Dictionary[OpenXRMlMarkerTracker, Node] = {}


func _ready() -> void:
	print("OpenXRMlMarkerUnderstandingExtensionWrapper.is_marker_understanding_supported: ", OpenXRMlMarkerUnderstandingExtensionWrapper.is_marker_understanding_supported())


func _process(delta: float):
	update_marker_displays()


func update_marker_displays():
	var marker_trackers = get_marker_trackers()
	if marker_display_scene != null:
		for marker_tracker in marker_trackers:
			if not marker_displays.has(marker_tracker):
				var display = marker_display_scene.instantiate()
				display.setup_scene(marker_tracker)
				add_child(display)
				marker_displays[marker_tracker] = display

	# Free MarkerDisplays for markers that no longer exist
	for key in marker_displays.keys():
		if key not in marker_trackers:
			marker_displays[key].queue_free()
			marker_displays.erase(key)
