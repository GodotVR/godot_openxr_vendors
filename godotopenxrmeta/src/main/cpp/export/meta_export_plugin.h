/**************************************************************************/
/*  meta_editor_plugin.h                                                  */
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

#include "export/export_plugin.h"

using namespace godot;

namespace {

static const int EYE_TRACKING_NONE_VALUE = 0;
static const int EYE_TRACKING_OPTIONAL_VALUE = 1;
static const int EYE_TRACKING_REQUIRED_VALUE = 2;

static const int PASSTHROUGH_NONE_VALUE = 0;
static const int PASSTHROUGH_OPTIONAL_VALUE = 1;
static const int PASSTHROUGH_REQUIRED_VALUE = 2;

static const int HAND_TRACKING_NONE_VALUE = 0;
static const int HAND_TRACKING_OPTIONAL_VALUE = 1;
static const int HAND_TRACKING_REQUIRED_VALUE = 2;

static const int HAND_TRACKING_FREQUENCY_LOW_VALUE = 0;
static const int HAND_TRACKING_FREQUENCY_HIGH_VALUE = 1;

static const int BOUNDARY_ENABLED_VALUE = 0;
static const int BOUNDARY_DISABLED_VALUE = 1;
static const int BOUNDARY_CONTEXTUAL_VALUE = 2;

} // namespace

class MetaEditorExportPlugin : public OpenXREditorExportPlugin {
	GDCLASS(MetaEditorExportPlugin, OpenXREditorExportPlugin)

public:
	MetaEditorExportPlugin();

	TypedArray<Dictionary> _get_export_options(const Ref<EditorExportPlatform> &platform) const override;

	PackedStringArray _get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const override;

	String _get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const override;

	String _get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	String _get_android_manifest_application_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	String _get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;

protected:
	static void _bind_methods();

private:
	PackedStringArray _get_supported_devices() const;

	bool _is_eye_tracking_enabled() const;

	Dictionary _eye_tracking_option;
	Dictionary _hand_tracking_option;
	Dictionary _hand_tracking_frequency_option;
	Dictionary _passthrough_option;
	Dictionary _use_anchor_api_option;
	Dictionary _use_scene_api_option;
	Dictionary _use_overlay_keyboard_option;
	Dictionary _use_experimental_features_option;
	Dictionary _boundary_mode_option;
	Dictionary _support_quest_1_option;
	Dictionary _support_quest_2_option;
	Dictionary _support_quest_3_option;
	Dictionary _support_quest_pro_option;
};

class MetaEditorPlugin : public EditorPlugin {
	GDCLASS(MetaEditorPlugin, EditorPlugin)

public:
	void _enter_tree() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();

private:
	MetaEditorExportPlugin *meta_export_plugin = nullptr;

};
