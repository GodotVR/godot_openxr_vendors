/**************************************************************************/
/*  openxr_performance_settings_extension_wrapper.h                       */
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

#ifndef OPENXR_PERFORMANCE_SETTINGS_EXTENSION_WRAPPER_H
#define OPENXR_PERFORMANCE_SETTINGS_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Performance levels for easier access
enum PerfSettingsLevel {
    PERF_SETTINGS_LEVEL_POWER_SAVINGS = 0,    // Maximum power savings
    PERF_SETTINGS_LEVEL_SUSTAINED_LOW = 1,    // Decreased performance for sustained use
    PERF_SETTINGS_LEVEL_SUSTAINED_HIGH = 2,   // Increased performance for sustained use
    PERF_SETTINGS_LEVEL_BOOST = 3             // Maximum performance for burst use
};

class OpenXRPerformanceSettingsExtensionWrapper : public OpenXRExtensionWrapperExtension {
    GDCLASS(OpenXRPerformanceSettingsExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
    godot::Dictionary _get_requested_extensions() override;
    void _on_instance_created(uint64_t instance) override;
    void _on_instance_destroyed() override;

    static OpenXRPerformanceSettingsExtensionWrapper *get_singleton();

    bool is_performance_settings_supported() const { return performance_settings_ext; }
    bool set_cpu_level(PerfSettingsLevel p_level);
    bool set_gpu_level(PerfSettingsLevel p_level); 

    OpenXRPerformanceSettingsExtensionWrapper();
    ~OpenXRPerformanceSettingsExtensionWrapper();

protected:
    static void _bind_methods();

private:
    EXT_PROTO_XRRESULT_FUNC3(xrPerfSettingsSetPerformanceLevelEXT,
            (XrSession), session,
            (XrPerfSettingsDomainEXT), domain,
            (XrPerfSettingsLevelEXT), level);

    bool initialize_performance_settings_extension(XrInstance p_instance);
    void cleanup();

    static OpenXRPerformanceSettingsExtensionWrapper *singleton;
    std::map<godot::String, bool *> request_extensions;
    bool performance_settings_ext = false;
};

VARIANT_ENUM_CAST(PerfSettingsLevel);

#endif // OPENXR_PERFORMANCE_SETTINGS_EXTENSION_WRAPPER_H