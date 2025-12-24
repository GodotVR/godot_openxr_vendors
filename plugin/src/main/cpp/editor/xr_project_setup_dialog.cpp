/**************************************************************************/
/*  xr_project_setup_dialog.cpp                                           */
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

#include "editor/xr_project_setup_dialog.h"

#include "editor_plugin.h"
#include "raw_headers/mr_startup.tscn.gen.h"
#include "raw_headers/start_mr.gd.gen.h"
#include "raw_headers/start_vr.gd.gen.h"
#include "raw_headers/vr_startup.tscn.gen.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/center_container.hpp>
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_separator.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

static const char *XR_STARTUP_SCENE_PATH = "res://xr_startup.tscn";
static const char *START_XR_SCRIPT_PATH = "res://start_xr.gd";

enum AlertType {
	ALERT_TYPE_ERROR,
	ALERT_TYPE_WARNING,
	ALERT_TYPE_NONE,
};

enum ProjectType {
	PROJECT_TYPE_ANY,
	PROJECT_TYPE_VR,
	PROJECT_TYPE_MR,
};

enum VendorType {
	VENDOR_TYPE_ANY,
	VENDOR_TYPE_META,
	VENDOR_TYPE_HTC,
	VENDOR_TYPE_PICO,
	VENDOR_TYPE_LYNX,
	VENDOR_TYPE_MAGIC_LEAP,
	VENDOR_TYPE_ANDROID_XR,
	VENDOR_TYPE_VALVE,
};

namespace godot_openxr_vendors {
class Recommendation {
public:
	virtual bool is_recommendation_applied() = 0;
	virtual void on_button_pressed() = 0;

	bool should_display(ProjectType p_project_type, VendorType p_vendor_type) {
		bool matches_project_type = project_type == PROJECT_TYPE_ANY || project_type == p_project_type;
		bool matches_vendor_type = vendor_type == VENDOR_TYPE_ANY || vendor_type == p_vendor_type;
		return matches_project_type && matches_vendor_type && !is_recommendation_applied();
	}

	void show_window_entry() {
		if (!vbox || !icon_container || !description_label || !button) {
			return;
		}

		vbox->show();
		icon_container->show();
		description_label->set_text(description);
		button->set_disabled(false);

		if (alert_type != ALERT_TYPE_NONE) {
			description_label->add_theme_color_override("font_color", font_color);
		}
	}

	void hide_window_entry() {
		if (vbox) {
			vbox->hide();
		}
	}

	String title;
	String description;
	String button_text;
	AlertType alert_type;
	ProjectType project_type;
	VendorType vendor_type;
	Color font_color = Color(1.0, 1.0, 1.0);
	bool requires_restart;

	VBoxContainer *vbox = nullptr;
	CenterContainer *icon_container = nullptr;
	Label *description_label = nullptr;
	Button *button = nullptr;

	Recommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart) :
			title(p_title), description(p_description), button_text(p_button_text), alert_type(p_alert_type), project_type(p_project_type), vendor_type(p_vendor_type), requires_restart(p_requires_restart) {}
};

class SimpleProjectSettingRecommendation : public Recommendation {
public:
	bool is_recommendation_applied() override {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();
		ERR_FAIL_NULL_V(project_settings, false);

		return project_settings->get(setting_path) == recommended_value;
	}

	void on_button_pressed() override {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();
		ERR_FAIL_NULL(project_settings);

		project_settings->set_setting(setting_path, recommended_value);
		project_settings->save();

		button->set_disabled(true);

		if (icon_container != nullptr) {
			icon_container->hide();
		}

		if (alert_type == ALERT_TYPE_ERROR) {
			description_label->set_text("Restart editor to apply updated setting");
		} else {
			description_label->set_text("Setting successfully updated");
		}
	}

	String setting_path;
	Variant recommended_value;

	SimpleProjectSettingRecommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart, String p_setting_path, Variant p_recommended_value) :
			Recommendation(p_title, p_description, p_button_text, p_alert_type, p_project_type, p_vendor_type, p_requires_restart), setting_path(p_setting_path), recommended_value(p_recommended_value) {}
};

