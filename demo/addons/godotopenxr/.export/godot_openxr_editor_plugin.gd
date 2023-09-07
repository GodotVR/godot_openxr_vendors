@tool
extends EditorPlugin

# A class member to hold the export plugin during its lifecycle.
var meta_export_plugin : EditorExportPlugin
var pico_export_plugin : EditorExportPlugin
var lynx_export_plugin : EditorExportPlugin
var khr_export_plugin : EditorExportPlugin


func _enter_tree():
	var plugin_version = get_plugin_version()
	
	# Initializing the export plugins
	meta_export_plugin = preload("meta/godot_openxr_meta_editor_export_plugin.gd").new("meta", plugin_version)
	pico_export_plugin = preload("pico/godot_openxr_pico_editor_export_plugin.gd").new("pico", plugin_version)
	lynx_export_plugin = preload("lynx/godot_openxr_lynx_editor_export_plugin.gd").new("lynx", plugin_version)
	khr_export_plugin = preload("khr/godot_openxr_khr_editor_export_plugin.gd").new("khr", plugin_version)
	
	add_export_plugin(meta_export_plugin)
	add_export_plugin(pico_export_plugin)
	add_export_plugin(lynx_export_plugin)
	add_export_plugin(khr_export_plugin)


func _exit_tree():
	# Cleaning up the export plugins
	remove_export_plugin(meta_export_plugin)
	remove_export_plugin(pico_export_plugin)
	remove_export_plugin(lynx_export_plugin)
	remove_export_plugin(khr_export_plugin)
	
	meta_export_plugin = null
	pico_export_plugin = null
	lynx_export_plugin = null
	khr_export_plugin = null
