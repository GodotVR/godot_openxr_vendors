extends PanelContainer

const CPU_USAGE_LABEL_SUFFIX = "device/cpu_utilization_average"
const GPU_USAGE_LABEL_SUFFIX = "device/gpu_utilization"

@onready var _cpu_loader: CPULoader = $CPULoad

@onready var _cpu_usage: Label = $Panel/MarginContainer/VBoxContainer/HBoxContainer/CPUContainer/Usage
@onready var _gpu_usage: Label = $Panel/MarginContainer/VBoxContainer/HBoxContainer/GPUContainer/Usage

@onready var _cpu_load_slider: HSlider = $Panel/MarginContainer/VBoxContainer/HBoxContainer/CPUContainer/Slider
@onready var _gpu_load_slider: HSlider = $Panel/MarginContainer/VBoxContainer/HBoxContainer/GPUContainer/Slider

@onready var _recommended_resolution: Label = $Panel/MarginContainer/VBoxContainer/RecommendedResolution

@onready var _xr_interface: OpenXRInterface = XRServer.find_interface("OpenXR")

var cpu_usage_label: String = ""
var gpu_usage_label: String = ""


func _ready() -> void:
	var counter_paths: PackedStringArray = []
	if OpenXRVendorPerformanceMetrics.is_enabled():
		counter_paths = OpenXRVendorPerformanceMetrics.get_performance_metrics_counter_paths()
	for path in counter_paths:
		if path.ends_with(CPU_USAGE_LABEL_SUFFIX):
			cpu_usage_label = path
		elif path.ends_with(GPU_USAGE_LABEL_SUFFIX):
			gpu_usage_label = path
		else:
			pass
	_cpu_load_slider.value_changed.connect(_update_cpu_load.bind(_cpu_load_slider.max_value))


func _process(_delta: float) -> void:
	var gpu_load: int = 1000 * _gpu_load_slider.value / _gpu_load_slider.max_value

	var shader_material = self.material as ShaderMaterial
	shader_material.set_shader_parameter("ITERATIONS", randi_range(gpu_load, gpu_load + 20))


func _update_cpu_load(value: float, maxv: float) -> void:
	var load_factor = value / maxv
	if load_factor < 0.01:
		_cpu_loader.set_interval_usec(-1)
	else:
		_cpu_loader.set_interval_usec(1.0 / load_factor)


func _update_data() -> void:
	_update_recommended_resolution()
	_cpu_usage.text = _get_usage_string("CPU Usage: ", cpu_usage_label)
	_gpu_usage.text = _get_usage_string("GPU Usage: ", gpu_usage_label)


func _update_recommended_resolution():
	var render_target_size = _xr_interface.get_render_target_size()
	_recommended_resolution.text = "Recommended resolution:\n  width %s\n  height %s" % [render_target_size.x, render_target_size.y]


func _get_usage_string(label_prefix: String, counter_name: String) -> String:
	if counter_name.is_empty():
		return label_prefix + "NaN"

	var metric: Dictionary = OpenXRVendorPerformanceMetrics.query_performance_metrics_counter(counter_name)

	var metric_value_string := ""
	if 0 != (metric["counter_flags"] & (OpenXRVendorPerformanceMetrics.PERFORMANCE_METRICS_COUNTER_FLAGS_UINT_VALUE_VALID_BIT)):
		metric_value_string = "%s" % metric["uint_value"]
	elif 0 != (metric["counter_flags"] & (OpenXRVendorPerformanceMetrics.PERFORMANCE_METRICS_COUNTER_FLAGS_FLOAT_VALUE_VALID_BIT)):
		metric_value_string = "%.02f" % metric["float_value"]

	var metric_unit_string := ""
	match metric["counter_unit"]:
		OpenXRVendorPerformanceMetrics.PERFORMANCE_METRICS_COUNTER_UNIT_PERCENTAGE:
			metric_unit_string = "%"
		OpenXRVendorPerformanceMetrics.PERFORMANCE_METRICS_COUNTER_UNIT_MILLISECONDS:
			metric_unit_string = "ms"
		OpenXRVendorPerformanceMetrics.PERFORMANCE_METRICS_COUNTER_UNIT_BYTES:
			metric_unit_string = "bytes"
		OpenXRVendorPerformanceMetrics.PERFORMANCE_METRICS_COUNTER_UNIT_HERTZ:
			metric_unit_string = "hz"

	return label_prefix + metric_value_string + " " + metric_unit_string
