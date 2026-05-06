extends XRAnchor3D

var _object_tracker: OpenXRAndroidTrackableObjectTracker
var _label: Label3D
var _extents: MeshInstance3D


# manually billboard the label since the billboard flags don't look quite right
func _process(delta: float) -> void:
	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()
	var forward: Vector3 = (camera_transform.origin - _label.global_position).normalized()
	var right: Vector3 = Vector3.UP.cross(forward).normalized()
	var up: Vector3 = forward.cross(right)
	_label.global_transform.basis = Basis(right, up, forward)


func set_object_tracker(new_object_tracker: OpenXRAndroidTrackableObjectTracker):
	# currently only handle one object tracker
	if _object_tracker:
		push_error("Expect exactly one call to each set_object_tracker; why was this called twice?")
		return

	_object_tracker = new_object_tracker
	_label = $Label3D
	_extents = $Extents

	# Set this XRAnchor3D's tracker to the XRTracker's name to receive automatic pose updates
	# (the XRTracker updates the "default" pose, which happens to be our default pose name too)
	tracker = _object_tracker.get_tracker_name()

	match _object_tracker.get_object_label():
		OpenXRAndroidTrackableObjectTracker.OBJECT_LABEL_UNKNOWN:
			_label.text = "OBJECT_LABEL_UNKNOWN\n"
		OpenXRAndroidTrackableObjectTracker.OBJECT_LABEL_KEYBOARD:
			_label.text = "OBJECT_LABEL_KEYBOARD\n"
		OpenXRAndroidTrackableObjectTracker.OBJECT_LABEL_MOUSE:
			_label.text = "OBJECT_LABEL_MOUSE\n"
		OpenXRAndroidTrackableObjectTracker.OBJECT_LABEL_LAPTOP:
			_label.text = "OBJECT_LABEL_LAPTOP\n"
		_:
			_label.text = "(bad object label)\n"

	new_object_tracker.updated.connect(_update)
	_update()


func _update():
	_extents.mesh.size = _object_tracker.get_extents()

	# keep the object label string since it never changes. Discard everything else
	var object_label_str: String = _label.text.split("\n", false, 1)[0] + "\n"

	match _object_tracker.get_tracking_state():
		OpenXRAndroidTrackableTracker.TRACKING_STATE_PAUSED:
			object_label_str += "TRACKING_STATE_PAUSED\n"
		OpenXRAndroidTrackableTracker.TRACKING_STATE_STOPPED:
			object_label_str += "TRACKING_STATE_STOPPED\n"
		OpenXRAndroidTrackableTracker.TRACKING_STATE_TRACKING:
			object_label_str += "TRACKING_STATE_TRACKING\n"
		OpenXRAndroidTrackableTracker.TRACKING_STATE_UNKNOWN:
			object_label_str += "TRACKING_STATE_UNKNOWN\n"
		_:
			object_label_str += "(bad tracking state)\n"

	object_label_str += "Extents: " + str(_object_tracker.get_extents()) + "\n"
	object_label_str += "Position: " + str(_object_tracker.get_center_pose().origin)

	_label.text = object_label_str
