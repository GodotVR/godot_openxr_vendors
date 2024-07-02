/**************************************************************************/
/*  khronos_editor_plugin.h                                               */
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

#include <godot_cpp/classes/editor_plugin.hpp>

#include "export_plugin.h"

using namespace godot;

static const int KHRONOS_VENDOR_OTHER = 0;
static const int KHRONOS_VENDOR_HTC = 1;

static const int MANIFEST_FALSE_VALUE = 0;
static const int MANIFEST_TRUE_VALUE = 1;

class KhronosEditorExportPlugin : public OpenXREditorExportPlugin {
	GDCLASS(KhronosEditorExportPlugin, OpenXREditorExportPlugin)

public:
	KhronosEditorExportPlugin();

	TypedArray<Dictionary> _get_export_options(const Ref<EditorExportPlatform> &platform) const override;

	Dictionary _get_export_options_overrides(const Ref<EditorExportPlatform> &p_platform) const override;

	PackedStringArray _get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const override;

	String _get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const override;

	String _get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	String _get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;

protected:
	static void _bind_methods();

	bool _is_khronos_htc_enabled() const {
		return _get_int_option("khronos_xr_features/vendors", KHRONOS_VENDOR_OTHER) == KHRONOS_VENDOR_HTC;
	}

	Dictionary _khronos_vendors_option;
	Dictionary _hand_tracking_option;
	Dictionary _tracker_option;
	Dictionary _eye_tracking_option;
	Dictionary _lip_expression_option;
};

class KhronosEditorPlugin : public EditorPlugin {
	GDCLASS(KhronosEditorPlugin, EditorPlugin)

public:
	void _enter_tree() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();

private:
	Ref<KhronosEditorExportPlugin> khronos_export_plugin;
};
