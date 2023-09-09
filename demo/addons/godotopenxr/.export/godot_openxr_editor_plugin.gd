@tool
extends EditorPlugin

var globals = preload("globals.gd")

# A class member to hold the export plugin during its lifecycle.
var meta_export_plugin : EditorExportPlugin
var pico_export_plugin : EditorExportPlugin
var lynx_export_plugin : EditorExportPlugin
var khronos_export_plugin : EditorExportPlugin


func _enter_tree():
	var plugin_version = get_plugin_version()
	
	# Initializing the export plugins
	meta_export_plugin = preload("meta/godot_openxr_meta_editor_export_plugin.gd").new("meta", plugin_version)
	pico_export_plugin = preload("pico/godot_openxr_pico_editor_export_plugin.gd").new("pico", plugin_version)
	lynx_export_plugin = preload("lynx/godot_openxr_lynx_editor_export_plugin.gd").new("lynx", plugin_version)
	khronos_export_plugin = preload("khronos/godot_openxr_khronos_editor_export_plugin.gd").new("khronos", plugin_version)
	
	add_export_plugin(meta_export_plugin)
	add_export_plugin(pico_export_plugin)
	add_export_plugin(lynx_export_plugin)
	add_export_plugin(khronos_export_plugin)


func _exit_tree():
	# Cleaning up the export plugins
	remove_export_plugin(meta_export_plugin)
	remove_export_plugin(pico_export_plugin)
	remove_export_plugin(lynx_export_plugin)
	remove_export_plugin(khronos_export_plugin)
	
	meta_export_plugin = null
	pico_export_plugin = null
	lynx_export_plugin = null
	khronos_export_plugin = null
