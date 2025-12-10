/**************************************************************************/
/*  android_xr_export_plugin.h                                            */
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

class AndroidXREditorExportPlugin : public OpenXRVendorsEditorExportPlugin {
	GDCLASS(AndroidXREditorExportPlugin, OpenXRVendorsEditorExportPlugin)

	static const int EYE_TRACKING_OPTIONAL_VALUE = 0;
	static const int EYE_TRACKING_REQUIRED_VALUE = 1;

	static const int FACE_TRACKING_OPTIONAL_VALUE = 0;
	static const int FACE_TRACKING_REQUIRED_VALUE = 1;

	static const int HAND_TRACKING_OPTIONAL_VALUE = 0;
	static const int HAND_TRACKING_REQUIRED_VALUE = 1;

	static const int TRACKED_CONTROLLERS_NONE_VALUE = 0;
	static const int TRACKED_CONTROLLERS_OPTIONAL_VALUE = 1;
	static const int TRACKED_CONTROLLERS_REQUIRED_VALUE = 2;

	static const int RECOMMENDED_BOUNDARY_TYPE_NONE_VALUE = 0;
	static const int RECOMMENDED_BOUNDARY_TYPE_LARGE_VALUE = 1;

public:
	AndroidXREditorExportPlugin();

	TypedArray<Dictionary> _get_export_options(const Ref<EditorExportPlatform> &platform) const override;

	PackedStringArray _get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const override;

	String _get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const override;
	bool _get_export_option_visibility(const Ref<EditorExportPlatform> &p_platform, const String &p_option) const override;
	bool _should_update_export_options(const Ref<EditorExportPlatform> &p_platform) const override;

	String _get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	String _get_android_manifest_application_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	String _get_android_manifest_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;

protected:
	static void _bind_methods();

	Dictionary _eye_tracking_option;
	Dictionary _face_tracking_option;
	Dictionary _hand_tracking_option;
	Dictionary _tracked_controllers_option;
	Dictionary _recommended_boundary_type_option;

	mutable bool _should_update_options = false;

	void _project_settings_changed() {
		_should_update_options = true;
	}
};
