extends Node

const TRANSFORM3D_UTILS = preload("res://transform3d_utils.gd")
const XR_UTILS = preload("res://xr_utils.gd")

const EDIT_ANCHOR_TRACKERS = "EDIT_ANCHOR_TRACKERS"
const VIEW_PERSISTED_ANCHORS = "VIEW_PERSISTED_ANCHORS"
const CHANGE_PLAY_AREA = "CHANGE_PLAY_AREA"
const EDIT_ANCHOR_TRACKER = "EDIT_ANCHOR_TRACKER"
const EDIT_PERSISTED_ANCHOR = "EDIT_PERSISTED_ANCHOR"

var _menu3d: Menu3D
var _menu2d: Menu2D


func _ready() -> void:
	set_process(false)


func _process(_delta: float) -> void:
	# _process() only runs when trying to find a plane trackable for a new anchor tracker

	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()
	var trackable_types_to_hit := [OpenXRAndroidRaycastExtension.TRACKABLE_TYPE_PLANE]
	var hitresults: Array[OpenXRAndroidHitResult] = OpenXRAndroidRaycastExtension.raycast(trackable_types_to_hit, camera_transform.origin, -camera_transform.basis.z, 1)
	if hitresults.is_empty():
		# this is okay; try again next frame
		return

	# sanity
	if hitresults.size() != 1 || !(hitresults[0].get_tracker() is OpenXRAndroidTrackablePlaneTracker):
		printerr("got a hit result, but expected 1 hit and for it to be a non-null plane trackable; got [%s] [%s]" % [hitresults.size(), hitresults[0].get_tracker()])
		return

	var anchor_tracker_pose_upright: Transform3D = TRANSFORM3D_UTILS.uprighted(hitresults[0].get_pose())
	_create_anchor_tracker(anchor_tracker_pose_upright, hitresults[0].get_tracker())

	# stop trying to create an anchor tracker for a plane trackable until we're told to again
	set_process(false)


