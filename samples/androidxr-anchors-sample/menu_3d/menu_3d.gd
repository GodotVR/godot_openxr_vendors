extends Node3D

enum MenuType {
	FIRST,
	EDIT_ANCHOR_TRACKERS,
	VIEW_PERSISTED_ANCHORS,
	CHANGE_PLAY_AREA,
	EDIT_ANCHOR_TRACKER,
	EDIT_PERSISTED_ANCHOR,
}


class MenuStackItem:
	var menu_type: MenuType

	# Instead of introducing many MenuStackItem-types, we instead assign to generic args and let each
	# MenuType determine which args to use (or none at all)
	var arg1

	static func create(new_type: MenuType, new_arg1):
		var new_menu_stack_item := MenuStackItem.new()
		new_menu_stack_item.menu_type = new_type
		new_menu_stack_item.arg1 = new_arg1
		return new_menu_stack_item


const TRANSFORM3D_UTILS = preload("res://transform3d_utils.gd")
const XR_UTILS = preload("res://xr_utils.gd")

# an [Array] of [class MenuStackItem]s
var _menu_stack := []

var _menu: Control

var _popped_backwards: bool = false


func _ready() -> void:
	_menu = $Viewport2Din3D.get_scene_root()
	_menu.resized.connect(_on_hud_resized)

	_push_menu(MenuType.FIRST)

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


func on_pinch_tapped(hand: OpenXRHand.Hands):
	$Viewport2Din3D.get_scene_root().on_pinch_tapped(hand)


func _push_menu(menu_type: MenuType, arg1 = null):
	_push_menu_impl(MenuStackItem.create(menu_type, arg1), false)