class ExportSettingRecommendation : public Recommendation {
public:
	bool is_recommendation_applied() override {
		Ref<ConfigFile> config;
		config.instantiate();
		config->load("res://export_presets.cfg");
		PackedStringArray sections = config->get_sections();

		for (const HashMap<String, Variant> &export_preset_values : export_preset_value_sets) {
			if (!export_preset_values.has("platform")) {
				continue;
			}

			for (const String &section : sections) {
				if (!config->has_section_key(section, "platform")) {
					continue;
				}

				if (config->get_value(section, "platform") == export_preset_values["platform"]) {
					String options_section = section + String(".options");
					if (!sections.has(options_section)) {
						continue;
					}

					bool all_values_set = true;
					for (const KeyValue<String, Variant> &entry : export_preset_values) {
						if (entry.key == "platform") {
							continue;
						}

						if (!config->has_section_key(options_section, entry.key)) {
							all_values_set = false;
							break;
						}

						if (config->get_value(options_section, entry.key) != entry.value) {
							all_values_set = false;
							break;
						}
					}

					if (all_values_set) {
						return true;
					}
				}
			}
		}

		return false;
	}

	void on_button_pressed() override {
		OpenXRVendorsEditorPlugin *editor_plugin = OpenXRVendorsEditorPlugin::get_singleton();
		if (editor_plugin == nullptr) {
			return;
		}

		editor_plugin->open_export_dialog();
	}

	Vector<HashMap<String, Variant>> export_preset_value_sets;

	ExportSettingRecommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart, Vector<HashMap<String, Variant>> p_export_preset_value_sets) :
			Recommendation(p_title, p_description, p_button_text, p_alert_type, p_project_type, p_vendor_type, p_requires_restart), export_preset_value_sets(p_export_preset_value_sets) {}
};

class SdkPathRecommendation : public Recommendation {
public:
	bool is_recommendation_applied() override {
		EditorInterface *editor_interface = EditorInterface::get_singleton();
		if (editor_interface == nullptr) {
			return false;
		}

		Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
		if (editor_settings.is_valid()) {
			String sdk_path = editor_settings->get_setting(sdk_setting_name);
			if (!sdk_path.is_empty() && DirAccess::dir_exists_absolute(sdk_path)) {
				return true;
			}
		}

		return false;
	}

	void on_button_pressed() override {
		OS *os = OS::get_singleton();
		if (os == nullptr) {
			return;
		}

		os->shell_open("https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_android.html");
	}

	String sdk_setting_name;

	SdkPathRecommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart, String p_sdk_setting_name) :
			Recommendation(p_title, p_description, p_button_text, p_alert_type, p_project_type, p_vendor_type, p_requires_restart), sdk_setting_name(p_sdk_setting_name) {}
};

class StartupSceneRecommendation : public Recommendation {
public:
	bool is_recommendation_applied() override {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();
		ERR_FAIL_NULL_V(project_settings, false);

		return project_settings->get("application/run/main_scene") != String("");
	}

	void on_button_pressed() override {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();
		ERR_FAIL_NULL(project_settings);

		EditorInterface *editor_interface = EditorInterface::get_singleton();
		ERR_FAIL_NULL(editor_interface);

		if (FileAccess::file_exists(START_XR_SCRIPT_PATH)) {
			ERR_FAIL_EDMSG(vformat("XR startup script %s already exists", START_XR_SCRIPT_PATH));
		}

		if (FileAccess::file_exists(XR_STARTUP_SCENE_PATH)) {
			ERR_FAIL_EDMSG(vformat("XR startup scene xr_startup.tscn already exists", XR_STARTUP_SCENE_PATH));
		}

		Ref<FileAccess> start_xr = FileAccess::open(START_XR_SCRIPT_PATH, FileAccess::WRITE);
		start_xr->store_string(start_xr_script_content);
		start_xr->close();

		Ref<FileAccess> startup_scene = FileAccess::open(XR_STARTUP_SCENE_PATH, FileAccess::WRITE);
		startup_scene->store_string(xr_startup_scene_content);
		startup_scene->close();

		project_settings->set_setting("application/run/main_scene", XR_STARTUP_SCENE_PATH);
		project_settings->save();

		editor_interface->get_resource_filesystem()->scan();
		editor_interface->open_scene_from_path(XR_STARTUP_SCENE_PATH);

		button->set_disabled(true);
	}

