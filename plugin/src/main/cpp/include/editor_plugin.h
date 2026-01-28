/**************************************************************************/
/*  editor_plugin.h                                                       */
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

#include <godot_cpp/classes/editor_export_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>

#include "editor_debugger_plugin.h"

using namespace godot;
class XrProjectSetupDialog;

namespace godot {
class LineEdit;
}

class OpenXRVendorsEditorPlugin : public EditorPlugin {
	GDCLASS(OpenXRVendorsEditorPlugin, EditorPlugin)

	static OpenXRVendorsEditorPlugin *singleton;

	Vector<Ref<EditorExportPlugin>> export_plugins;
	Ref<OpenXRVendorsEditorDebuggerPlugin> debugger_plugin;

	XrProjectSetupDialog *_xr_project_setup_dialog = nullptr;

	void _add_export_plugin(const Ref<EditorExportPlugin> &p_plugin);

	void _open_project_setup();

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	static OpenXRVendorsEditorPlugin *get_singleton();

	void open_asset_library(const String &p_filter_string);
	void _on_asset_library_request_completed(int p_result, int p_response_code, const PackedStringArray &p_headers, const PackedByteArray &p_body, LineEdit *p_asset_library_filter, String p_search_string);

	void open_project_settings(const String &p_filter_string);

	void open_export_dialog();

	PackedStringArray _run_scene(const String &p_scene, const PackedStringArray &p_args) const override;

	OpenXRVendorsEditorPlugin();
	~OpenXRVendorsEditorPlugin();
};
