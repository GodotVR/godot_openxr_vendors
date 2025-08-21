/**************************************************************************/
/*  openxr_vendor_performance_metrics.cpp                                 */
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

#include "classes/openxr_vendor_performance_metrics.h"

#include "classes/openxr_vendor_performance_metrics_provider.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRVendorPerformanceMetrics *OpenXRVendorPerformanceMetrics::singleton = nullptr;

OpenXRVendorPerformanceMetrics *OpenXRVendorPerformanceMetrics::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRVendorPerformanceMetrics());
	}
	return singleton;
}

OpenXRVendorPerformanceMetrics::OpenXRVendorPerformanceMetrics() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRVendorPerformanceMetrics singleton already exists.");
	singleton = this;
}

OpenXRVendorPerformanceMetrics::~OpenXRVendorPerformanceMetrics() {
	singleton = nullptr;
}

bool OpenXRVendorPerformanceMetrics::is_enabled() {
	if (provider == nullptr) {
		UtilityFunctions::print_verbose("No vendor performance metrics provider has been set");
		return false;
	}

	return provider->is_enabled();
}

void OpenXRVendorPerformanceMetrics::set_vendor_performance_metrics_provider(OpenXRVendorPerformanceMetricsProvider *p_provider) {
	if (provider == p_provider) {
		return;
	}

	provider = p_provider;
}

OpenXRVendorPerformanceMetricsProvider *OpenXRVendorPerformanceMetrics::get_vendor_performance_metrics_provider() {
	return provider;
}

void OpenXRVendorPerformanceMetrics::set_capture_performance_metrics(bool p_enabled) {
	ERR_FAIL_NULL_MSG(provider, "No vendor performance metrics provider has been set");

	if (p_enabled == capture_performance_metrics) {
		return;
	}

	if (!provider->set_capture_performance_metrics(p_enabled)) {
		return;
	}

	capture_performance_metrics = p_enabled;

	if (capture_performance_metrics && !custom_monitors_added) {
		provider->add_custom_monitors();
		custom_monitors_added = true;
	}
}

bool OpenXRVendorPerformanceMetrics::is_capturing_performance_metrics() {
	return capture_performance_metrics;
}

PackedStringArray OpenXRVendorPerformanceMetrics::get_performance_metrics_counter_paths() {
	ERR_FAIL_NULL_V_MSG(provider, PackedStringArray(), "No vendor performance metrics provider has been set");

	return provider->get_performance_metrics_counter_paths();
}

Dictionary OpenXRVendorPerformanceMetrics::query_performance_metrics_counter(const String &p_counter_path) {
	ERR_FAIL_NULL_V_MSG(provider, Dictionary(), "No vendor performance metrics provider has been set");

	return provider->query_performance_metrics_counter(p_counter_path);
}

void OpenXRVendorPerformanceMetrics::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRVendorPerformanceMetrics::is_enabled);

	ClassDB::bind_method(D_METHOD("set_capture_performance_metrics", "enabled"), &OpenXRVendorPerformanceMetrics::set_capture_performance_metrics);
	ClassDB::bind_method(D_METHOD("is_capturing_performance_metrics"), &OpenXRVendorPerformanceMetrics::is_capturing_performance_metrics);

	ClassDB::bind_method(D_METHOD("get_performance_metrics_counter_paths"), &OpenXRVendorPerformanceMetrics::get_performance_metrics_counter_paths);

	ClassDB::bind_method(D_METHOD("query_performance_metrics_counter", "counter_path"), &OpenXRVendorPerformanceMetrics::query_performance_metrics_counter);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "capture_performance_metrics", PROPERTY_HINT_NONE, ""), "set_capture_performance_metrics", "is_capturing_performance_metrics");

	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_FLAGS_ANY_VALUE_VALID_BIT)
	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_FLAGS_UINT_VALUE_VALID_BIT)
	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_FLAGS_FLOAT_VALUE_VALID_BIT)

	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_UNIT_GENERIC)
	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_UNIT_PERCENTAGE)
	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_UNIT_MILLISECONDS)
	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_UNIT_BYTES)
	BIND_ENUM_CONSTANT(PERFORMANCE_METRICS_COUNTER_UNIT_HERTZ)
}
