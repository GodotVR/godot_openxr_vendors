/**************************************************************************/
/*  openxr_vendor_performance_metrics.h                                   */
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

#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/binder_common.hpp>

class OpenXRVendorPerformanceMetricsProvider;

using namespace godot;

class OpenXRVendorPerformanceMetrics : public Object {
	GDCLASS(OpenXRVendorPerformanceMetrics, Object);

public:
	enum PerformanceMetricsCounterFlags {
		PERFORMANCE_METRICS_COUNTER_FLAGS_ANY_VALUE_VALID_BIT = 1,
		PERFORMANCE_METRICS_COUNTER_FLAGS_UINT_VALUE_VALID_BIT = 2,
		PERFORMANCE_METRICS_COUNTER_FLAGS_FLOAT_VALUE_VALID_BIT = 4,
	};

	enum PerformanceMetricsCounterUnit {
		PERFORMANCE_METRICS_COUNTER_UNIT_GENERIC,
		PERFORMANCE_METRICS_COUNTER_UNIT_PERCENTAGE,
		PERFORMANCE_METRICS_COUNTER_UNIT_MILLISECONDS,
		PERFORMANCE_METRICS_COUNTER_UNIT_BYTES,
		PERFORMANCE_METRICS_COUNTER_UNIT_HERTZ,
	};

	OpenXRVendorPerformanceMetrics();
	~OpenXRVendorPerformanceMetrics();

	static OpenXRVendorPerformanceMetrics *get_singleton();

	bool is_enabled();

	void set_vendor_performance_metrics_provider(OpenXRVendorPerformanceMetricsProvider *p_provider);

	void set_capture_performance_metrics(bool p_enabled);
	bool is_capturing_performance_metrics();

	PackedStringArray get_performance_metrics_counter_paths();

	Dictionary query_performance_metrics_counter(const String &p_counter_path);

protected:
	static void _bind_methods();

private:
	static OpenXRVendorPerformanceMetrics *singleton;

	OpenXRVendorPerformanceMetricsProvider *provider = nullptr;

	bool capture_performance_metrics = false;

	bool custom_monitors_added = false;
};

VARIANT_ENUM_CAST(OpenXRVendorPerformanceMetrics::PerformanceMetricsCounterFlags)
VARIANT_ENUM_CAST(OpenXRVendorPerformanceMetrics::PerformanceMetricsCounterUnit)
