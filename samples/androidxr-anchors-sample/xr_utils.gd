extends Object


static func get_anchor_tracker_string(anchor_tracker: OpenXRAndroidAnchorTracker) -> String:
	if anchor_tracker == null:
		return "No anchor tracker"

	var tracker: OpenXRAndroidTrackableTracker = anchor_tracker.get_tracker()
	if tracker is OpenXRAndroidTrackablePlaneTracker:
		return "Plane Anchor"
	elif tracker is OpenXRAndroidTrackableObjectTracker:
		# do these exist?
		return "Object Anchor"
	return "Spatial Anchor"


static func get_persist_state_string_from_uuid(uuid: StringName) -> String:
	return get_persist_state_string(get_persist_state(uuid))


static func get_persist_state(uuid: StringName) -> OpenXRAndroidAnchorTracker.PersistState:
	if uuid.is_empty():
		return OpenXRAndroidAnchorTracker.PERSIST_STATE_NOT_REQUESTED
	return OpenXRAndroidDeviceAnchorPersistenceExtension.get_anchor_tracker_persist_state_uuid(uuid)


static func get_persist_state_string(persisted_state: OpenXRAndroidAnchorTracker.PersistState) -> String:
	match persisted_state:
		OpenXRAndroidAnchorTracker.PERSIST_STATE_NOT_REQUESTED:
			return "Unpersisted"
		OpenXRAndroidAnchorTracker.PERSIST_STATE_PENDING:
			return "Persisted pending"
		OpenXRAndroidAnchorTracker.PERSIST_STATE_PERSISTED:
			return "Persisted"
		OpenXRAndroidAnchorTracker.PERSIST_STATE_ERROR:
			return "Persisted error"

	return "(invalid persist state)"
