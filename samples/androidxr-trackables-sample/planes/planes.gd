extends Node3D

const PLANE_TRACKABLE_VISUAL = preload("res://planes/plane_trackable_visual/plane_trackable_visual.tscn")

var _visuals_dict: Dictionary


func _ready():
	XRServer.tracker_added.connect(_on_tracker_added)
	XRServer.tracker_removed.connect(_on_tracker_removed)


func _on_tracker_added(tracker_name: StringName, _type: int):
	var tracker: XRTracker = XRServer.get_tracker(tracker_name)

	if not tracker:
		push_error("Couldn't retrieve tracker!")
		return

	var trackable_visual: XRAnchor3D
	if tracker is OpenXRAndroidTrackablePlaneTracker:
		# don't visualize planes that are subsumed by another plane, since the subsuming plane
		# "overrides" the original.  Visualizing every plane is too noisy.
		if tracker.get_subsumed_by_plane():
			return

		trackable_visual = PLANE_TRACKABLE_VISUAL.instantiate()
		trackable_visual.set_plane_tracker(tracker)

		# similar to the comment above; we only want to visualize the subsuming plane, so we remove the
		# original plane when it is subsumed.
		trackable_visual.subsumed.connect(_on_tracker_removed_impl.bind(tracker_name))
	else:
		return

	add_child(trackable_visual)
	_visuals_dict[tracker_name] = trackable_visual


func _on_tracker_removed(tracker_name: StringName, _type: int):
	_on_tracker_removed_impl(tracker_name)


func _on_tracker_removed_impl(tracker_name: StringName):
	if !_visuals_dict.has(tracker_name):
		return

	_visuals_dict[tracker_name].queue_free()
	_visuals_dict.erase(tracker_name)
