extends StartXR

var counter_paths: PackedStringArray


func _ready():
	super._ready()

	if !OpenXRVendorPerformanceMetrics.is_enabled():
		printerr("Performance metrics are not enabled")
		return

	counter_paths = OpenXRVendorPerformanceMetrics.get_performance_metrics_counter_paths()
	OpenXRVendorPerformanceMetrics.set_capture_performance_metrics(true)
	if !OpenXRVendorPerformanceMetrics.is_capturing_performance_metrics():
		printerr("Unable to capture performance metrics state")
		return

	$XROrigin3D/Hud.data_source = _get_performance_metrics


func _get_performance_metrics() -> String:
	var text = ""
	for counter_path in counter_paths:
		var metric: Dictionary = OpenXRVendorPerformanceMetrics.query_performance_metrics_counter(counter_path)
		if metric.is_empty():
			continue

		if !text.is_empty():
			text += "\n"

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

		# remove the leading "/perfmetrics_android/" (first 21 characters) present in each "path"
		text += ("/".join(counter_path.split("/").slice(2)) + "=" + metric_value_string + metric_unit_string)

	return text
