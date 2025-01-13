/**************************************************************************/
/*  openxr_performance_settings_extension_wrapper.cpp                      */
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

#include "extensions/openxr_performance_settings_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRPerformanceSettingsExtensionWrapper *OpenXRPerformanceSettingsExtensionWrapper::singleton = nullptr;

OpenXRPerformanceSettingsExtensionWrapper *OpenXRPerformanceSettingsExtensionWrapper::get_singleton() {
    if (singleton == nullptr) {
        singleton = memnew(OpenXRPerformanceSettingsExtensionWrapper());
    }
    return singleton;
}

OpenXRPerformanceSettingsExtensionWrapper::OpenXRPerformanceSettingsExtensionWrapper() :
        OpenXRExtensionWrapperExtension() {
    ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRPerformanceSettingsExtensionWrapper singleton already exists.");

    request_extensions[XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME] = &performance_settings_ext;
    singleton = this;
}

OpenXRPerformanceSettingsExtensionWrapper::~OpenXRPerformanceSettingsExtensionWrapper() {
    cleanup();
}

void OpenXRPerformanceSettingsExtensionWrapper::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_performance_settings_supported"), &OpenXRPerformanceSettingsExtensionWrapper::is_performance_settings_supported);
    ClassDB::bind_method(D_METHOD("set_cpu_level", "level"), &OpenXRPerformanceSettingsExtensionWrapper::set_cpu_level);
    ClassDB::bind_method(D_METHOD("set_gpu_level", "level"), &OpenXRPerformanceSettingsExtensionWrapper::set_gpu_level);

    BIND_ENUM_CONSTANT(PERF_SETTINGS_LEVEL_POWER_SAVINGS);
    BIND_ENUM_CONSTANT(PERF_SETTINGS_LEVEL_SUSTAINED_LOW);
    BIND_ENUM_CONSTANT(PERF_SETTINGS_LEVEL_SUSTAINED_HIGH);
    BIND_ENUM_CONSTANT(PERF_SETTINGS_LEVEL_BOOST);
}

void OpenXRPerformanceSettingsExtensionWrapper::cleanup() {
    performance_settings_ext = false;
}

Dictionary OpenXRPerformanceSettingsExtensionWrapper::_get_requested_extensions() {
    Dictionary result;
    for (const auto& it : request_extensions) {
        result[it.first] = reinterpret_cast<uint64_t>(it.second);
    }
    return result;
}

void OpenXRPerformanceSettingsExtensionWrapper::_on_instance_created(uint64_t instance) {
    if (performance_settings_ext) {
        bool result = initialize_performance_settings_extension((XrInstance)instance);
        if (!result) {
            UtilityFunctions::printerr("Failed to initialize performance_settings extension");
            performance_settings_ext = false;
        }
    }
}

void OpenXRPerformanceSettingsExtensionWrapper::_on_instance_destroyed() {
    cleanup();
}

bool OpenXRPerformanceSettingsExtensionWrapper::initialize_performance_settings_extension(XrInstance p_instance) {
    GDEXTENSION_INIT_XR_FUNC_V(xrPerfSettingsSetPerformanceLevelEXT);
    return true;
}

XrPerfSettingsLevelEXT level_to_openxr(PerfSettingsLevel p_level) {
    // Convert our enum to OpenXR enum
    switch (p_level) {
        case PERF_SETTINGS_LEVEL_POWER_SAVINGS:
            return XR_PERF_SETTINGS_LEVEL_POWER_SAVINGS_EXT;
            break;
        case PERF_SETTINGS_LEVEL_SUSTAINED_LOW:
            return XR_PERF_SETTINGS_LEVEL_SUSTAINED_LOW_EXT;
            break;
        case PERF_SETTINGS_LEVEL_SUSTAINED_HIGH:
            return XR_PERF_SETTINGS_LEVEL_SUSTAINED_HIGH_EXT;
        case PERF_SETTINGS_LEVEL_BOOST:
            return XR_PERF_SETTINGS_LEVEL_BOOST_EXT;
        default:
            UtilityFunctions::printerr("Invalid performance settings level");
            return XR_PERF_SETTINGS_LEVEL_POWER_SAVINGS_EXT;
    }
}


bool OpenXRPerformanceSettingsExtensionWrapper::set_gpu_level(PerfSettingsLevel p_level) {
    if (!performance_settings_ext) {
        return false;
    }

    XrPerfSettingsLevelEXT level = level_to_openxr(p_level);

    XrResult result = xrPerfSettingsSetPerformanceLevelEXT(SESSION, XR_PERF_SETTINGS_DOMAIN_GPU_EXT, level);
    if (XR_FAILED(result)) {
        UtilityFunctions::printerr(vformat("xrPerfSettingsSetPerformanceLevelEXT failed: %s", get_openxr_api()->get_error_string(result)));
        return false;
    }

    return true;
}

bool OpenXRPerformanceSettingsExtensionWrapper::set_cpu_level(PerfSettingsLevel p_level) {
    if (!performance_settings_ext) {
        return false;
    }

    XrPerfSettingsLevelEXT level = level_to_openxr(p_level);

    XrResult result = xrPerfSettingsSetPerformanceLevelEXT(SESSION, XR_PERF_SETTINGS_DOMAIN_CPU_EXT, level);
    if (XR_FAILED(result)) {
        UtilityFunctions::printerr(vformat("xrPerfSettingsSetPerformanceLevelEXT failed: %s", get_openxr_api()->get_error_string(result)));
        return false;
    }

    return true;
}