/**************************************************************************/
/*  openxr_android_xrs_utils.cpp                                          */
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

#include "openxr_android_xrs_utils.h"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/os.hpp>

constexpr char XRS_RUNTIME_NAME[] = "Android XR Streaming";

constexpr char XRS_CLIENT_PACKAGE[] = "com.google.vr.streaming.client";
constexpr char XRS_CLIENT_ACTIVITY[] = "com.google.vr.streaming.client.XrStreamingActivity";

using namespace godot;

namespace {
String get_android_home_from_environment() {
	return OS::get_singleton()->get_environment("ANDROID_HOME");
}

String get_android_home_from_editor_settings() {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_COND_V(editor_interface == nullptr, "");

	Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
	ERR_FAIL_COND_V(editor_settings.is_null(), "");

	return editor_settings->get_setting("export/android/android_sdk_path");
}

// This method is called from the debugged application as well as the editor process
String get_adb_path() {
	// In the editor, we have access to Editor settings and can get adb path from there.
	// But, in the debugged application, we rely on environment variables for getting adb path.
	// This is because EditorSettings interface is not available when this method is called.
	String android_home = OS::get_singleton()->has_feature("editor_hint") ? get_android_home_from_editor_settings() : get_android_home_from_environment();
	ERR_FAIL_COND_V(android_home.is_empty(), "");

	String exe_ext;
	if (OS::get_singleton()->get_name() == "Windows") {
		exe_ext = ".exe";
	}

	String adb_path = android_home.path_join("platform-tools/adb" + exe_ext);
	return adb_path;
}
} // anonymous namespace

namespace OpenXRAndroidXRSUtils {
// Function returns true only if it was responsible for starting the AndroidXR streaming client.
// If client was not found or client was already running, it returns false to indicate stopping the client should not be attempted.
bool try_init_xrs_client() {
	String adb = get_adb_path();
	ERR_FAIL_COND_V(adb.is_empty(), false);

	// Ensure the server is up and running for further commands, we don't care about output here.
	OS::get_singleton()->execute(adb, { "start-server" });

	Array output;
	OS::get_singleton()->execute(adb, { "shell", "pm list packages" }, output);
	if (!((String)output.back()).contains(XRS_CLIENT_PACKAGE)) {
		UtilityFunctions::printerr("OpenXRSessionHelperExtension: Android XR Streaming client not installed on device");
		return false;
	}

	output.clear();
	OS::get_singleton()->execute(adb, { "shell", "dumpsys activity activities | grep ResumedActivity" }, output);
	if (((String)output.back()).contains(XRS_CLIENT_PACKAGE)) {
		// AndroidXR streaming client already running and in focus, doing nothing
		return false;
	}

	output.clear();
	OS::get_singleton()->execute(adb, { "shell", "dumpsys activity activities | grep ActivityRecord" }, output);
	bool client_is_already_running = ((String)output.back()).contains(XRS_CLIENT_PACKAGE);

	// This starts the client if it was not running before, or brings it to focus if it was already running.
	UtilityFunctions::print_verbose("Attempting to start AndroidXR streaming client on connected device");
	OS::get_singleton()->execute_with_pipe(adb, { "shell", vformat("am start -n %s/%s", XRS_CLIENT_PACKAGE, XRS_CLIENT_ACTIVITY) }, false);

	// return value indicates if this function was responsible for starting the client.
	return !client_is_already_running;
}

void cleanup_xrs_client() {
	String adb = get_adb_path();
	ERR_FAIL_COND(adb.is_empty());

	PackedStringArray args = { "shell", vformat("am force-stop %s", XRS_CLIENT_PACKAGE) };
	OS::get_singleton()->execute(adb, args);
}

String get_xrs_runtime_name() {
	return XRS_RUNTIME_NAME;
}
} //namespace OpenXRAndroidXRSUtils