# Callback from "menu3d_group"
func update_menu(menu3d: Menu3D, menu_stack_item: Menu3D.MenuStackItem):
	if _menu3d == null || _menu2d == null:
		_menu3d = menu3d
		_menu2d = menu3d.get_menu2d()

	if _menu3d == null || _menu2d == null:
		push_error("Invalid menu3d or menu2d")
		return

	match menu_stack_item.current_menu:
		Menu3D.MENU3D_MAIN:
			_menu2d.add_choice("create anchor tracker in front of camera", _create_anchor_tracker_in_front_of_camera)

			# _process() is only used for plane trackables, so we can check the state of "process" instead
			# of dedicating a new boolean field
			var prefix: String
			if is_processing():
				prefix = "stop: "
			else:
				prefix = "start: "
			_menu2d.add_choice(prefix + "create anchor tracker for next plane trackable in front of camera", _create_anchor_tracker_for_next_plane_trackable_in_front_of_camera.bind(!is_processing()))

			if !OpenXRAndroidTrackablesExtension.get_anchor_trackers().is_empty():
				_menu2d.add_choice("edit anchor trackers", menu3d.push_menu.bind(EDIT_ANCHOR_TRACKERS))

			if !OpenXRAndroidDeviceAnchorPersistenceExtension.get_all_persisted_anchors().is_empty():
				_menu2d.add_choice("view persisted anchors", menu3d.push_menu.bind(VIEW_PERSISTED_ANCHORS))

			_menu2d.add_choice("change play area", menu3d.push_menu.bind(CHANGE_PLAY_AREA))

		EDIT_ANCHOR_TRACKERS:
			var anchor_trackers: Array[OpenXRAndroidAnchorTracker] = OpenXRAndroidTrackablesExtension.get_anchor_trackers()
			if anchor_trackers.is_empty():
				_menu3d.back()
				return

			# skip this menu (EDIT_ANCHOR_TRACKERS) if we only have one anchor tracker
			# (only when adding to the menu stack, since we don't want to skip this menu when going from 2
			# to 1 anchors)
			if !_menu3d.popped_backwards() && anchor_trackers.size() == 1:
				# remove this menu (EDIT_ANCHOR_TRACKER) from the stack so we don't pop back to it later
				menu3d.discard_current_menu()

				# and go to the next menu
				menu3d.push_menu(EDIT_ANCHOR_TRACKER, anchor_trackers[0])
				return

			_menu2d.set_title_text("Choose an anchor tracker to edit")
			_menu2d.add_choice("back", menu3d.back)

			for anchor_tracker in anchor_trackers:
				var text = XR_UTILS.get_anchor_tracker_string(anchor_tracker) + " " + XR_UTILS.get_persist_state_string_from_uuid(anchor_tracker.get_persist_uuid()) + " " + _get_location_string(anchor_tracker)
				_menu2d.add_choice(text, menu3d.push_menu.bind(EDIT_ANCHOR_TRACKER, anchor_tracker))

		VIEW_PERSISTED_ANCHORS:
			var persisted_anchors: Array[StringName] = OpenXRAndroidDeviceAnchorPersistenceExtension.get_all_persisted_anchors()
			if persisted_anchors.is_empty():
				_menu3d.back()
				return

			# skip this menu (VIEW_PERSISTED_ANCHORS) if we only have one persisted anchor
			# (only when adding to the menu stack, since we don't want to skip this menu when going from 2
			# to 1 persisted anchors)
			if !_menu3d.popped_backwards() && persisted_anchors.size() == 1:
				# remove this menu (VIEW_PERSISTED_ANCHORS) from the stack so we don't pop back to it later
				menu3d.discard_current_menu()

				# and go to the next menu
				menu3d.push_menu(EDIT_PERSISTED_ANCHOR, persisted_anchors[0])
				return

			_menu2d.set_title_text("Choose a persisted anchor")
			_menu2d.add_choice("back", _menu3d.back)
			var anchor_trackers: Array[OpenXRAndroidAnchorTracker] = OpenXRAndroidTrackablesExtension.get_anchor_trackers()
			for persisted_anchor in persisted_anchors:
				var append := _get_persisted_anchor_tracker_location_string(persisted_anchor, ": ", anchor_trackers)
				_menu2d.add_choice(persisted_anchor + append, menu3d.push_menu.bind(EDIT_PERSISTED_ANCHOR, persisted_anchor))

		CHANGE_PLAY_AREA:
			var xr_interface: XRInterface = XRServer.find_interface("OpenXR")
			var current_play_area_mode: XRInterface.PlayAreaMode = xr_interface.get_play_area_mode()
			var append: String = _get_play_area_mode_string(current_play_area_mode)
			if append.is_empty():
				printerr("Unexpected play area received from XRServer; going back")
				_menu3d.back()
				return

			_menu2d.set_title_text("Change play area; current play area is: " + append)
			_menu2d.add_choice("back", _menu3d.back)

			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_3DOF)
			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_SITTING)
			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_ROOMSCALE)
			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_STAGE)

		EDIT_ANCHOR_TRACKER:
			var anchor_tracker: OpenXRAndroidAnchorTracker = menu_stack_item.arg1
			var uuid: StringName = anchor_tracker.get_persist_uuid()
			_menu2d.set_title_text("Modifying %s:\n  %s\n  %s" % [XR_UTILS.get_anchor_tracker_string(anchor_tracker), XR_UTILS.get_persist_state_string_from_uuid(uuid), _get_location_string(anchor_tracker)])
			_menu2d.add_choice("back", _menu3d.back)

			if uuid.is_empty():
				_menu2d.add_choice("persist this anchor tracker", _persist_anchor_tracker.bind(anchor_tracker))
			else:
				_menu2d.add_choice("unpersist anchor tracker", _unpersist_anchor_tracker.bind(uuid, true))

			_menu2d.add_choice("destroy this anchor tracker", _destroy_anchor_tracker.bind(anchor_tracker))

		EDIT_PERSISTED_ANCHOR:
			var persisted_anchor: StringName = menu_stack_item.arg1
			var append := _get_persisted_anchor_tracker_location_string(persisted_anchor, "\n  ", OpenXRAndroidTrackablesExtension.get_anchor_trackers())

			_menu2d.set_title_text("Modifying persisted anchor\n  " + persisted_anchor + append)

			_menu2d.add_choice("back", _menu3d.back)

			if append.is_empty():
				_menu2d.add_choice("create anchor tracker from this persisted anchor", _create_anchor_tracker_from_persisted_anchor.bind(persisted_anchor))

			_menu2d.add_choice("unpersist this anchor", _unpersist_anchor_tracker.bind(persisted_anchor, false))


