extends StartXR

const PLANE_TRACKABLE_VISUAL = preload("res://plane_trackable_visual/plane_trackable_visual.tscn")

var _plane_visuals_dict: Dictionary


func _ready():
	super._ready()

	OS.request_permissions()
	XRServer.tracker_added.connect(_on_tracker_added)
	XRServer.tracker_removed.connect(_on_tracker_removed)


func _on_tracker_added(tracker_name: StringName, _type: int):
	var tracker: XRTracker = XRServer.get_tracker(tracker_name)

	if not tracker:
		push_error("Couldn't retrieve tracker!")
		return

	if !(tracker is OpenXRAndroidTrackablePlaneTracker):
		# this is okay
		return

	# don't visualize planes that are subsumed by another plane, since the subsuming plane "overrides"
	# the original.  Visualizing every plane is too noisy.
	if tracker.get_subsumed_by_plane():
		return

	var plane_trackable_visual: XRAnchor3D = PLANE_TRACKABLE_VISUAL.instantiate()
	plane_trackable_visual.set_trackable_plane(tracker)
	plane_trackable_visual.subsumed.connect(_on_tracker_removed_impl.bind(tracker_name))
	$XROrigin3D.add_child(plane_trackable_visual)
	_plane_visuals_dict[tracker_name] = plane_trackable_visual


func _on_tracker_removed(tracker_name: StringName, _type: int):
	_on_tracker_removed_impl(tracker_name)


func _on_tracker_removed_impl(tracker_name: StringName):
	if !_plane_visuals_dict.has(tracker_name):
		return

	_plane_visuals_dict[tracker_name].queue_free()
	_plane_visuals_dict.erase(tracker_name)
