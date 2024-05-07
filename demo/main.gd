extends Node3D

@export var passthrough_gradient: GradientTexture1D
@export var passthrough_curve: Curve
@export var bcs: Vector3
@export var color_lut: Image
@export var color_lut2: Image

var xr_interface: XRInterface = null
var hand_tracking_source: Array[OpenXRInterface.HandTrackedSource]
var passthrough_enabled: bool = false
var fb_passthrough
var meta_color_lut: OpenXRMetaPassthroughColorLut
var meta_color_lut2: OpenXRMetaPassthroughColorLut
var lut_tween: Tween
var selected_spatial_anchor_node: Node3D = null

@onready var left_hand: XRController3D = $XROrigin3D/LeftHand
@onready var right_hand: XRController3D = $XROrigin3D/RightHand
@onready var left_hand_mesh: MeshInstance3D = $XROrigin3D/LeftHand/LeftHandMesh
@onready var right_hand_mesh: MeshInstance3D = $XROrigin3D/RightHand/RightHandMesh
@onready var left_controller_model: OpenXRFbRenderModel = $XROrigin3D/LeftHand/LeftControllerFbRenderModel
@onready var right_controller_model: OpenXRFbRenderModel = $XROrigin3D/RightHand/RightControllerFbRenderModel
@onready var left_hand_pointer: XRController3D = $XROrigin3D/LeftHandPointer
@onready var left_hand_pointer_raycast: RayCast3D = $XROrigin3D/LeftHandPointer/RayCast3D
@onready var scene_pointer_mesh: MeshInstance3D = $XROrigin3D/LeftHandPointer/ScenePointerMesh
@onready var scene_colliding_mesh: MeshInstance3D = $XROrigin3D/LeftHandPointer/SceneCollidingMesh
@onready var floor_mesh: MeshInstance3D = $Floor
@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var scene_manager: OpenXRFbSceneManager = $XROrigin3D/OpenXRFbSceneManager
@onready var spatial_anchor_manager: OpenXRFbSpatialAnchorManager = $XROrigin3D/OpenXRFbSpatialAnchorManager
@onready var open_xr_fb_passthrough_geometry: OpenXRFbPassthroughGeometry = %OpenXRFbPassthroughGeometry
@onready var passthrough_mode_info: Label3D = $XROrigin3D/RightHand/PassthroughModeInfo
@onready var passthrough_filter_info: Label3D = $XROrigin3D/RightHand/PassthroughFilterInfo

const SPATIAL_ANCHORS_FILE = "user://openxr_fb_spatial_anchors.json"
const SpatialAnchor = preload("res://spatial_anchor.tscn")

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
	if xr_interface and xr_interface.is_initialized():
		xr_interface.session_begun.connect(self.load_spatial_anchors_from_file)
		var vp: Viewport = get_viewport()
		vp.use_xr = true

	hand_tracking_source.resize(OpenXRInterface.HAND_MAX)
	for hand in OpenXRInterface.HAND_MAX:
		hand_tracking_source[hand] = xr_interface.get_hand_tracking_source(hand)

	fb_passthrough = Engine.get_singleton("OpenXRFbPassthroughExtensionWrapper")
	meta_color_lut = OpenXRMetaPassthroughColorLut.create_from_image(color_lut, OpenXRMetaPassthroughColorLut.COLOR_LUT_CHANNELS_RGB)
	meta_color_lut2 = OpenXRMetaPassthroughColorLut.create_from_image(color_lut2, OpenXRMetaPassthroughColorLut.COLOR_LUT_CHANNELS_RGB)

	randomize()


