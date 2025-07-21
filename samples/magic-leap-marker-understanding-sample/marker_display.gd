extends Node3D
class_name MarkerDisplay

@onready var description: Label3D = $Description
var tracker_entity: OpenXRMlMarkerTracker


func setup_scene(p_tracker: OpenXRMlMarkerTracker):
	tracker_entity = p_tracker


func _process(deltaTime):
	var desc = ""
	desc += "Type: " + str(tracker_entity.get_marker_type()) + "\n"
	desc += "Reprojection Error: " + str(tracker_entity.get_reprojection_error_meters()) + " m\n"
	desc += "Length: " + str(tracker_entity.get_marker_length()) + " m\n"
	desc += "Number: " + str(tracker_entity.get_marker_number()) + "\n"
	desc += "String: " + tracker_entity.get_marker_string() + "\n"
	desc += "Tracking Confidence: " + str(tracker_entity.get_pose("default").tracking_confidence)
	description.text = desc
