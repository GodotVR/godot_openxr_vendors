/**************************************************************************/
/*  openxr_session_helper_extension.cpp                                   */
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

#include "extensions/openxr_session_helper_extension.h"

#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openxr_android_xrs_utils.h"

using namespace godot;

OpenXRSessionHelperExtension *OpenXRSessionHelperExtension::singleton = nullptr;

OpenXRSessionHelperExtension *OpenXRSessionHelperExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRSessionHelperExtension());
	}
	return singleton;
}

OpenXRSessionHelperExtension::OpenXRSessionHelperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRSessionHelperExtension singleton already exists.");
	singleton = this;
}

OpenXRSessionHelperExtension::~OpenXRSessionHelperExtension() {
	singleton = nullptr;
}

void OpenXRSessionHelperExtension::_on_instance_created(uint64_t instance) {
	GDEXTENSION_INIT_XR_FUNC(xrGetInstanceProperties);

	if (OS::get_singleton()->get_cmdline_args().has("--xr_auto_start_androidxr_streaming_client") && _get_openxr_runtime_name((XrInstance)instance) == OpenXRAndroidXRSUtils::get_xrs_runtime_name()) {
		UtilityFunctions::print_verbose("OpenXRSessionHelperExtension: Android XR Streaming runtime detected, will attempt to start client");
		_should_request_stop_xrs_client = OpenXRAndroidXRSUtils::try_init_xrs_client();
	}
}

void OpenXRSessionHelperExtension::_on_state_ready() {
	if (_should_request_stop_xrs_client) {
		EngineDebugger *engine_debugger = EngineDebugger::get_singleton();
		if (engine_debugger && engine_debugger->is_active()) {
			engine_debugger->send_message("godot_openxr_vendors:request_stop_xrs_client_on_session_end", {});
			_should_request_stop_xrs_client = false;
		}
	}
}

void OpenXRSessionHelperExtension::_bind_methods() {}

String OpenXRSessionHelperExtension::_get_openxr_runtime_name(XrInstance instance) {
	XrInstanceProperties instanceProps = {
		XR_TYPE_INSTANCE_PROPERTIES, // type;
		nullptr, // next
		0, // runtimeVersion, from here will be set by our get call
		"" // runtimeName
	};

	XrResult result = xrGetInstanceProperties((XrInstance)instance, &instanceProps);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("OpenXRSessionHelperExtension: xrGetInstanceProperties failed: %s", get_openxr_api()->get_error_string(result)));
		return "";
	}

	return instanceProps.runtimeName;
}