	String xr_startup_scene_content;
	String start_xr_script_content;

	StartupSceneRecommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart, String p_xr_startup_scene_content, String p_start_xr_script_content) :
			Recommendation(p_title, p_description, p_button_text, p_alert_type, p_project_type, p_vendor_type, p_requires_restart), xr_startup_scene_content(p_xr_startup_scene_content), start_xr_script_content(p_start_xr_script_content) {}
};

class BootSplashRecommendation : public Recommendation {
public:
	bool is_recommendation_applied() override {
		ProjectSettings *project_settings = ProjectSettings::get_singleton();
		ERR_FAIL_NULL_V(project_settings, false);

		String boot_splash_path = project_settings->get("application/boot_splash/image");
		return !boot_splash_path.is_empty() && FileAccess::file_exists(boot_splash_path);
	}

	void on_button_pressed() override {
		OpenXRVendorsEditorPlugin *editor_plugin = OpenXRVendorsEditorPlugin::get_singleton();
		if (editor_plugin == nullptr) {
			return;
		}

		editor_plugin->open_project_settings("application/boot_splash/image");
	}

	BootSplashRecommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart) :
			Recommendation(p_title, p_description, p_button_text, p_alert_type, p_project_type, p_vendor_type, p_requires_restart) {}
};

class MetaToolkitInstalledRecommendation : public Recommendation {
public:
	bool is_recommendation_applied() override {
		return FileAccess::file_exists("res://addons/godot_meta_toolkit/toolkit.gdextension");
	}

	void on_button_pressed() override {
		OpenXRVendorsEditorPlugin *editor_plugin = OpenXRVendorsEditorPlugin::get_singleton();
		if (editor_plugin == nullptr) {
			UtilityFunctions::print("Editor plugin is null!");
			return;
		}
		UtilityFunctions::print("Editor plugin is not null.");

		button->set_disabled(true);
		editor_plugin->open_asset_library("Godot Meta Toolkit");
	}

	MetaToolkitInstalledRecommendation(String p_title, String p_description, String p_button_text, AlertType p_alert_type, ProjectType p_project_type, VendorType p_vendor_type, bool p_requires_restart) :
			Recommendation(p_title, p_description, p_button_text, p_alert_type, p_project_type, p_vendor_type, p_requires_restart) {}
};
} //namespace godot_openxr_vendors

void XrProjectSetupDialog::_bind_methods() {
}

void XrProjectSetupDialog::_notification(uint32_t p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			recommendations.clear();

			// Vendor neutral general errors.
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("OpenXR", "OpenXR must be enabled", "Apply", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, true, "xr/openxr/enabled", true)));
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("XR Shaders", "XR shaders must be compiled", "Apply", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, true, "xr/shaders/enabled", true)));
#ifndef ANDROID_ENABLED
			recommendations.push_back(memnew(SdkPathRecommendation("Java SDK", "Please set a valid Java SDK path in Editor Settings", "Info", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false, "export/android/java_sdk_path")));
			recommendations.push_back(memnew(SdkPathRecommendation("Android SDK", "Please set a valid Android SDK path in Editor Settings", "Info", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false, "export/android/android_sdk_path")));
