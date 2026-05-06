extends Node3D
class_name Menu3D

const MENU3D_MAIN = "menu3d_main"


class MenuStackItem:
	var current_menu: String

	# Instead of introducing many MenuStackItem-types, we instead assign to generic args and let each
	# MenuType determine which args to use (or none at all)
	var arg1

	static func create(new_type: String, new_arg1):
		var new_menu_stack_item := MenuStackItem.new()
		new_menu_stack_item.current_menu = new_type
		new_menu_stack_item.arg1 = new_arg1
		return new_menu_stack_item


const TRANSFORM3D_UTILS = preload("res://transform3d_utils.gd")

# an [Array] of [class MenuStackItem]s
var _menu_stack := []

var _menu2d: Menu2D

var _popped_backwards: bool = false


func _ready() -> void:
	_menu2d = $Viewport2Din3D.get_scene_root()
	push_menu(MENU3D_MAIN)

	_menu2d.resized.connect(_on_hud_resized)
	_on_hud_resized()


func get_menu2d() -> Menu2D:
	return _menu2d


func push_menu(current_menu: String, arg1 = null):
	_push_menu_impl(MenuStackItem.create(current_menu, arg1), false)


func discard_current_menu():
	_menu_stack.pop_back()


func popped_backwards() -> bool:
	return _popped_backwards


func back():
	# the last item on the stack is always the current menu
	_menu_stack.pop_back()

	_popped_backwards = true
	refresh()


func refresh():
	_push_menu_impl(_menu_stack.pop_back(), true)


func _push_menu_impl(menu_stack_item: MenuStackItem, refreshed: bool):
	_menu_stack.push_back(menu_stack_item)
	_menu2d.clear_choices()

	# keep _popped_backwards state until the current menu is not refreshed
	_popped_backwards = _popped_backwards && refreshed

	if menu_stack_item.current_menu == MENU3D_MAIN:
		_menu2d.set_title_text("Main menu")

	get_tree().call_group("menu3d_group", "update_menu", self, menu_stack_item)


func _on_hud_resized():
	$Viewport2Din3D.set_viewport_size(_menu2d.size)