func _push_menu_impl(menu_stack_item: MenuStackItem, refreshed: bool):
	_menu_stack.push_back(menu_stack_item)
	_menu.clear_choices()

	# keep _popped_backwards state until the current menu is not refreshed
	_popped_backwards = _popped_backwards && refreshed

	match menu_stack_item.menu_type:
		MenuType.FIRST:
			_menu.set_title_text("Main menu\nPinch left hand to move highlight to the next row\nPinch right hand to choose the highlighted row")
			_menu.add_choice("create anchor tracker in front of camera", _create_anchor_tracker_in_front_of_camera)

			# _process() is only used for plane trackables, so we can check the state of "process" instead
			# of dedicating a new boolean field
			var prefix: String
			if is_processing():
				prefix = "stop: "
			else:
				prefix = "start: "
			_menu.add_choice(prefix + "create anchor tracker for next plane trackable in front of camera", _create_anchor_tracker_for_next_plane_trackable_in_front_of_camera.bind(!is_processing()))

			if !OpenXRAndroidTrackablesExtension.get_anchor_trackers().is_empty():
				_menu.add_choice("edit anchor trackers", _push_menu.bind(MenuType.EDIT_ANCHOR_TRACKERS))

			if !OpenXRAndroidDeviceAnchorPersistenceExtension.get_all_persisted_anchors().is_empty():
				_menu.add_choice("view persisted anchors", _push_menu.bind(MenuType.VIEW_PERSISTED_ANCHORS))

			_menu.add_choice("change play area", _push_menu.bind(MenuType.CHANGE_PLAY_AREA))

		MenuType.EDIT_ANCHOR_TRACKERS:
			var anchor_trackers: Array[OpenXRAndroidAnchorTracker] = OpenXRAndroidTrackablesExtension.get_anchor_trackers()
			if anchor_trackers.is_empty():
				_back()
				return

			# skip this menu (EDIT_ANCHOR_TRACKERS) if we only have one anchor tracker
			# (only when adding to the menu stack, since we don't want to skip this menu when going from 2
			# to 1 anchors)
			if !_popped_backwards && anchor_trackers.size() == 1:
				# remove this menu (EDIT_ANCHOR_TRACKER) from the stack so we don't pop back to it later
				_menu_stack.pop_back()

				# and go to the next menu
				_push_menu(MenuType.EDIT_ANCHOR_TRACKER, anchor_trackers[0])
				return

			_menu.set_title_text("Choose an anchor tracker to edit")
			_menu.add_choice("back", _back)

			for anchor_tracker in anchor_trackers:
				var text = XR_UTILS.get_anchor_tracker_string(anchor_tracker) + " " + XR_UTILS.get_persist_state_string_from_uuid(anchor_tracker.get_persist_uuid()) + " " + _get_location_string(anchor_tracker)
				_menu.add_choice(text, _push_menu.bind(MenuType.EDIT_ANCHOR_TRACKER, anchor_tracker))

		MenuType.VIEW_PERSISTED_ANCHORS:
			var persisted_anchors: Array[StringName] = OpenXRAndroidDeviceAnchorPersistenceExtension.get_all_persisted_anchors()
			if persisted_anchors.is_empty():
				_back()
				return

			# skip this menu (VIEW_PERSISTED_ANCHORS) if we only have one persisted anchor
			# (only when adding to the menu stack, since we don't want to skip this menu when going from 2
			# to 1 persisted anchors)
			if !_popped_backwards && persisted_anchors.size() == 1:
				# remove this menu (VIEW_PERSISTED_ANCHORS) from the stack so we don't pop back to it later
				_menu_stack.pop_back()

				# and go to the next menu
				_push_menu(MenuType.EDIT_PERSISTED_ANCHOR, persisted_anchors[0])
				return

			_menu.set_title_text("Choose a persisted anchor")
			_menu.add_choice("back", _back)
			var anchor_trackers: Array[OpenXRAndroidAnchorTracker] = OpenXRAndroidTrackablesExtension.get_anchor_trackers()
			for persisted_anchor in persisted_anchors:
				var append := _get_persisted_anchor_tracker_location_string(persisted_anchor, ": ", anchor_trackers)
				_menu.add_choice(persisted_anchor + append, _push_menu.bind(MenuType.EDIT_PERSISTED_ANCHOR, persisted_anchor))

		MenuType.CHANGE_PLAY_AREA:
			var xr_interface: XRInterface = XRServer.find_interface("OpenXR")
			var current_play_area_mode: XRInterface.PlayAreaMode = xr_interface.get_play_area_mode()
			var append: String = _get_play_area_mode_string(current_play_area_mode)
			if append.is_empty():
				printerr("Unexpected play area received from XRServer; going back")
				_back()
				return

			_menu.set_title_text("Change play area; current play area is: " + append)
			_menu.add_choice("back", _back)

			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_3DOF)
			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_SITTING)
			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_ROOMSCALE)
			_try_to_add_play_area_choice(current_play_area_mode, XRInterface.XR_PLAY_AREA_STAGE)

		MenuType.EDIT_ANCHOR_TRACKER:
			var anchor_tracker: OpenXRAndroidAnchorTracker = menu_stack_item.arg1
			var uuid: StringName = anchor_tracker.get_persist_uuid()
			_menu.set_title_text("Modifying %s:\n  %s\n  %s" % [XR_UTILS.get_anchor_tracker_string(anchor_tracker), XR_UTILS.get_persist_state_string_from_uuid(uuid), _get_location_string(anchor_tracker)])
			_menu.add_choice("back", _back)

			if uuid.is_empty():
				_menu.add_choice("persist this anchor tracker", _persist_anchor_tracker.bind(anchor_tracker))
			else:
				_menu.add_choice("unpersist anchor tracker", _unpersist_anchor_tracker.bind(uuid, true))

			_menu.add_choice("destroy this anchor tracker", _destroy_anchor_tracker.bind(anchor_tracker))

		MenuType.EDIT_PERSISTED_ANCHOR:
			var persisted_anchor: StringName = menu_stack_item.arg1
			var append := _get_persisted_anchor_tracker_location_string(persisted_anchor, "\n  ", OpenXRAndroidTrackablesExtension.get_anchor_trackers())

			_menu.set_title_text("Modifying persisted anchor\n  " + persisted_anchor + append)

			_menu.add_choice("back", _back)

			if append.is_empty():
				_menu.add_choice("create anchor tracker from this persisted anchor", _create_anchor_tracker_from_persisted_anchor.bind(persisted_anchor))

			_menu.add_choice("unpersist this anchor", _unpersist_anchor_tracker.bind(persisted_anchor, false))

		_:
			printerr("Unhandled menu type!")
			_back()
			return


