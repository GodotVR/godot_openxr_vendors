/**************************************************************************/
/*  openxr_hybrid_app.cpp                                                 */
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

#include "classes/openxr_hybrid_app.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/class_db.hpp>

OpenXRHybridApp *OpenXRHybridApp::singleton = nullptr;

void OpenXRHybridApp::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_hybrid_app"), &OpenXRHybridApp::is_hybrid_app);
	ClassDB::bind_method(D_METHOD("get_mode"), &OpenXRHybridApp::get_mode);
	ClassDB::bind_method(D_METHOD("switch_mode", "mode", "data"), &OpenXRHybridApp::switch_mode, DEFVAL(String()));
	ClassDB::bind_method(D_METHOD("get_launch_data"), &OpenXRHybridApp::get_launch_data);

	BIND_ENUM_CONSTANT(HYBRID_MODE_NONE);
	BIND_ENUM_CONSTANT(HYBRID_MODE_IMMERSIVE);
	BIND_ENUM_CONSTANT(HYBRID_MODE_PANEL);
}

bool OpenXRHybridApp::is_hybrid_app_enabled() const {
	return ProjectSettings::get_singleton()->get_setting_with_override("xr/hybrid_app/enabled");
}

bool OpenXRHybridApp::is_hybrid_app() const {
#ifdef ANDROID_ENABLED
	return OS::get_singleton()->has_feature("godot_openxr_hybrid_app");
#else
	Ref<OpenXRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	return !Engine::get_singleton()->is_editor_hint() && xr_interface.is_valid() && is_hybrid_app_enabled();
#endif
}

OpenXRHybridApp::HybridMode OpenXRHybridApp::get_mode() const {
	if (!is_hybrid_app()) {
		return HYBRID_MODE_NONE;
	}

#ifdef ANDROID_ENABLED
	if (OS::get_singleton()->has_feature("godot_openxr_panel_app")) {
		return HYBRID_MODE_PANEL;
	}

	return HYBRID_MODE_IMMERSIVE;
#else
	Ref<OpenXRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	return xr_interface->is_initialized() ? HYBRID_MODE_IMMERSIVE : HYBRID_MODE_PANEL;
#endif
}

bool OpenXRHybridApp::switch_mode(HybridMode p_mode, const String &p_data) {
	ERR_FAIL_COND_V(p_mode == HYBRID_MODE_NONE, false);

	if (!is_hybrid_app()) {
		return false;
	}

#ifdef ANDROID_ENABLED
	Object *godot_openxr_singleton = Engine::get_singleton()->get_singleton("GodotOpenXRHybridAppInternal");
	if (!godot_openxr_singleton) {
		return false;
	}

	return godot_openxr_singleton->call("hybridAppSwitchTo", p_mode, p_data);
#else
	OS *os = OS::get_singleton();

	PackedStringArray old_args = os->get_cmdline_args();

	PackedStringArray mode_args;
	if (p_mode == HYBRID_MODE_IMMERSIVE) {
		mode_args.push_back("--xr-mode");
		mode_args.push_back("on");
		mode_args.push_back("--xr_mode_openxr");
	} else {
		mode_args.push_back("--xr-mode");
		mode_args.push_back("off");
		mode_args.push_back("--xr_mode_regular");
	}

	if (!p_data.is_empty()) {
		mode_args.push_back("--openxr-hybrid-data");
		mode_args.push_back(Marshalls::get_singleton()->utf8_to_base64(p_data));
	}

	// This path works when running from the editor.
	EngineDebugger *engine_debugger = EngineDebugger::get_singleton();
	if (engine_debugger && engine_debugger->is_active()) {
		String scene_path;
		for (int i = 0; i < old_args.size() - 1; i++) {
			if (old_args[i] == "--scene") {
				scene_path = old_args[i + 1];
			}
		}

		Array req;
		req.push_back(scene_path);
		req.push_back(mode_args);

		engine_debugger->send_message("godot_openxr_vendors:request_run_scene", req);
		return true;
	}

	// The rest if for when running without being connected to the editor.

	SceneTree *scene_tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	ERR_FAIL_NULL_V(scene_tree, false);

	PackedStringArray new_args;

	bool skip_next = false;
	for (const String &arg : old_args) {
		if (skip_next) {
			skip_next = false;
			continue;
		}
		if (arg == "--xr_mode_openxr" || arg == "--xr_mode_regular") {
			// Skip these.
		} else if (arg == "--xr-mode" || arg == "--openxr-hybrid-data") {
			// Skip these and the next one.
			skip_next = true;
		} else {
			new_args.push_back(arg);
		}
	}

	new_args.append_array(mode_args);

	int pid = os->create_instance(new_args);
	if (pid == -1) {
		return false;
	}

	scene_tree->quit();
	return true;
#endif
}

String OpenXRHybridApp::get_launch_data() const {
	if (!is_hybrid_app()) {
		return String();
	}

#ifdef ANDROID_ENABLED
	Object *godot_openxr_singleton = Engine::get_singleton()->get_singleton("GodotOpenXRHybridAppInternal");
	if (!godot_openxr_singleton) {
		return String();
	}

	return godot_openxr_singleton->call("getHybridAppLaunchData");
#else
	PackedStringArray args = OS::get_singleton()->get_cmdline_args();
	for (int i = 0; i < args.size() - 1; i++) {
		if (args[i] == "--openxr-hybrid-data") {
			return Marshalls::get_singleton()->base64_to_utf8(args[i + 1]);
		}
	}

	return String();
#endif
}

OpenXRHybridApp *OpenXRHybridApp::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRHybridApp());
	}
	return singleton;
}

OpenXRHybridApp::OpenXRHybridApp() :
		Object() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRHybridApp singleton already exists.");

	singleton = this;
}

OpenXRHybridApp::~OpenXRHybridApp() {
	singleton = nullptr;
}
