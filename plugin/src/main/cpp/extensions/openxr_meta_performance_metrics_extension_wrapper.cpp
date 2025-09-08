/**************************************************************************/
/*  openxr_meta_performance_metrics_extension_wrapper.cpp                 */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "extensions/openxr_meta_performance_metrics_extension_wrapper.h"

#include "classes/openxr_vendor_performance_metrics.h"
#include "classes/openxr_vendor_performance_metrics_provider.h"
#include "openxr/openxr.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/performance.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRMetaPerformanceMetricsExtensionWrapper *OpenXRMetaPerformanceMetricsExtensionWrapper::singleton = nullptr;

OpenXRMetaPerformanceMetricsExtensionWrapper *OpenXRMetaPerformanceMetricsExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaPerformanceMetricsExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaPerformanceMetricsExtensionWrapper::OpenXRMetaPerformanceMetricsExtensionWrapper() :
		OpenXRVendorPerformanceMetricsProvider() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaPerformanceMetricsExtensionWrapper singleton already exists.");

	request_extensions[XR_META_PERFORMANCE_METRICS_EXTENSION_NAME] = &meta_performance_metrics_ext;
	singleton = this;
}

OpenXRMetaPerformanceMetricsExtensionWrapper::~OpenXRMetaPerformanceMetricsExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRMetaPerformanceMetricsExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRMetaPerformanceMetricsExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (meta_performance_metrics_ext) {
		bool result = initialize_meta_performance_metrics_extension((XrInstance)instance);
		if (!result) {
			ERR_PRINT("Failed to initialize XR_META_performance_metrics extension");
			meta_performance_metrics_ext = false;
			return;
		}

		OpenXRVendorPerformanceMetrics::get_singleton()->set_vendor_performance_metrics_provider(this);
	}
}

void OpenXRMetaPerformanceMetricsExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRMetaPerformanceMetricsExtensionWrapper::_on_state_ready() {
	bool enable_on_startup = (bool)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/vendor_performance_metrics/capture_on_startup");
	if (enable_on_startup) {
		OpenXRVendorPerformanceMetrics::get_singleton()->set_capture_performance_metrics(true);
	}
}

bool OpenXRMetaPerformanceMetricsExtensionWrapper::set_capture_performance_metrics(bool p_enabled) {
	ERR_FAIL_COND_V_MSG(!meta_performance_metrics_ext, false, "XR_META_performance_metrics extension is not enabled");

	performance_metrics_state.enabled = p_enabled;
	XrResult result = xrSetPerformanceMetricsStateMETA(SESSION, &performance_metrics_state);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose(vformat("Failed to set performance metrics state [%s]", get_openxr_api()->get_error_string(result)));
		return false;
	}

	return true;
}

void OpenXRMetaPerformanceMetricsExtensionWrapper::add_custom_monitors() {
	if (!counter_paths_populated) {
		populate_performance_metrics_counter_paths();
	}

	Performance *performance = Performance::get_singleton();
	for (const String &path : performance_metrics_counter_paths) {
		String monitor_id = "xr_" + path.trim_prefix("/perfmetrics_meta/");
		performance->add_custom_monitor(monitor_id, callable_mp(this, &OpenXRMetaPerformanceMetricsExtensionWrapper::get_monitor_data).bind(path));
	}
}

Variant OpenXRMetaPerformanceMetricsExtensionWrapper::get_monitor_data(const String &p_counter_path) {
	Dictionary query_data = query_performance_metrics_counter(p_counter_path);
	if (query_data.is_empty()) {
		return -1;
	} else if ((int)query_data["counter_flags"] & XR_PERFORMANCE_METRICS_COUNTER_FLOAT_VALUE_VALID_BIT_META) {
		return query_data["float_value"];
	} else if ((int)query_data["counter_flags"] & XR_PERFORMANCE_METRICS_COUNTER_UINT_VALUE_VALID_BIT_META) {
		return query_data["uint_value"];
	}

	return -1;
}

