extends Node3D

# This script uses "A Better XR Start Script" in the Godot Docs as a starting template
# https://docs.godotengine.org/en/latest/tutorials/xr/a_better_xr_start_script.html

signal focus_lost
signal focus_gained
signal pose_recentered

@export var maximum_refresh_rate: int = 90

var xr_interface: XRInterface = null
var xr_is_focussed: bool = false
var hand_tracking_source: Array[OpenXRInterface.HandTrackedSource]
var passthrough_enabled: bool = false
var scene_and_spatial_anchors_displayed: bool = true
var selected_spatial_anchor_node: Node3D = null

@onready var left_hand: XRController3D = $XROrigin3D/LeftHand
@onready var right_hand: XRController3D = $XROrigin3D/RightHand
@onready var right_hand_pointer: XRController3D = $XROrigin3D/RightHandPointer
@onready var right_hand_pointer_raycast: RayCast3D = $XROrigin3D/RightHandPointer/RayCast3D
@onready var scene_pointer_mesh: MeshInstance3D = $XROrigin3D/RightHandPointer/ScenePointerMesh
@onready var scene_colliding_mesh: MeshInstance3D = $XROrigin3D/RightHandPointer/SceneCollidingMesh
@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var scene_manager: OpenXRFbSceneManager = $XROrigin3D/OpenXRFbSceneManager
@onready var spatial_anchor_manager: OpenXRFbSpatialAnchorManager = $XROrigin3D/OpenXRFbSpatialAnchorManager

const SPATIAL_ANCHORS_FILE = "user://openxr_fb_spatial_anchors.json"

const COLORS = [
	"#FF0000", # Red
	"#00FF00", # Green
	"#0000FF", # Blue
	"#FFFF00", # Yellow
	"#00FFFF", # Cyan
	"#FF00FF", # Magenta
	"#FF8000", # Orange
	"#800080", # Purple
]

# Called when the node enters the scene tree for the first time.
func _ready():
	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.initialize():
		print("OpenXR instantiated successfully.")
		var vp: Viewport = get_viewport()

		# Enable XR on our viewport
		vp.use_xr = true

		# Make sure v-sync is off, v-sync is handled by OpenXR
		DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)

		# Enable VRS
		if RenderingServer.get_rendering_device():
			vp.vrs_mode = Viewport.VRS_XR
		elif int(ProjectSettings.get_setting("xr/openxr/foveation_level")) == 0:
			push_warning("OpenXR: Recommend setting Foveation level to High in Project Settings")

		# Connect the OpenXR events
		xr_interface.session_begun.connect(_on_openxr_session_begun)
		xr_interface.session_visible.connect(_on_openxr_visible_state)
		xr_interface.session_focussed.connect(_on_openxr_focused_state)
		xr_interface.session_stopping.connect(_on_openxr_stopping)
		xr_interface.pose_recentered.connect(_on_openxr_pose_recentered)
	else:
		# We couldn't start OpenXR.
		print("OpenXR not instantiated!")
		get_tree().quit()


# Handle OpenXR session ready
func _on_openxr_session_begun() -> void:
	# Get the reported refresh rate
	var current_refresh_rate = xr_interface.get_display_refresh_rate()
	if current_refresh_rate > 0:
		print("OpenXR: Refresh rate reported as ", str(current_refresh_rate))
	else:
		print("OpenXR: No refresh rate given by XR runtime")

	# See if we have a better refresh rate available
	var new_rate = current_refresh_rate
	var available_rates : Array = xr_interface.get_available_display_refresh_rates()
	if available_rates.size() == 0:
		print("OpenXR: Target does not support refresh rate extension")
	elif available_rates.size() == 1:
		# Only one available, so use it
		new_rate = available_rates[0]
	else:
		for rate in available_rates:
			if rate > new_rate and rate <= maximum_refresh_rate:
				new_rate = rate

	# Did we find a better rate?
	if current_refresh_rate != new_rate:
		print("OpenXR: Setting refresh rate to ", str(new_rate))
		xr_interface.set_display_refresh_rate(new_rate)
		current_refresh_rate = new_rate

	# Now match our physics rate
	Engine.physics_ticks_per_second = current_refresh_rate

	load_spatial_anchors_from_file()
	enable_passthrough(true)


