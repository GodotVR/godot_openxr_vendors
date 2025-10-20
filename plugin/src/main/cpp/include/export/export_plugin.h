/**************************************************************************/
/*  export_plugin.h                                                       */
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

#include "classes/openxr_hybrid_app.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/editor_export_platform.hpp>
#include <godot_cpp/classes/editor_export_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/ref.hpp>

using namespace godot;

static const char *PLUGIN_VERSION = "4.2.0-dev"; // Also update 'config.gradle#defaultVersion'

// Set of supported vendors
static const char *META_VENDOR_NAME = "meta";
static const char *PICO_VENDOR_NAME = "pico";
static const char *LYNX_VENDOR_NAME = "lynx";
static const char *KHRONOS_VENDOR_NAME = "khronos";
static const char *MAGICLEAP_VENDOR_NAME = "magicleap";

static const char *VENDORS_LIST[] = {
	META_VENDOR_NAME,
	PICO_VENDOR_NAME,
	LYNX_VENDOR_NAME,
	KHRONOS_VENDOR_NAME,
	MAGICLEAP_VENDOR_NAME,
};

// Set of custom feature tags supported by the plugin
static const char *EYE_GAZE_INTERACTION_FEATURE = "XR_EXT_eye_gaze_interaction";
static const char *HYBRID_APP_FEATURE = "godot_openxr_hybrid_app";

static const int REGULAR_MODE_VALUE = 0;
static const int OPENXR_MODE_VALUE = 1;

static const int MANIFEST_FALSE_VALUE = 0;
static const int MANIFEST_TRUE_VALUE = 1;

/// Base class for the vendor editor export plugin
class OpenXRVendorsEditorExportPlugin : public EditorExportPlugin {
	GDCLASS(OpenXRVendorsEditorExportPlugin, EditorExportPlugin)

public:
	OpenXRVendorsEditorExportPlugin();

	String _get_name() const override;

	TypedArray<Dictionary> _get_export_options(const Ref<EditorExportPlatform> &platform) const override;
	Dictionary _get_export_options_overrides(const Ref<EditorExportPlatform> &p_platform) const override;

	PackedStringArray _get_export_features(const Ref<EditorExportPlatform> &platform, bool debug) const override;

	String _get_export_option_warning(const Ref<EditorExportPlatform> &platform, const String &option) const override;

	bool _supports_platform(const Ref<EditorExportPlatform> &platform) const override;

	PackedStringArray _get_android_dependencies(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	PackedStringArray _get_android_dependencies_maven_repos(const Ref<EditorExportPlatform> &platform, bool debug) const override;
	PackedStringArray _get_android_libraries(const Ref<EditorExportPlatform> &platform, bool debug) const override;

	String _get_android_manifest_activity_element_contents(const Ref<EditorExportPlatform> &platform, bool debug) const override;

	void set_vendor_name(const String &vendor_name) {
		_vendor = vendor_name;
	}

protected:
	static void _bind_methods();

	static Dictionary _generate_export_option(const String &name, const String &class_name,
			Variant::Type type,
			PropertyHint property_hint,
			const String &hint_string,
			PropertyUsageFlags property_usage,
			const Variant &default_value,
			bool update_visibility);

	Dictionary _get_vendor_toggle_option() const {
		return _get_vendor_toggle_option(_vendor);
	}

	Dictionary _get_vendor_toggle_option(const String &vendor_name) const;

	bool _is_hybrid_app_enabled() const;
	OpenXRHybridApp::HybridMode _get_hybrid_app_launch_mode() const;

	String _get_opening_activity_tag_for_panel_app() const;

	String _get_common_activity_intent_filter_contents() const;

	bool _is_openxr_enabled() const;

	bool _get_bool_option(const String &option) const;

	int _get_int_option(const String &option, int default_value) const;

	String _bool_to_string(bool p_value) const;

	String _get_android_orientation_label(DisplayServer::ScreenOrientation screen_orientation) const;

	bool _is_vendor_plugin_enabled(const String &vendor_name) const {
		return _get_bool_option(_get_vendor_toggle_option_name(vendor_name));
	}

	bool _is_vendor_plugin_enabled() const {
		return _is_vendor_plugin_enabled(_vendor);
	}

	bool _is_android_aar_file_available(bool debug) const {
		return FileAccess::file_exists(_get_android_aar_file_path(debug));
	}

private:
	/// Path to the Android library aar file. If the return file path is not available, we
	/// fall back to the maven central dependency.
	String _get_android_aar_file_path(bool debug) const;

	/// Maven central dependency used as fall back when the Android library aar file is not
	/// available.
	String _get_android_maven_central_dependency() const;

	String _get_vendor_toggle_option_name() const {
		return _get_vendor_toggle_option_name(_vendor);
	}

	String _get_vendor_toggle_option_name(const String &vendor_name) const;

	String _vendor;
	const String _plugin_version;
};