#endif // !ANDROID_ENABLED

			// Require Godot Meta Toolkit for Meta exports.
			recommendations.push_back(memnew(MetaToolkitInstalledRecommendation("Meta Toolkit Installed", "Please install the Godot Meta Toolkit", "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_META, false)));

			// Meta export preset error.
			HashMap<String, Variant> meta_export_preset_values;
			meta_export_preset_values["platform"] = "Android";
			meta_export_preset_values["meta_toolkit/enable_meta_toolkit"] = true;

			String meta_export_preset_description =
					"Please create a valid export preset for Meta:"
					"\n   - Android"
					"\n       - Meta Toolkit enabled";
			recommendations.push_back(memnew(ExportSettingRecommendation("Meta Export", meta_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_META, false, { meta_export_preset_values })));

#ifdef ANDROID_ENABLED
			bool gradle_supported = godot::internal::godot_version.minor >= 6;
#else
			bool gradle_supported = true;
#endif // ANDROID_ENABLED

			// HTC export preset error.
			HashMap<String, Variant> htc_export_preset_values;
			htc_export_preset_values["platform"] = "Android";
			htc_export_preset_values["xr_features/xr_mode"] = 1;
			htc_export_preset_values["xr_features/enable_khronos_plugin"] = true;
			if (gradle_supported) {
				htc_export_preset_values["gradle_build/use_gradle_build"] = true;
			}

			String htc_export_preset_description =
					"Please create a valid export preset for HTC:"
					"\n   - Android"
					"\n       - XR mode set to OpenXR"
					"\n       - Khronos plugin enabled";
			if (gradle_supported) {
				htc_export_preset_description += "\n       - Gradle build enabled";
			}

			recommendations.push_back(memnew(ExportSettingRecommendation("HTC Export", htc_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_HTC, false, { htc_export_preset_values })));

			// Pico export preset error.
			HashMap<String, Variant> pico_export_preset_values;
			pico_export_preset_values["platform"] = "Android";
			pico_export_preset_values["xr_features/xr_mode"] = 1;
			pico_export_preset_values["xr_features/enable_pico_plugin"] = true;
			if (gradle_supported) {
				pico_export_preset_values["gradle_build/use_gradle_build"] = true;
			}

			String pico_export_preset_description =
					"Please create a valid export preset for Pico:"
					"\n   - Android"
					"\n       - XR mode set to OpenXR"
					"\n       - Pico plugin enabled";
			if (gradle_supported) {
				pico_export_preset_description += "\n       - Gradle build enabled";
			}

			recommendations.push_back(memnew(ExportSettingRecommendation("Pico Export", pico_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_PICO, false, { pico_export_preset_values })));

			// Lynx export preset error.
			HashMap<String, Variant> lynx_export_preset_values;
			lynx_export_preset_values["platform"] = "Android";
			lynx_export_preset_values["xr_features/xr_mode"] = 1;
			lynx_export_preset_values["xr_features/enable_lynx_plugin"] = true;
			if (gradle_supported) {
				lynx_export_preset_values["gradle_build/use_gradle_build"] = true;
			}

			String lynx_export_preset_description =
					"Please create a valid export preset for Lynx:"
					"\n   - Android"
					"\n       - XR mode set to OpenXR"
					"\n       - Lynx plugin enabled";
			if (gradle_supported) {
				lynx_export_preset_description += "\n       - Gradle build enabled";
			}

			recommendations.push_back(memnew(ExportSettingRecommendation("Lynx Export", lynx_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_LYNX, false, { lynx_export_preset_values })));

			// Magic Leap export preset error.
			HashMap<String, Variant> magic_leap_export_preset_values;
			magic_leap_export_preset_values["platform"] = "Android";
			magic_leap_export_preset_values["xr_features/xr_mode"] = 1;
			magic_leap_export_preset_values["xr_features/enable_magicleap_plugin"] = true;
			if (gradle_supported) {
				magic_leap_export_preset_values["gradle_build/use_gradle_build"] = true;
			}

			String magic_leap_export_preset_description =
					"Please create a valid export preset for Magic Leap:"
					"\n   - Android"
					"\n       - XR mode set to OpenXR"
					"\n       - Magic Leap plugin enabled";
			if (gradle_supported) {
				magic_leap_export_preset_description += "\n       - Gradle build enabled";
			}

			recommendations.push_back(memnew(ExportSettingRecommendation("Magic Leap Export", magic_leap_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_MAGIC_LEAP, false, { magic_leap_export_preset_values })));

			// Android XR export preset error.
			HashMap<String, Variant> androidxr_export_preset_values;
			androidxr_export_preset_values["platform"] = "Android";
			androidxr_export_preset_values["xr_features/xr_mode"] = 1;
			androidxr_export_preset_values["xr_features/enable_androidxr_plugin"] = true;
			if (gradle_supported) {
				androidxr_export_preset_values["gradle_build/use_gradle_build"] = true;
			}

			String andoirdxr_export_preset_description =
					"Please create a valid export preset for Andoird XR:"
					"\n   - Android"
					"\n       - XR mode set to OpenXR"
					"\n       - Android XR plugin enabled";
			if (gradle_supported) {
				andoirdxr_export_preset_description += "\n       - Gradle build enabled";
			}

			recommendations.push_back(memnew(ExportSettingRecommendation("Android XR Export", andoirdxr_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_ANDROID_XR, false, { androidxr_export_preset_values })));

			// Valve export preset error.
			HashMap<String, Variant> valve_android_export_preset_values;
			valve_android_export_preset_values["platform"] = "Android";
			valve_android_export_preset_values["xr_features/xr_mode"] = 1;
			valve_android_export_preset_values["xr_features/enable_khronos_plugin"] = true;
			if (gradle_supported) {
				valve_android_export_preset_values["gradle_build/use_gradle_build"] = true;
			}

			HashMap<String, Variant> valve_linux_export_preset_values;
			valve_linux_export_preset_values["platform"] = "Linux";
			valve_linux_export_preset_values["binary_format/architecture"] = "arm64";

			String valve_export_preset_description =
					"Please create one of the following valid export presets for Valve:"
					"\n   - Android"
					"\n       - XR mode set to OpenXR"
					"\n       - Khronos plugin enabled";
			if (gradle_supported) {
				valve_export_preset_description += "\n       - Gradle build enabled";
			}
			valve_export_preset_description +=
					"\n   - Linux"
					"\n       - ARM64 architecture";

			recommendations.push_back(memnew(ExportSettingRecommendation("Vavle Export", valve_export_preset_description, "Open", ALERT_TYPE_ERROR, PROJECT_TYPE_ANY, VENDOR_TYPE_VALVE, false, { valve_android_export_preset_values, valve_linux_export_preset_values })));

			// Vendor neutral general warnings.
			recommendations.push_back(memnew(BootSplashRecommendation("Boot Splash", "No valid boot splash image was found", "Open", ALERT_TYPE_WARNING, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false)));
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("MSAA", "Recommended to set MSAA to 2x", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false, "rendering/anti_aliasing/quality/msaa_3d", 1)));
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("VRS Mode", "Recommended to set VRS mode to VR", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false, "rendering/vrs/mode", 2)));
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("Foveation Level", "Recommended to set foveation level to high", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false, "xr/openxr/foveation_level", 3)));
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("Dynamic Foveation", "Recommended to enable dynamic foveation", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_ANY, VENDOR_TYPE_ANY, false, "xr/openxr/foveation_dynamic", true)));

			// Vendor neutral VR warnings.
			recommendations.push_back(memnew(StartupSceneRecommendation("Main Scene", "No main scene is set, add VR startup scene?", "Add", ALERT_TYPE_WARNING, PROJECT_TYPE_VR, VENDOR_TYPE_ANY, false, vr_startup_tscn, start_vr_gd)));

			// Vendor neutral MR warnings.
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("Environment Blend Mode", "Environment blend mode should be non-opaque for mixed reality", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_MR, VENDOR_TYPE_ANY, false, "xr/openxr/environment_blend_mode", 2)));
			recommendations.push_back(memnew(StartupSceneRecommendation("Main Scene", "No main scene is set, add MR startup scene?", "Add", ALERT_TYPE_WARNING, PROJECT_TYPE_MR, VENDOR_TYPE_ANY, false, mr_startup_tscn, start_mr_gd)));

			// Meta MR warnings.
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("Meta Passthrough", "Meta passthrough extension should be enabled for mixed reality", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_MR, VENDOR_TYPE_META, false, "xr/openxr/extensions/meta/passthrough", true)));

			// HTC MR warnings.
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("HTC Passthrough", "HTC passthrough extension should be enabled for mixed reality", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_MR, VENDOR_TYPE_HTC, false, "xr/openxr/extensions/htc/passthrough", true)));

			// Pico MR warnings.
			recommendations.push_back(memnew(SimpleProjectSettingRecommendation("Pico Passthrough", "Meta passthrough extension should be enabled for mixed reality", "Apply", ALERT_TYPE_WARNING, PROJECT_TYPE_MR, VENDOR_TYPE_PICO, false, "xr/openxr/extensions/meta/passthrough", true)));

			EditorInterface *editor_interface = EditorInterface::get_singleton();
			if (editor_interface) {
				Ref<Theme> editor_theme = editor_interface->get_editor_theme();
				if (editor_theme.is_valid()) {
					error_texture = editor_theme->get_icon("StatusError", "EditorIcons");
					warning_texture = editor_theme->get_icon("StatusWarning", "EditorIcons");
					error_color = editor_theme->get_color("error_color", "Editor");
					warning_color = editor_theme->get_color("warning_color", "Editor");
				}
			}

			VBoxContainer *outer_vbox = memnew(VBoxContainer);
			outer_vbox->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
			add_child(outer_vbox);

			VBoxContainer *top_vbox = memnew(VBoxContainer);
			outer_vbox->add_child(top_vbox);

			HBoxContainer *filter_options_hbox = memnew(HBoxContainer);
			filter_options_hbox->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_BEGIN);
			filter_options_hbox->add_theme_constant_override("separation", 10);
			top_vbox->add_child(filter_options_hbox);

			HBoxContainer *project_type_hbox = memnew(HBoxContainer);
			filter_options_hbox->add_child(project_type_hbox);

			Label *project_type_label = memnew(Label);
			project_type_label->set_text("Project Type");
			project_type_hbox->add_child(project_type_label);

			project_type_selector = memnew(OptionButton);
			project_type_selector->add_item("Virtual Reality", PROJECT_TYPE_VR);
			project_type_selector->add_item("Mixed Reality", PROJECT_TYPE_MR);
			project_type_selector->select(project_type_selector->get_item_index(PROJECT_TYPE_VR));
			project_type_hbox->add_child(project_type_selector);
			project_type_selector->connect("item_selected", callable_mp(this, &XrProjectSetupDialog::_on_filter_selected));

			HBoxContainer *vendor_type_hbox = memnew(HBoxContainer);
			filter_options_hbox->add_child(vendor_type_hbox);

			Label *vendor_type_label = memnew(Label);
			vendor_type_label->set_text("Vendor Type");
			vendor_type_hbox->add_child(vendor_type_label);

			vendor_type_selector = memnew(OptionButton);
			vendor_type_selector->add_item("Meta", VENDOR_TYPE_META);
			vendor_type_selector->add_item("HTC", VENDOR_TYPE_HTC);
			vendor_type_selector->add_item("Pico", VENDOR_TYPE_PICO);
			vendor_type_selector->add_item("Lynx", VENDOR_TYPE_LYNX);
			vendor_type_selector->add_item("Magic Leap", VENDOR_TYPE_MAGIC_LEAP);
			vendor_type_selector->add_item("Android XR", VENDOR_TYPE_ANDROID_XR);
			vendor_type_selector->add_item("Valve", VENDOR_TYPE_VALVE);
			vendor_type_selector->select(project_type_selector->get_item_index(VENDOR_TYPE_META));
			vendor_type_hbox->add_child(vendor_type_selector);
			vendor_type_selector->connect("item_selected", callable_mp(this, &XrProjectSetupDialog::_on_filter_selected));

			restart_editor_hbox = memnew(HBoxContainer);
			restart_editor_hbox->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_BEGIN);
			restart_editor_hbox->hide();
			top_vbox->add_child(restart_editor_hbox);

			CenterContainer *icon_container = memnew(CenterContainer);
			restart_editor_hbox->add_child(icon_container);

			TextureRect *icon = memnew(TextureRect);
			icon->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
			icon->set_texture(warning_texture);
			icon->set_custom_minimum_size(Vector2(15.0, 15.0));
			icon_container->add_child(icon);

			Label *restart_editor_label = memnew(Label);
			restart_editor_label->set_text("Please restart the editor to apply updated settings");
			restart_editor_label->add_theme_color_override("font_color", warning_color);
			restart_editor_hbox->add_child(restart_editor_label);

			ScrollContainer *scroll_container = memnew(ScrollContainer);
			scroll_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_container->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
			outer_vbox->add_child(scroll_container);

			scroll_vbox = memnew(VBoxContainer);
			scroll_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_vbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_container->add_child(scroll_vbox);

			for (Recommendation *recommendation : recommendations) {
				add_window_entry(recommendation);
			}

			rec_list_empty_label = memnew(Label);
			rec_list_empty_label->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_CENTER);
			rec_list_empty_label->set_v_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_CENTER);
			rec_list_empty_label->set_text("All recommended settings have been applied!");
			scroll_vbox->add_child(rec_list_empty_label);

			set_exclusive(false);
			set_min_size(Vector2i(700, 300));
		} break;
	}
}

