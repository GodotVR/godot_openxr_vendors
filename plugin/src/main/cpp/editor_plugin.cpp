/**************************************************************************/
/*  editor_plugin.cpp                                                     */
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

#include "editor_plugin.h"

#include <godot_cpp/classes/project_settings.hpp>

#include "export/khronos_export_plugin.h"
#include "export/lynx_export_plugin.h"
#include "export/magicleap_export_plugin.h"
#include "export/meta_export_plugin.h"
#include "export/pico_export_plugin.h"

using namespace godot;

void OpenXRVendorsEditorPlugin::_add_export_plugin(const Ref<EditorExportPlugin> &p_plugin) {
	add_export_plugin(p_plugin);
	export_plugins.push_back(p_plugin);
}

void OpenXRVendorsEditorPlugin::_bind_methods() {
}

void OpenXRVendorsEditorPlugin::_enter_tree() {
	Ref<KhronosEditorExportPlugin> khronos_export_plugin;
	khronos_export_plugin.instantiate();
	_add_export_plugin(khronos_export_plugin);

	Ref<LynxEditorExportPlugin> lynx_export_plugin;
	lynx_export_plugin.instantiate();
	_add_export_plugin(lynx_export_plugin);

	Ref<MagicleapEditorExportPlugin> magicleap_export_plugin;
	magicleap_export_plugin.instantiate();
	_add_export_plugin(magicleap_export_plugin);

	Ref<MetaEditorExportPlugin> meta_export_plugin;
	meta_export_plugin.instantiate();
	_add_export_plugin(meta_export_plugin);

	Ref<PicoEditorExportPlugin> pico_export_plugin;
	pico_export_plugin.instantiate();
	_add_export_plugin(pico_export_plugin);

	debugger_plugin.instantiate();
	add_debugger_plugin(debugger_plugin);
}

void OpenXRVendorsEditorPlugin::_exit_tree() {
	for (const Ref<EditorExportPlugin> &export_plugin : export_plugins) {
		remove_export_plugin(export_plugin);
	}
	export_plugins.clear();

	remove_debugger_plugin(debugger_plugin);
	debugger_plugin.unref();
}

PackedStringArray OpenXRVendorsEditorPlugin::_run_scene(const String &p_scene, const PackedStringArray &p_args) const {
	if (!ProjectSettings::get_singleton()->get_setting_with_override("xr/hybrid_app/enabled")) {
		return p_args;
	}

	// If the XR mode is set explicitly, then let it be.
	if (p_args.find("--xr-mode") != -1) {
		return p_args;
	}

	const PackedStringArray &override_arguments = debugger_plugin->get_override_arguments();

	PackedStringArray new_args = p_args;
	if (override_arguments.size() > 0) {
		new_args.append_array(override_arguments);
		debugger_plugin->clear_override_arguments();
	} else {
		OpenXRHybridApp::HybridMode hybrid_mode = (OpenXRHybridApp::HybridMode)(int)ProjectSettings::get_singleton()->get_setting_with_override("xr/hybrid_app/launch_mode");

		if (hybrid_mode == OpenXRHybridApp::HYBRID_MODE_IMMERSIVE) {
			new_args.push_back("--xr-mode");
			new_args.push_back("on");
			new_args.push_back("--xr_mode_openxr");
		} else if (hybrid_mode == OpenXRHybridApp::HYBRID_MODE_PANEL) {
			new_args.push_back("--xr-mode");
			new_args.push_back("off");
			new_args.push_back("--xr_mode_regular");
		}
	}

	return new_args;
}

OpenXRVendorsEditorPlugin::OpenXRVendorsEditorPlugin() {
}