# Handle OpenXR visible state
func _on_openxr_visible_state() -> void:
	# We always pass this state at startup,
	# but the second time we get this it means our player took off their headset
	if xr_is_focussed:
		print("OpenXR lost focus")

		xr_is_focussed = false

		# pause our game
		process_mode = Node.PROCESS_MODE_DISABLED

		emit_signal("focus_lost")


# Handle OpenXR focused state
func _on_openxr_focused_state() -> void:
	print("OpenXR gained focus")
	xr_is_focussed = true

	# unpause our game
	process_mode = Node.PROCESS_MODE_INHERIT

	emit_signal("focus_gained")


# Handle OpenXR stopping state
func _on_openxr_stopping() -> void:
	# Our session is being stopped.
	print("OpenXR is stopping")

	if "--quit-with-openxr" in OS.get_cmdline_user_args():
		# When we're running tests via the XR Simulator, it will end the OpenXR
		# session automatically, and in that case, we want to quit.
		get_tree().quit()


# Handle OpenXR pose recentered signal
func _on_openxr_pose_recentered() -> void:
	# User recentered view, we have to react to this by recentering the view.
	# This is game implementation dependent.
	emit_signal("pose_recentered")


func load_spatial_anchors_from_file() -> void:
	var file := FileAccess.open(SPATIAL_ANCHORS_FILE, FileAccess.READ)
	if not file:
		return

	var json := JSON.new()
	if json.parse(file.get_as_text()) != OK:
		print("ERROR: Unable to parse ", SPATIAL_ANCHORS_FILE)
		return

	if not json.data is Dictionary:
		print("ERROR: ", SPATIAL_ANCHORS_FILE, " contains invalid data")
		return

	var anchor_data: Dictionary = json.data
	if anchor_data.size() > 0:
		spatial_anchor_manager.load_anchors(anchor_data.keys(), anchor_data, OpenXRFbSpatialEntity.STORAGE_LOCAL, true)


func save_spatial_anchors_to_file() -> void:
	var file := FileAccess.open(SPATIAL_ANCHORS_FILE, FileAccess.WRITE)
	if not file:
		print("ERROR: Unable to open file for writing: ", SPATIAL_ANCHORS_FILE)
		return

	var anchor_data := {}
	for uuid in spatial_anchor_manager.get_anchor_uuids():
		var entity: OpenXRFbSpatialEntity = spatial_anchor_manager.get_spatial_entity(uuid)
		anchor_data[uuid] = entity.custom_data

	file.store_string(JSON.stringify(anchor_data))
	file.close()


func _on_spatial_anchor_tracked(_anchor_node: XRAnchor3D, _spatial_entity: OpenXRFbSpatialEntity, is_new: bool) -> void:
	if is_new:
		save_spatial_anchors_to_file()


func _on_spatial_anchor_untracked(_anchor_node: XRAnchor3D, _spatial_entity: OpenXRFbSpatialEntity) -> void:
	save_spatial_anchors_to_file()


func enable_passthrough(enable: bool) -> void:
	if passthrough_enabled == enable:
		return

	var supported_blend_modes = xr_interface.get_supported_environment_blend_modes()
	if XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND in supported_blend_modes and XRInterface.XR_ENV_BLEND_MODE_OPAQUE in supported_blend_modes:
		if enable:
			# Switch to passthrough.
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
			get_viewport().transparent_bg = true
			world_environment.environment.background_color = Color(0.0, 0.0, 0.0, 0.0)
		else:
			# Switch back to VR.
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
			get_viewport().transparent_bg = false
			world_environment.environment.background_color = Color(0.3, 0.3, 0.3, 1.0)
		passthrough_enabled = enable