void XrProjectSetupDialog::add_window_entry(Recommendation *p_recommendation) {
	VBoxContainer *rec_vbox = memnew(VBoxContainer);
	rec_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	scroll_vbox->add_child(rec_vbox);
	p_recommendation->vbox = rec_vbox;

	MarginContainer *margin_container = memnew(MarginContainer);
	margin_container->add_theme_constant_override("margin_left", 5);
	margin_container->add_theme_constant_override("margin_top", 5);
	margin_container->add_theme_constant_override("margin_right", 5);
	margin_container->add_theme_constant_override("margin_bottom", 5);
	rec_vbox->add_child(margin_container);

	HBoxContainer *hbox = memnew(HBoxContainer);
	margin_container->add_child(hbox);

	Label *title_label = memnew(Label);
	title_label->set_text(p_recommendation->title);
	title_label->set_custom_minimum_size(Vector2(180.0, 0.0));
	hbox->add_child(title_label);

	HBoxContainer *message_hbox = memnew(HBoxContainer);
	message_hbox->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_BEGIN);
	hbox->add_child(message_hbox);

	CenterContainer *icon_container = memnew(CenterContainer);
	message_hbox->add_child(icon_container);
	p_recommendation->icon_container = icon_container;

	TextureRect *icon = memnew(TextureRect);
	icon->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
	icon_container->add_child(icon);

	Label *description_label = memnew(Label);
	description_label->set_text(p_recommendation->description);
	message_hbox->add_child(description_label);
	p_recommendation->description_label = description_label;

	switch (p_recommendation->alert_type) {
		case ALERT_TYPE_ERROR: {
			icon->set_texture(error_texture);
			icon->set_custom_minimum_size(Vector2(15.0, 15.0));
			p_recommendation->font_color = error_color;
		} break;
		case ALERT_TYPE_WARNING: {
			icon->set_texture(warning_texture);
			icon->set_custom_minimum_size(Vector2(15.0, 15.0));
			p_recommendation->font_color = warning_color;
		} break;
		case ALERT_TYPE_NONE: {
		} break;
	}

	Button *button = memnew(Button);
	button->set_text(p_recommendation->button_text);
	button->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
	hbox->add_child(button);
	p_recommendation->button = button;
	button->connect("pressed", callable_mp(this, &XrProjectSetupDialog::_on_recommendation_button_pressed).bind(reinterpret_cast<uint64_t>(p_recommendation)));

	HSeparator *separator = memnew(HSeparator);
	rec_vbox->add_child(separator);
}

