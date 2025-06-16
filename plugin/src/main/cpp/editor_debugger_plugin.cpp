/**************************************************************************/
/*  editor_debugger_plugin.cpp                                                     */
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

#include "editor_debugger_plugin.h"

#include <godot_cpp/classes/editor_interface.hpp>

void OpenXRVendorsEditorDebuggerPlugin::_bind_methods() {
}

bool OpenXRVendorsEditorDebuggerPlugin::_has_capture(const String &p_capture) const {
	return p_capture == "godot_openxr_vendors";
}

bool OpenXRVendorsEditorDebuggerPlugin::_capture(const String &p_message, const Array &p_data, int32_t p_session_id) {
	if (p_message == "godot_openxr_vendors:request_run_scene") {
		ERR_FAIL_COND_V(p_data.size() != 2, false);
		override_arguments = p_data[1];
		callable_mp(EditorInterface::get_singleton(), &EditorInterface::play_custom_scene).call_deferred(p_data[0]);
		return true;
	}

	return false;
}

OpenXRVendorsEditorDebuggerPlugin::OpenXRVendorsEditorDebuggerPlugin() {
}