func _create_anchor_tracker_in_front_of_camera():
	var anchor_tracker_pose_upright: Transform3D = TRANSFORM3D_UTILS.offset_then_uprighted(get_tree().root.get_camera_3d().get_camera_transform(), Vector3(0.0, 0.0, -0.5))
	_create_anchor_tracker(anchor_tracker_pose_upright, null)


func _create_anchor_tracker(anchor_tracker_pose: Transform3D, trackable: OpenXRAndroidTrackableTracker):
	var anchor_tracker: OpenXRAndroidAnchorTracker = OpenXRAndroidTrackablesExtension.create_anchor_tracker(anchor_tracker_pose, trackable)
	if anchor_tracker == null:
		printerr("failed to create an anchor tracker")
		return

	_refresh()


func _create_anchor_tracker_for_next_plane_trackable_in_front_of_camera(start_looking: bool):
	set_process(start_looking)
	_refresh()


func _create_anchor_tracker_from_persisted_anchor(persisted_anchor: StringName):
	var anchor_tracker: OpenXRAndroidAnchorTracker = OpenXRAndroidDeviceAnchorPersistenceExtension.create_persisted_anchor_tracker(persisted_anchor)
	if anchor_tracker == null:
		printerr("failed to create anchor tracker from persisted anchor")
		return

	_back()


func _unpersist_anchor_tracker(persisted_anchor: StringName, refresh_current_menu: bool):
	OpenXRAndroidDeviceAnchorPersistenceExtension.unpersist_anchor_uuid(persisted_anchor)

	if refresh_current_menu:
		_refresh()
	else:
		_back()


func _persist_anchor_tracker(anchor_tracker: OpenXRAndroidAnchorTracker):
	if !OpenXRAndroidDeviceAnchorPersistenceExtension.persist_anchor_tracker(anchor_tracker):
		printerr("failed to persist anchor tracker")
		return

	_refresh()


func _destroy_anchor_tracker(anchor_tracker: OpenXRAndroidAnchorTracker):
	OpenXRAndroidTrackablesExtension.destroy_anchor_tracker(anchor_tracker)

	_back()


func _try_to_add_play_area_choice(current_play_area_mode: XRInterface.PlayAreaMode, play_area_mode: XRInterface.PlayAreaMode):
	if current_play_area_mode != play_area_mode:
		_menu.add_choice(_get_play_area_mode_string(play_area_mode), _switch_to_play_area.bind(play_area_mode))


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
	_back()


func _back():
	# the last item on the stack is always the current menu
	_menu_stack.pop_back()

	_popped_backwards = true
	_refresh()

	# we don't save which row the menu we popped to started from; don't accidentally select some
	# non-zero idx row
	_menu.set_current_choice_idx(0)


func _refresh():
	var current_choice_idx: int = _menu.get_current_choice_idx()

	_push_menu_impl(_menu_stack.pop_back(), true)

	# keeps the same row highlighted (assuming there are the same number of rows as before)
	_menu.set_current_choice_idx(current_choice_idx)


func _get_persisted_anchor_tracker_location_string(persisted_anchor: StringName, append: String, anchor_trackers: Array[OpenXRAndroidAnchorTracker]) -> String:
	var idx := anchor_trackers.find_custom(func(ele: OpenXRAndroidAnchorTracker) -> bool: return ele.get_persist_uuid() == persisted_anchor)
	if idx == -1:
		return ""
	return append + _get_location_string(anchor_trackers[idx])


func _get_location_string(anchor_tracker: OpenXRAndroidAnchorTracker) -> String:
	return "" if anchor_tracker == null else str(anchor_tracker.get_location_pose().origin)


func _on_hud_resized():
	$Viewport2Din3D.set_viewport_size(_menu.size)
