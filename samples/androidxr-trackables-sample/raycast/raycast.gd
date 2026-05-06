extends Node3D

@export var scenes: Array[PackedScene]

var _next_scene_idx := 0


func initialize():
	if scenes.is_empty():
		push_error("No scenes! add some in the inspector")
		return

	# reposition the hud
	var camera_transform: Transform3D = get_tree().root.get_camera_3d().get_camera_transform()
	var forward_xz := Vector3(camera_transform.basis.z.x, 0.0, camera_transform.basis.z.z).normalized()
	var right := Vector3.UP.cross(forward_xz)
	var xform := Transform3D(right, Vector3.UP, forward_xz, camera_transform.origin)

	# put it to the left
	xform.origin = xform * Vector3(-1.5, 0.0, -0.3)

	# and point it towards the camera
	$Viewport2Din3D.global_transform = xform.looking_at(camera_transform.origin, Vector3.UP, true)

	show_next_scene()


func show_next_scene():
	var next_scene: Node3D = scenes[_next_scene_idx].instantiate()
	_next_scene_idx += 1
	if scenes.size() <= _next_scene_idx:
		_next_scene_idx = 0

	$Viewport2Din3D.get_scene_root().text = next_scene.hud_instructions

	# should only be one
	if 0 < $Scene.get_child_count():
		if $Scene.get_child_count() != 1:
			push_error("Somehow have more than one active scene")

		$Scene.get_child(0).queue_free()

	$Scene.add_child(next_scene)


# Callback from "menu3d_group"
func update_menu(menu3d: Menu3D, menu_stack_item: Menu3D.MenuStackItem):
	if menu_stack_item.current_menu != Menu3D.MENU3D_MAIN:
		return

	menu3d.get_menu2d().add_choice("switch raycast mode to: " + scenes[_next_scene_idx].get_state().get_node_name(0), _show_next_scene.bind(menu3d))


func _show_next_scene(menu3d: Menu3D):
	show_next_scene()
	menu3d.refresh()