func _create_anchor_tracker_in_front_of_camera():
	var anchor_tracker_pose_upright: Transform3D = TRANSFORM3D_UTILS.offset_then_uprighted(get_tree().root.get_camera_3d().get_camera_transform(), Vector3(0.0, 0.0, -0.5))
	_create_anchor_tracker(anchor_tracker_pose_upright, null)


func _create_anchor_tracker(anchor_tracker_pose: Transform3D, trackable: OpenXRAndroidTrackableTracker):
	var anchor_tracker: OpenXRAndroidAnchorTracker = OpenXRAndroidTrackablesExtension.create_anchor_tracker(anchor_tracker_pose, trackable)
	if anchor_tracker == null:
		printerr("failed to create an anchor tracker")
		return

	_menu3d.refresh()


func _create_anchor_tracker_for_next_plane_trackable_in_front_of_camera(start_looking: bool):
	set_process(start_looking)
	_menu3d.refresh()


func _create_anchor_tracker_from_persisted_anchor(persisted_anchor: StringName):
	var anchor_tracker: OpenXRAndroidAnchorTracker = OpenXRAndroidDeviceAnchorPersistenceExtension.create_persisted_anchor_tracker(persisted_anchor)
	if anchor_tracker == null:
		printerr("failed to create anchor tracker from persisted anchor")
		return

	_menu3d.back()


func _unpersist_anchor_tracker(persisted_anchor: StringName, refresh_current_menu: bool):
	OpenXRAndroidDeviceAnchorPersistenceExtension.unpersist_anchor_uuid(persisted_anchor)

	if refresh_current_menu:
		_menu3d.refresh()
	else:
		_menu3d.back()


func _persist_anchor_tracker(anchor_tracker: OpenXRAndroidAnchorTracker):
	if !OpenXRAndroidDeviceAnchorPersistenceExtension.persist_anchor_tracker(anchor_tracker):
		printerr("failed to persist anchor tracker")
		return

	_menu3d.refresh()


func _destroy_anchor_tracker(anchor_tracker: OpenXRAndroidAnchorTracker):
	OpenXRAndroidTrackablesExtension.destroy_anchor_tracker(anchor_tracker)

	_menu3d.back()


func _try_to_add_play_area_choice(current_play_area_mode: XRInterface.PlayAreaMode, play_area_mode: XRInterface.PlayAreaMode):
	if current_play_area_mode != play_area_mode:
		_menu2d.add_choice(_get_play_area_mode_string(play_area_mode), _switch_to_play_area.bind(play_area_mode))


func _get_play_area_mode_string(play_area_mode: XRInterface.PlayAreaMode) -> String:
	match play_area_mode:
		XRInterface.XR_PLAY_AREA_3DOF:
			return "3DOF"
		XRInterface.XR_PLAY_AREA_SITTING:
			return "SITTING"
		XRInterface.XR_PLAY_AREA_ROOMSCALE:
			return "ROOMSCALE"
		XRInterface.XR_PLAY_AREA_STAGE:
			return "STAGE"

	return "(unknown)"


func _switch_to_play_area(play_area_mode: XRInterface.PlayAreaMode):
	XRServer.find_interface("OpenXR").set_play_area_mode(play_area_mode)
	_menu3d.back()


func _get_persisted_anchor_tracker_location_string(persisted_anchor: StringName, append: String, anchor_trackers: Array[OpenXRAndroidAnchorTracker]) -> String:
	var idx := anchor_trackers.find_custom(func(ele: OpenXRAndroidAnchorTracker) -> bool: return ele.get_persist_uuid() == persisted_anchor)
	if idx == -1:
		return ""
	return append + _get_location_string(anchor_trackers[idx])


func _get_location_string(anchor_tracker: OpenXRAndroidAnchorTracker) -> String:
	return "" if anchor_tracker == null else str(anchor_tracker.get_location_pose().origin)
