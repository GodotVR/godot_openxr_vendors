/**************************************************************************/
/*  openxr_meta_performance_metrics_extension_wrapper.h                   */
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

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <map>

#include "util.h"

using namespace godot;

class OpenXRMetaPerformanceMetricsExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMetaPerformanceMetricsExtensionWrapper, OpenXRExtensionWrapperExtension);

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

	static OpenXRMetaPerformanceMetricsExtensionWrapper *get_singleton();

	OpenXRMetaPerformanceMetricsExtensionWrapper();
	~OpenXRMetaPerformanceMetricsExtensionWrapper();

	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;
	void _on_state_ready() override;

	bool is_enabled() { return meta_performance_metrics_ext; }

	void set_capture_performance_metrics(bool p_enabled);
	bool is_capturing_performance_metrics();

	PackedStringArray get_performance_metrics_counter_paths();

	Dictionary query_performance_metrics_counter(const String &p_counter_path);

protected:
	static void
	_bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC2(xrSetPerformanceMetricsStateMETA,
			(XrSession), session,
			(const XrPerformanceMetricsStateMETA *), state);

	EXT_PROTO_XRRESULT_FUNC2(xrGetPerformanceMetricsStateMETA,
			(XrSession), session,
			(XrPerformanceMetricsStateMETA *), state);

	EXT_PROTO_XRRESULT_FUNC4(xrEnumeratePerformanceMetricsCounterPathsMETA,
			(XrInstance), instance,
			(uint32_t), counterPathCapacityInput,
			(uint32_t *), counterPathCountOutput,
			(XrPath *), counterPaths);

	EXT_PROTO_XRRESULT_FUNC3(xrQueryPerformanceMetricsCounterMETA,
			(XrSession), session,
			(XrPath), counterPath,
			(XrPerformanceMetricsCounterMETA *), counter);

	EXT_PROTO_XRRESULT_FUNC5(xrPathToString,
			(XrInstance), instance,
			(XrPath), path,
			(uint32_t), bufferCapacityInput,
			(uint32_t *), bufferCountOutput,
			(char *), buffer);

	bool initialize_meta_performance_metrics_extension(const XrInstance instance);

	void cleanup();

	void populate_performance_metrics_counter_paths();

	Variant get_monitor_data(const String &p_counter_path);

	static OpenXRMetaPerformanceMetricsExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool meta_performance_metrics_ext = false;

	bool counter_paths_populated = false;
	Vector<XrPath> performance_metrics_counter_xr_paths;
	PackedStringArray performance_metrics_counter_paths;

	bool custom_monitors_added = false;

	XrPerformanceMetricsStateMETA performance_metrics_state = {
		XR_TYPE_PERFORMANCE_METRICS_STATE_META, // type
		nullptr, // next
		false // enabled
	};
};

VARIANT_ENUM_CAST(OpenXRMetaPerformanceMetricsExtensionWrapper::PerformanceMetricsCounterFlags)
VARIANT_ENUM_CAST(OpenXRMetaPerformanceMetricsExtensionWrapper::PerformanceMetricsCounterUnit)