func display_scene_and_spatial_anchors(value: bool) -> void:
	if scene_and_spatial_anchors_displayed == value:
		return

	scene_manager.visible = value
	spatial_anchor_manager.visible = value
	right_hand_pointer.visible = value
	right_hand_pointer_raycast.enabled = value

	scene_and_spatial_anchors_displayed = value


func _physics_process(_delta: float) -> void:
	if right_hand_pointer.visible:
		var previous_selected_spatial_anchor_node = selected_spatial_anchor_node

		if right_hand_pointer_raycast.is_colliding():
			var collision_point: Vector3 = right_hand_pointer_raycast.get_collision_point()
			scene_colliding_mesh.global_position = collision_point

			var pointer_length: float = (collision_point - right_hand_pointer.global_position).length()
			scene_pointer_mesh.mesh.size.z = pointer_length
			scene_pointer_mesh.position.z = -pointer_length / 2.0

			var collider: CollisionObject3D = right_hand_pointer_raycast.get_collider()
			if collider and collider.get_collision_layer_value(3):
				selected_spatial_anchor_node = collider
			else:
				selected_spatial_anchor_node = null
		else:
			scene_pointer_mesh.mesh.size.z = 5
			scene_pointer_mesh.position.z = -2.5
			selected_spatial_anchor_node = null

		if previous_selected_spatial_anchor_node != selected_spatial_anchor_node:
			if previous_selected_spatial_anchor_node:
				previous_selected_spatial_anchor_node.set_selected(false)
			if selected_spatial_anchor_node:
				selected_spatial_anchor_node.set_selected(true)
				scene_colliding_mesh.visible = false
			else:
				scene_colliding_mesh.visible = true


func _on_left_hand_button_pressed(name):
	if name == "ax_button":
		display_scene_and_spatial_anchors(not scene_and_spatial_anchors_displayed)
	elif name == "by_button":
		enable_passthrough(not passthrough_enabled)
	elif name == "menu_button":
		scene_manager.request_scene_capture()


func _on_right_hand_button_pressed(name: String) -> void:
	if name == "trigger_click" and right_hand_pointer.visible:
		if right_hand_pointer_raycast.is_colliding():
			if selected_spatial_anchor_node:
				var anchor_parent = selected_spatial_anchor_node.get_parent()
				if anchor_parent is XRAnchor3D:
					spatial_anchor_manager.untrack_anchor(anchor_parent.tracker)
			else:
				var anchor_transform := Transform3D()
				anchor_transform.origin = right_hand_pointer_raycast.get_collision_point()

				var collision_normal: Vector3 = right_hand_pointer_raycast.get_collision_normal()
				if collision_normal.is_equal_approx(Vector3.UP):
					anchor_transform.basis = anchor_transform.basis.rotated(Vector3(1.0, 0.0, 0.0), PI / 2.0)
				elif collision_normal.is_equal_approx(Vector3.DOWN):
					anchor_transform.basis = anchor_transform.basis.rotated(Vector3(1.0, 0.0, 0.0), -PI / 2.0)
				else:
					anchor_transform.basis = Basis.looking_at(right_hand_pointer_raycast.get_collision_normal())

				spatial_anchor_manager.create_anchor(anchor_transform, { color = COLORS[randi() % COLORS.size()] })
	elif name == "ax_button":
		var anchor_transform := right_hand.transform
		spatial_anchor_manager.create_anchor(anchor_transform, { color = COLORS[randi() % COLORS.size()] })


func _on_scene_manager_scene_capture_completed(success: bool) -> void:
	if success:
		# Recreate scene anchors since the user may have changed them.
		if scene_manager.are_scene_anchors_created():
			scene_manager.remove_scene_anchors()
		scene_manager.create_scene_anchors()


func _on_scene_manager_scene_data_missing() -> void:
	scene_manager.request_scene_capture()