void XrProjectSetupDialog::filter_recommendations() {
	ProjectType project_type = ProjectType(project_type_selector->get_selected_id());
	VendorType vendor_type = VendorType(vendor_type_selector->get_selected_id());
	for (Recommendation *recommendation : recommendations) {
		if (recommendation->should_display(project_type, vendor_type)) {
			recommendation->show_window_entry();
		} else {
			recommendation->hide_window_entry();
		}
	}

	// Hide label to not include it in check for visible children.
	rec_list_empty_label->hide();
	bool show_rec_list_empty_label = true;

	for (int i = 0; i < scroll_vbox->get_child_count(); i++) {
		Control *child = Object::cast_to<Control>(scroll_vbox->get_child(i));
		if (child && child->is_visible()) {
			show_rec_list_empty_label = false;
			break;
		}
	}

	if (show_rec_list_empty_label) {
		rec_list_empty_label->show();
	}
}

void XrProjectSetupDialog::_on_recommendation_button_pressed(uint64_t p_recommendation) {
	Recommendation *recommendation = reinterpret_cast<Recommendation *>(p_recommendation);
	recommendation->on_button_pressed();

	if (recommendation->requires_restart) {
		restart_editor_hbox->show();
	}
}

void XrProjectSetupDialog::_on_filter_selected(int p_item_index) {
	filter_recommendations();
}

void XrProjectSetupDialog::open() {
	popup_centered_ratio(0.35);
}

XrProjectSetupDialog::XrProjectSetupDialog() {
	set_title("XR Project Setup");
	connect("focus_entered", callable_mp(this, &XrProjectSetupDialog::filter_recommendations));
}

XrProjectSetupDialog::~XrProjectSetupDialog() {
}
