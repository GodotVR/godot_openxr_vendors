extends Node3D

@onready var description: Label3D = $Description

func update_from_data(data: OpenXRMlMarkerData):
	var desc = ""
	desc += "Type: " + str(data.get_marker_type()) + "\n"
	desc += "Reprojection Error: " + str(data.get_reprojection_error_meters()) + " m\n"
	desc += "Length: " + str(data.get_marker_length()) + " m\n"
	desc += "Number: " + str(data.get_marker_number()) + "\n"
	desc += "String: " + data.get_marker_string() + "\n"
	desc += "Tracking Confidence: " + str(data.get_marker_pose().tracking_confidence)
	description.text = desc
	transform = data.get_marker_pose().transform
