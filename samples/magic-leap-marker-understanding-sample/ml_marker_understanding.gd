extends Node

@export var marker_display_scene: PackedScene = null
@export var settings: OpenXRMlMarkerDetectorSettings = null
var marker_detector: OpenXRMlMarkerDetector = null
var marker_displays: Dictionary = {}

func _ready() -> void:
	print("OpenXRMlMarkerUnderstandingExtensionWrapper.is_marker_understanding_supported: ", OpenXRMlMarkerUnderstandingExtensionWrapper.is_marker_understanding_supported())
	marker_detector = OpenXRMlMarkerDetector.create_detector(settings)

func _process(delta: float):
	update_marker_displays()

func update_marker_displays():
	if marker_detector == null:
		return

	var marker_data = marker_detector.get_marker_data()
	if marker_display_scene != null:
		for marker in marker_data:
			if not marker_displays.has(marker):
				var display = marker_display_scene.instantiate()
				add_child(display)
				marker_displays[marker] = display
			marker_displays[marker].update_from_data(marker)

	# Free MarkerDisplays for markers that no longer exist
	for key in marker_displays.keys():
		if key not in marker_data:
			marker_displays[key].queue_free()
			marker_displays.erase(key)
