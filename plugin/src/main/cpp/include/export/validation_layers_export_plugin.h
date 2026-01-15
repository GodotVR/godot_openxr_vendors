/**************************************************************************/
/*  validation_layers_export_plugin.h                                     */
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
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class OpenXRValidationLayersEditorExportPlugin : public EditorExportPlugin {
	GDCLASS(OpenXRValidationLayersEditorExportPlugin, EditorExportPlugin)

	Dictionary _enable_export_option;

protected:
	static void _bind_methods() {}

public:
	OpenXRValidationLayersEditorExportPlugin();

	String _get_name() const override;

	bool _supports_platform(const Ref<EditorExportPlatform> &platform) const override;

	String _get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const override;
	TypedArray<Dictionary> _get_export_options(const Ref<EditorExportPlatform> &platform) const override;

	PackedStringArray _get_android_dependencies(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	PackedStringArray _get_android_dependencies_maven_repos(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	PackedStringArray _get_android_libraries(const Ref<EditorExportPlatform> &platform, bool debug) const;

private:
	/// Path to the Android library aar file. If the return file path is not available, we
	/// fall back to the maven central dependency.
	String _get_android_aar_file_path(bool debug) const;

	bool _is_android_aar_file_available(bool debug) const {
		return FileAccess::file_exists(_get_android_aar_file_path(debug));
	}

	bool _is_enabled() const {
		return (bool)get_option("xr_features/enable_openxr_validation_layers");
	}

	String _get_version() const;
};
