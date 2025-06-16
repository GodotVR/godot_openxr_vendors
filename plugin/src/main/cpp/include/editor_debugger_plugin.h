/**************************************************************************/
/*  editor_debugger_plugin.h                                              */
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

#include <godot_cpp/classes/editor_debugger_plugin.hpp>

using namespace godot;

class OpenXRVendorsEditorDebuggerPlugin : public EditorDebuggerPlugin {
	GDCLASS(OpenXRVendorsEditorDebuggerPlugin, EditorDebuggerPlugin)

	PackedStringArray override_arguments;

protected:
	static void _bind_methods();

public:
	virtual bool _has_capture(const String &p_capture) const override;
	virtual bool _capture(const String &p_message, const Array &p_data, int32_t p_session_id) override;

	inline const PackedStringArray &get_override_arguments() const { return override_arguments; }
	inline void clear_override_arguments() { override_arguments.clear(); }

	OpenXRVendorsEditorDebuggerPlugin();
};