func load_spatial_anchors_from_file() -> void:
	var file := FileAccess.open(SPATIAL_ANCHORS_FILE, FileAccess.READ)
	if not file:
		print ("no file")
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
	print("Supported blend modes: ", supported_blend_modes)
	if XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND in supported_blend_modes and XRInterface.XR_ENV_BLEND_MODE_OPAQUE in supported_blend_modes:
		print("Passthrough supported.")
		if enable:
			# Switch to passthrough.
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
			get_viewport().transparent_bg = true
			world_environment.environment.background_mode = Environment.BG_COLOR
			world_environment.environment.background_color = Color(0.0, 0.0, 0.0, 0.0)
			floor_mesh.visible = false
			scene_manager.visible = true
			spatial_anchor_manager.visible = true
			left_hand_pointer.visible = true
			left_hand_pointer_raycast.enabled = true
			if not scene_manager.are_scene_anchors_created():
				scene_manager.create_scene_anchors()
		else:
			# Switch back to VR.
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
			get_viewport().transparent_bg = false
			world_environment.environment.background_mode = Environment.BG_SKY
			floor_mesh.visible = true
			scene_manager.visible = false
			spatial_anchor_manager.visible = false
			left_hand_pointer.visible = false
			left_hand_pointer_raycast.enabled = false
		passthrough_enabled = enable
	else:
		print("Switching to/from passthrough not supported.")


func _physics_process(_delta: float) -> void:
	for hand in OpenXRInterface.HAND_MAX:
		var source = xr_interface.get_hand_tracking_source(hand)
		if hand_tracking_source[hand] == source:
			continue

		var controller = left_controller_model if (hand == OpenXRInterface.HAND_LEFT) else right_controller_model
		controller.visible = (source == OpenXRInterface.HAND_TRACKED_SOURCE_CONTROLLER)

		if source == OpenXRInterface.HAND_TRACKED_SOURCE_UNOBSTRUCTED:
			match hand:
				OpenXRInterface.HAND_LEFT:
					left_hand.tracker = "/user/fbhandaim/left"
				OpenXRInterface.HAND_RIGHT:
					right_hand.tracker = "/user/fbhandaim/right"
		else:
			match hand:
				OpenXRInterface.HAND_LEFT:
					left_hand.tracker = "left_hand"
					left_hand.pose = "grip"
				OpenXRInterface.HAND_RIGHT:
					right_hand.tracker = "right_hand"
					right_hand.pose = "grip"

		hand_tracking_source[hand] = source

	if left_hand_pointer.visible:
		var previous_selected_spatial_anchor_node = selected_spatial_anchor_node

		if left_hand_pointer_raycast.is_colliding():
			var collision_point: Vector3 = left_hand_pointer_raycast.get_collision_point()
			scene_colliding_mesh.global_position = collision_point

			var pointer_length: float = (collision_point - left_hand_pointer.global_position).length()
			scene_pointer_mesh.mesh.size.z = pointer_length
			scene_pointer_mesh.position.z = -pointer_length / 2.0

			var collider: CollisionObject3D = left_hand_pointer_raycast.get_collider()
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
	if name == "menu_button":
		print("Triggering scene capture")
		scene_manager.request_scene_capture()

	elif name == "by_button":
		enable_passthrough(not passthrough_enabled)

	elif name == "trigger_click" and left_hand_pointer.visible:
		if left_hand_pointer_raycast.is_colliding():
			if selected_spatial_anchor_node:
				var anchor_parent = selected_spatial_anchor_node.get_parent()
				if anchor_parent is XRAnchor3D:
					spatial_anchor_manager.untrack_anchor(anchor_parent.tracker)
			else:
				var anchor_transform := Transform3D()
				anchor_transform.origin = left_hand_pointer_raycast.get_collision_point()

				var collision_normal: Vector3 = left_hand_pointer_raycast.get_collision_normal()
				if collision_normal.is_equal_approx(Vector3.UP):
					anchor_transform.basis = anchor_transform.basis.rotated(Vector3(1.0, 0.0, 0.0), PI / 2.0)
				elif collision_normal.is_equal_approx(Vector3.DOWN):
					anchor_transform.basis = anchor_transform.basis.rotated(Vector3(1.0, 0.0, 0.0), -PI / 2.0)
				else:
					anchor_transform.basis = Basis.looking_at(left_hand_pointer_raycast.get_collision_normal())

				print ("Attempting to create spatial anchor at: ", anchor_transform)
				spatial_anchor_manager.create_anchor(anchor_transform, { color = COLORS[randi() % COLORS.size()] })