void OpenXRMetaPerformanceMetricsExtensionWrapper::populate_performance_metrics_counter_paths() {
	uint32_t num_counter_paths = 0;
	XrInstance xr_instance = (XrInstance)get_openxr_api()->get_instance();

	XrResult result = xrEnumeratePerformanceMetricsCounterPathsMETA(xr_instance, 0, &num_counter_paths, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose(vformat("Failed to get performance metrics counter path count [%s]", get_openxr_api()->get_error_string(result)));
		return;
	}

	if (num_counter_paths <= 0) {
		counter_paths_populated = true;
		return;
	}

	performance_metrics_counter_xr_paths.resize(num_counter_paths);
	result = xrEnumeratePerformanceMetricsCounterPathsMETA(xr_instance, num_counter_paths, &num_counter_paths, performance_metrics_counter_xr_paths.ptrw());
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose(vformat("Failed to get performance metrics counter paths [%s]", get_openxr_api()->get_error_string(result)));
		performance_metrics_counter_xr_paths.clear();
		return;
	}

	performance_metrics_counter_paths.resize(num_counter_paths);
	int i = 0;

	for (const XrPath &xr_path : performance_metrics_counter_xr_paths) {
		uint32_t buffer_size = 0;
		result = xrPathToString(xr_instance, xr_path, 0, &buffer_size, nullptr);
		if (XR_FAILED(result)) {
			UtilityFunctions::print_verbose(vformat("Failed to get XrPath string buffer size [%s]", get_openxr_api()->get_error_string(result)));
			performance_metrics_counter_paths.clear();
			performance_metrics_counter_xr_paths.clear();
			return;
		}

		CharString string_buffer;
		string_buffer.resize(buffer_size);
		result = xrPathToString(xr_instance, xr_path, buffer_size, &buffer_size, string_buffer.ptrw());
		if (XR_FAILED(result)) {
			UtilityFunctions::print_verbose(vformat("Failed to get XrPath string [%s]", get_openxr_api()->get_error_string(result)));
			performance_metrics_counter_paths.clear();
			performance_metrics_counter_xr_paths.clear();
			return;
		}

		performance_metrics_counter_paths[i++] = String::utf8(string_buffer);
	}

	counter_paths_populated = true;
}

PackedStringArray OpenXRMetaPerformanceMetricsExtensionWrapper::get_performance_metrics_counter_paths() {
	ERR_FAIL_COND_V_MSG(!meta_performance_metrics_ext, PackedStringArray(), "XR_META_performance_metrics extension is not enabled");

	if (!counter_paths_populated) {
		populate_performance_metrics_counter_paths();
	}

	return performance_metrics_counter_paths;
}

Dictionary OpenXRMetaPerformanceMetricsExtensionWrapper::query_performance_metrics_counter(const String &p_counter_path) {
	ERR_FAIL_COND_V_MSG(!meta_performance_metrics_ext, Dictionary(), "XR_META_performance_metrics extension is not enabled");

	if (!counter_paths_populated) {
		populate_performance_metrics_counter_paths();
	}

	int counter_path_index = performance_metrics_counter_paths.find(p_counter_path);
	ERR_FAIL_COND_V_MSG(counter_path_index < 0, Dictionary(), vformat("String \"%s\" is not a valid counter path"));

	XrPath xr_path = performance_metrics_counter_xr_paths[counter_path_index];
	XrPerformanceMetricsCounterMETA counter = {
		XR_TYPE_PERFORMANCE_METRICS_COUNTER_META, // type
		nullptr, // next
	};

	XrResult result = xrQueryPerformanceMetricsCounterMETA(SESSION, xr_path, &counter);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose(vformat("Failed to query performance metrics counter [%s]", get_openxr_api()->get_error_string(result)));
		return Dictionary();
	}

	OpenXRVendorPerformanceMetrics::PerformanceMetricsCounterUnit counter_unit = OpenXRVendorPerformanceMetrics::PerformanceMetricsCounterUnit(counter.counterUnit);

	Dictionary ret;
	if (counter.counterFlags & XR_PERFORMANCE_METRICS_COUNTER_ANY_VALUE_VALID_BIT_META) {
		ret["counter_path"] = p_counter_path;
		ret["counter_flags"] = counter.counterFlags;
		ret["counter_unit"] = counter_unit;

		if (counter.counterFlags & XR_PERFORMANCE_METRICS_COUNTER_UINT_VALUE_VALID_BIT_META) {
			ret["uint_value"] = counter.uintValue;
		}
		if (counter.counterFlags & XR_PERFORMANCE_METRICS_COUNTER_FLOAT_VALUE_VALID_BIT_META) {
			ret["float_value"] = counter.floatValue;
		}
	}
	return ret;
}

void OpenXRMetaPerformanceMetricsExtensionWrapper::cleanup() {
	meta_performance_metrics_ext = false;
	counter_paths_populated = false;
	performance_metrics_counter_xr_paths.clear();
	performance_metrics_counter_paths.clear();
}

bool OpenXRMetaPerformanceMetricsExtensionWrapper::initialize_meta_performance_metrics_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrSetPerformanceMetricsStateMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumeratePerformanceMetricsCounterPathsMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrQueryPerformanceMetricsCounterMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrPathToString);

	return true;
}
