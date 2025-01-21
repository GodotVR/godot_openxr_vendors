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
#include <godot_cpp/classes/os.hpp>
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

bool OpenXRHybridApp::is_hybrid_app() const {
	OS *os = OS::get_singleton();
	ERR_FAIL_NULL_V(os, false);

	return os->has_feature("godot_openxr_hybrid_app");
}

OpenXRHybridApp::HybridMode OpenXRHybridApp::get_mode() const {
	OS *os = OS::get_singleton();
	ERR_FAIL_NULL_V(os, HYBRID_MODE_NONE);

	if (!is_hybrid_app()) {
		return HYBRID_MODE_NONE;
	}

	if (os->has_feature("godot_openxr_panel_app")) {
		return HYBRID_MODE_PANEL;
	}

	return HYBRID_MODE_IMMERSIVE;
}

bool OpenXRHybridApp::switch_mode(HybridMode p_mode, const String &p_data) {
	ERR_FAIL_COND_V(p_mode == HYBRID_MODE_NONE, false);

	if (!is_hybrid_app()) {
		return false;
	}

	Object *godot_openxr_singleton = Engine::get_singleton()->get_singleton("GodotOpenXRHybridAppInternal");
	if (!godot_openxr_singleton) {
		return false;
	}

	return godot_openxr_singleton->call("hybridAppSwitchTo", p_mode, p_data);
}

String OpenXRHybridApp::get_launch_data() const {
	if (!is_hybrid_app()) {
		return String();
	}

	Object *godot_openxr_singleton = Engine::get_singleton()->get_singleton("GodotOpenXRHybridAppInternal");
	if (!godot_openxr_singleton) {
		return String();
	}

	return godot_openxr_singleton->call("getHybridAppLaunchData");
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
