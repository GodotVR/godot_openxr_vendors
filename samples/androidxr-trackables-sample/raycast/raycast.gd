extends Node3D

@export var scenes: Array[PackedScene]

const TRANSFORM3D_UTILS = preload("res://transform3d_utils.gd")


func initialize():
	if scenes.is_empty():
		push_error("No scenes! add some in the inspector")
		return

	# reposition the hud to the left
	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()
	var xform: Transform3D = TRANSFORM3D_UTILS.upright_then_offset(camera_transform, Vector3(-1.5, 0.0, -0.3))

	# and point it towards the camera
	$Viewport2Din3D.global_transform = xform.looking_at(camera_transform.origin, Vector3.UP, true)

	_free_scenes()


# Callback from "menu3d_group"
func are_permissions_granted() -> bool:
	return OpenXRAndroidRaycastExtension.are_permissions_granted()


# Callback from "menu3d_group"
func update_menu(menu3d: Menu3D, menu_stack_item: Menu3D.MenuStackItem):
	if menu_stack_item.current_menu != Menu3D.MENU3D_MAIN:
		return

	var current_scene_name: String
	if 0 < $Scene.get_child_count():
		current_scene_name = $Scene.get_child(0).name

	for scene in scenes:
		if current_scene_name == scene.get_state().get_node_name(0):
			menu3d.get_menu2d().add_choice("Stop [%s]" % current_scene_name, _stop_raycasts.bind(menu3d))
		else:
			menu3d.get_menu2d().add_choice("Switch raycast mode to [%s]" % scene.get_state().get_node_name(0), _show_scene.bind(menu3d, scene))


func _show_scene(menu3d: Menu3D, packed_scene: PackedScene):
	var scene: Node3D = packed_scene.instantiate()
	$Viewport2Din3D.get_scene_root().text = scene.hud_instructions

	# free the old scene(s)
	_free_scenes()

	# and add our new scene
	$Scene.add_child(scene)
	$Viewport2Din3D.visible = true

	menu3d.refresh()


func _stop_raycasts(menu3d: Menu3D):
	_free_scenes()
	menu3d.refresh()


func _free_scenes():
	var children: Array[Node] = $Scene.get_children()

	# there should be only one child, but free all of them in case we somehow have more
	if 1 < children.size():
		push_error("Somehow have more than one active scene; freeing all of them")

	for child in children:
		$Scene.remove_child(child)
		child.queue_free()

	$Viewport2Din3D.visible = false