func _on_right_hand_button_pressed(name: String) -> void:
	match name:
		"by_button":
			update_passthrough_mode()
		"ax_button":
			update_passthrough_filter()


func _on_left_controller_fb_render_model_render_model_loaded() -> void:
	left_hand_mesh.hide()

func _on_right_controller_fb_render_model_render_model_loaded() -> void:
	right_hand_mesh.hide()

func _on_scene_manager_scene_capture_completed(success: bool) -> void:
	print("Scene Capture Complete: ", success)
	if success:
		# Recreate scene anchors since the user may have changed them.
		if scene_manager.are_scene_anchors_created():
			scene_manager.remove_scene_anchors()
			scene_manager.create_scene_anchors()

		# Switch to passthrough.
		enable_passthrough(true)

func _on_scene_manager_scene_data_missing() -> void:
	scene_manager.request_scene_capture()

func update_passthrough_mode() -> void:
	const STRING_BASE = "[B] Passthrough Mode: "

	match fb_passthrough.get_current_layer_purpose():
		OpenXRFbPassthroughExtensionWrapper.LAYER_PURPOSE_NONE:
			enable_passthrough_environment(true)
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
			passthrough_mode_info.text = STRING_BASE + "Full"
		OpenXRFbPassthroughExtensionWrapper.LAYER_PURPOSE_RECONSTRUCTION:
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
			open_xr_fb_passthrough_geometry.show()
			passthrough_mode_info.text = STRING_BASE + "Geometry"
		OpenXRFbPassthroughExtensionWrapper.LAYER_PURPOSE_PROJECTED:
			enable_passthrough_environment(false)
			open_xr_fb_passthrough_geometry.hide()
			passthrough_mode_info.text = STRING_BASE + "None"

func enable_passthrough_environment(enable: bool) -> void:
	if enable:
		get_viewport().transparent_bg = true
		world_environment.environment.background_mode = Environment.BG_COLOR
	else:
		get_viewport().transparent_bg = false
		world_environment.environment.background_mode = Environment.BG_SKY

func update_passthrough_filter() -> void:
	const STRING_BASE = "[A] Passthrough Filter: "

	match fb_passthrough.get_current_passthrough_filter():
		OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_DISABLED:
			fb_passthrough.set_color_map(passthrough_gradient)
			passthrough_filter_info.text = STRING_BASE + "Color Map"
		OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_COLOR_MAP:
			fb_passthrough.set_mono_map(passthrough_curve)
			passthrough_filter_info.text = STRING_BASE + "Mono Map"
		OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_MONO_MAP:
			fb_passthrough.set_brightness_contrast_saturation(bcs.x, bcs.y, bcs.z)
			passthrough_filter_info.text = STRING_BASE + "Brightness Contrast Saturation"
		OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION:
			fb_passthrough.set_color_lut(1.0, meta_color_lut)
			passthrough_filter_info.text = STRING_BASE + "Color Map LUT"
		OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_COLOR_MAP_LUT:
			lut_tween = create_tween()
			lut_tween.set_loops()
			lut_tween.tween_method(fb_passthrough.set_interpolated_color_lut.bind(meta_color_lut, meta_color_lut2), 0.0, 1.0, 2.0).set_delay(.1)
			lut_tween.tween_method(fb_passthrough.set_interpolated_color_lut.bind(meta_color_lut, meta_color_lut2), 1.0, 0.0, 2.0).set_delay(.1)
			passthrough_filter_info.text = STRING_BASE + "Interpolated Color Map LUT"
		OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT:
			lut_tween.kill()
			fb_passthrough.set_passthrough_filter(OpenXRFbPassthroughExtensionWrapper.PASSTHROUGH_FILTER_DISABLED)
			passthrough_filter_info.text = STRING_BASE + "Disabled"
