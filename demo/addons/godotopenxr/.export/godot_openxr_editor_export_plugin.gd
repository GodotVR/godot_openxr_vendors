@tool
class_name GodotOpenXREditorExportPlugin extends EditorExportPlugin

const OPENXR_MODE_VALUE = 1
	
var _vendor: String
var _plugin_version: String

func _init(vendor: String, version: String):
	_vendor = vendor
	_plugin_version = version


func _get_name() -> String:
	return "GodotOpenXR" + _vendor.capitalize()


# Path to the Android library aar file
# If this is not available, we fall back to the maven central dependency
func _get_android_aar_file_path(debug: bool) -> String:
	return "res://addons/godotopenxr/.bin/" + _vendor + "/godotopenxr" + _vendor + "-" + ("debug.aar" if debug else "release.aar")


# Maven central dependency used as fall back when the Android library aar file is not available
func _get_android_maven_central_dependency() -> String:
	return "org.godotengine:godot-openxr-loaders-" + _vendor + ":" + _plugin_version


func _get_vendor_toggle_option_name() -> String:
	return "xr_features/enable_" + _vendor + "_plugin"


func _get_vendor_toggle_option() -> Dictionary:
	var toggle_option = {
		"option": {
			"name": _get_vendor_toggle_option_name(),
			"class_name": "",
			"type": TYPE_BOOL,
			"hint": PROPERTY_HINT_NONE,
			"hint_string": "",
			"usage": PROPERTY_USAGE_DEFAULT,
		},
	"default_value": false,
	"update_visibility": false,
	}
	return toggle_option


func _is_openxr_enabled() -> bool:
	return _get_int_option("xr_features/xr_mode", 0) == OPENXR_MODE_VALUE


func _get_export_options(platform) -> Array[Dictionary]:
	if not _supports_platform(platform):
		return []
	
	return [
		_get_vendor_toggle_option(), 
	]


func _get_export_option_warning(platform, option) -> String:
	if not _supports_platform(platform):
		return ""
	
	if option != _get_vendor_toggle_option_name():
		return ""
	
	if not(_is_openxr_enabled()) and _get_bool_option(option):
		return "\"Enable " + _vendor.capitalize() + " Plugin\" requires \"XR Mode\" to be \"OpenXR\".\n"
	
	return ""		


func _supports_platform(platform) -> bool:
	if platform is EditorExportPlatformAndroid:
		return true
	return false


func _get_bool_option(option: String) -> bool:
	var option_enabled = get_option(option)
	if option_enabled is bool:
		return option_enabled
	return false


func _get_int_option(option: String, default_value: int) -> int:
	var option_value = get_option(option)
	if option_value is int:
		return option_value
	return default_value


func _is_vendor_plugin_enabled() -> bool:
	return _get_bool_option(_get_vendor_toggle_option_name())
	

func _is_android_aar_file_available(debug: bool) -> bool:
	return FileAccess.file_exists(_get_android_aar_file_path(debug))
	

func _get_android_dependencies(platform, debug) -> PackedStringArray:
	if not _supports_platform(platform):
		return PackedStringArray()
		
	if _is_vendor_plugin_enabled() and not _is_android_aar_file_available(debug):
		return PackedStringArray([_get_android_maven_central_dependency()])
	
	return PackedStringArray()


func _get_android_libraries(platform, debug) -> PackedStringArray:
	if not _supports_platform(platform):
		return PackedStringArray()
		
	if _is_vendor_plugin_enabled() and _is_android_aar_file_available(debug):
		return PackedStringArray([_get_android_aar_file_path(debug)])
		
	return PackedStringArray()
