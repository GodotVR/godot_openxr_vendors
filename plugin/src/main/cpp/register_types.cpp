/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "export/export_plugin.h"
#include "export/khronos_export_plugin.h"
#include "export/lynx_export_plugin.h"
#include "export/magicleap_export_plugin.h"
#include "export/meta_export_plugin.h"
#include "export/pico_export_plugin.h"

#include "extensions/openxr_fb_android_surface_swapchain_create_extension_wrapper.h"
#include "extensions/openxr_fb_body_tracking_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_alpha_blend_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_depth_test_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_image_layout_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_secure_content_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_settings_extension_wrapper.h"
#include "extensions/openxr_fb_face_tracking_extension_wrapper.h"
#include "extensions/openxr_fb_hand_tracking_aim_extension_wrapper.h"
#include "extensions/openxr_fb_hand_tracking_capsules_extension_wrapper.h"
#include "extensions/openxr_fb_hand_tracking_mesh_extension_wrapper.h"
#include "extensions/openxr_fb_passthrough_extension_wrapper.h"
#include "extensions/openxr_fb_render_model_extension_wrapper.h"
#include "extensions/openxr_fb_scene_capture_extension_wrapper.h"
#include "extensions/openxr_fb_scene_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_sharing_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_storage_batch_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_user_extension_wrapper.h"
#include "extensions/openxr_htc_facial_tracking_extension_wrapper.h"
#include "extensions/openxr_htc_passthrough_extension_wrapper.h"
#include "extensions/openxr_meta_recommended_layer_resolution_extension_wrapper.h"
#include "extensions/openxr_meta_spatial_entity_mesh_extension_wrapper.h"

#include "classes/openxr_fb_hand_tracking_mesh.h"
#include "classes/openxr_fb_passthrough_geometry.h"
#include "classes/openxr_fb_render_model.h"
#include "classes/openxr_fb_scene_manager.h"
#include "classes/openxr_fb_spatial_anchor_manager.h"
#include "classes/openxr_fb_spatial_entity.h"
#include "classes/openxr_fb_spatial_entity_batch.h"
#include "classes/openxr_fb_spatial_entity_query.h"
#include "classes/openxr_fb_spatial_entity_user.h"
#include "classes/openxr_hybrid_app.h"
#include "classes/openxr_meta_passthrough_color_lut.h"

using namespace godot;

struct ExtensionSingleton {
	String class_name;
	OpenXRExtensionWrapperExtension *extension;
};

static Vector<OpenXRExtensionWrapperExtension *> extensions_registered;
static Vector<ExtensionSingleton> extensions_singletons;

static inline void _register_extension_with_openxr(OpenXRExtensionWrapperExtension *p_extension) {
	p_extension->register_extension_wrapper();
	extensions_registered.push_back(p_extension);
}

static inline void _register_extension_as_singleton(OpenXRExtensionWrapperExtension *p_extension) {
	String class_name = p_extension->get_class();
	Engine::get_singleton()->register_singleton(class_name, p_extension);
	extensions_singletons.push_back({ class_name, p_extension });
}

static void _add_bool_project_setting(ProjectSettings *p_project_settings, const String &p_name, bool p_default_value) {
	if (!p_project_settings->has_setting(p_name)) {
		p_project_settings->set_setting(p_name, p_default_value);
	}

	p_project_settings->set_initial_value(p_name, p_default_value);
	p_project_settings->set_as_basic(p_name, false);
	Dictionary property_info;
	property_info["name"] = p_name;
	property_info["type"] = Variant::Type::BOOL;
	property_info["hint"] = PROPERTY_HINT_NONE;
	p_project_settings->add_property_info(property_info);
}

static inline bool _get_bool_project_setting(const String &p_name) {
	return (bool)ProjectSettings::get_singleton()->get_setting_with_override(p_name);
}

void initialize_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE: {
			add_plugin_project_settings();

			ClassDB::register_class<OpenXRFbPassthroughExtensionWrapper>();
			ClassDB::register_class<OpenXRFbRenderModelExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSceneCaptureExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntitySharingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityStorageExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityStorageBatchExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityQueryExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityContainerExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityUserExtensionWrapper>();
			ClassDB::register_class<OpenXRMetaRecommendedLayerResolutionExtensionWrapper>();
			ClassDB::register_class<OpenXRMetaSpatialEntityMeshExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSceneExtensionWrapper>();
			ClassDB::register_class<OpenXRFbFaceTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbBodyTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingMeshExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingAimExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingCapsulesExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerSecureContentExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerDepthTestExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerAlphaBlendExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerImageLayoutExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerSettingsExtensionWrapper>();
			ClassDB::register_class<OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper>();
			ClassDB::register_class<OpenXRHtcFacialTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRHtcPassthroughExtensionWrapper>();

			if (_get_bool_project_setting("xr/openxr/extensions/meta/passthrough")) {
				_register_extension_with_openxr(OpenXRFbPassthroughExtensionWrapper::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/render_model")) {
				_register_extension_with_openxr(OpenXRFbRenderModelExtensionWrapper::get_singleton());
			}

			// Three settings to match the three permissions for the Android manifest.
			bool meta_anchor_api = _get_bool_project_setting("xr/openxr/extensions/meta/anchor_api");
			bool meta_anchor_sharing = _get_bool_project_setting("xr/openxr/extensions/meta/anchor_sharing");
			bool meta_scene_api = _get_bool_project_setting("xr/openxr/extensions/meta/scene_api");
			if (meta_anchor_api || meta_anchor_sharing || meta_scene_api) {
				// There are needed by all of them.
				_register_extension_with_openxr(OpenXRFbSpatialEntityExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityStorageBatchExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton());

				if (meta_scene_api) {
					_register_extension_with_openxr(OpenXRFbSceneExtensionWrapper::get_singleton());
					_register_extension_with_openxr(OpenXRFbSceneCaptureExtensionWrapper::get_singleton());
					_register_extension_with_openxr(OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton());
				}

				if (meta_anchor_sharing) {
					_register_extension_with_openxr(OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton());
					_register_extension_with_openxr(OpenXRFbSpatialEntityUserExtensionWrapper::get_singleton());
				}
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/dynamic_resolution")) {
				_register_extension_with_openxr(OpenXRMetaRecommendedLayerResolutionExtensionWrapper::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/face_tracking")) {
				_register_extension_with_openxr(OpenXRFbFaceTrackingExtensionWrapper::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/body_tracking")) {
				_register_extension_with_openxr(OpenXRFbBodyTrackingExtensionWrapper::get_singleton());
			}

			// All of the hand tracking extensions depend on the Godot hand tracking setting being set first.
			if (_get_bool_project_setting("xr/openxr/extensions/hand_tracking")) {
				if (_get_bool_project_setting("xr/openxr/extensions/meta/hand_tracking_mesh")) {
					_register_extension_with_openxr(OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton());
				}

				if (_get_bool_project_setting("xr/openxr/extensions/meta/hand_tracking_aim")) {
					_register_extension_with_openxr(OpenXRFbHandTrackingAimExtensionWrapper::get_singleton());
				}

				if (_get_bool_project_setting("xr/openxr/extensions/meta/hand_tracking_capsules")) {
					_register_extension_with_openxr(OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton());
				}
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/composition_layer_settings")) {
				_register_extension_with_openxr(OpenXRFbCompositionLayerSecureContentExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerDepthTestExtensionWrapper::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerImageLayoutExtensionWrapper::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/htc/face_tracking")) {
				_register_extension_with_openxr(OpenXRHtcFacialTrackingExtensionWrapper::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/htc/passthrough")) {
				_register_extension_with_openxr(OpenXRHtcPassthroughExtensionWrapper::get_singleton());
			}

		} break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			_register_extension_as_singleton(OpenXRFbPassthroughExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbRenderModelExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbSceneCaptureExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbSceneExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbHandTrackingAimExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbCompositionLayerDepthTestExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRHtcFacialTrackingExtensionWrapper::get_singleton());
			_register_extension_as_singleton(OpenXRHtcPassthroughExtensionWrapper::get_singleton());

			ClassDB::register_class<OpenXRFbRenderModel>();
			ClassDB::register_class<OpenXRFbHandTrackingMesh>();
			ClassDB::register_class<OpenXRFbSceneManager>();
			ClassDB::register_class<OpenXRFbSpatialAnchorManager>();
			ClassDB::register_class<OpenXRFbSpatialEntity>();
			ClassDB::register_class<OpenXRFbSpatialEntityBatch>();
			ClassDB::register_class<OpenXRFbSpatialEntityQuery>();
			ClassDB::register_class<OpenXRFbSpatialEntityUser>();
			ClassDB::register_class<OpenXRFbPassthroughGeometry>();
			ClassDB::register_class<OpenXRMetaPassthroughColorLut>();

			ClassDB::register_class<OpenXRHybridApp>();
			Engine::get_singleton()->register_singleton("OpenXRHybridApp", OpenXRHybridApp::get_singleton());
		} break;

		case MODULE_INITIALIZATION_LEVEL_EDITOR: {
			ClassDB::register_class<OpenXREditorExportPlugin>();

			ClassDB::register_class<KhronosEditorExportPlugin>();
			ClassDB::register_class<KhronosEditorPlugin>();
			EditorPlugins::add_by_type<KhronosEditorPlugin>();

			ClassDB::register_class<LynxEditorPlugin>();
			EditorPlugins::add_by_type<LynxEditorPlugin>();

			ClassDB::register_class<MetaEditorExportPlugin>();
			ClassDB::register_class<MetaEditorPlugin>();
			EditorPlugins::add_by_type<MetaEditorPlugin>();

			ClassDB::register_class<PicoEditorExportPlugin>();
			ClassDB::register_class<PicoEditorPlugin>();
			EditorPlugins::add_by_type<PicoEditorPlugin>();

			ClassDB::register_class<MagicleapEditorExportPlugin>();
			ClassDB::register_class<MagicleapEditorPlugin>();
			EditorPlugins::add_by_type<MagicleapEditorPlugin>();
		} break;

		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}

void terminate_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE:
			break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			for (const ExtensionSingleton &extension_singleton : extensions_singletons) {
				Engine::get_singleton()->unregister_singleton(extension_singleton.class_name);

				// Any extensions that were registered with OpenXRAPI will get cleaned up by OpenXRAPI,
				// however, we'll need to make sure that those that weren't get cleaned up properly.
				if (!extensions_registered.has(extension_singleton.extension)) {
					memdelete(extension_singleton.extension);
				}
			}
			extensions_singletons.clear();
			extensions_registered.clear();

			Engine::get_singleton()->unregister_singleton("OpenXRHybridApp");
			memdelete(OpenXRHybridApp::get_singleton());
		} break;

		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			break;

		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}

void add_plugin_project_settings() {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	if (project_settings == nullptr) {
		return;
	}

	// Multi-vendor stuff first.

	{
		String hybrid_app_enabled_setting = "xr/hybrid_app/enabled";
		if (!project_settings->has_setting(hybrid_app_enabled_setting)) {
			project_settings->set_setting(hybrid_app_enabled_setting, false);
		}

		project_settings->set_initial_value(hybrid_app_enabled_setting, false);
		project_settings->set_as_basic(hybrid_app_enabled_setting, true);
		Dictionary hybrid_app_enabled_property_info;
		hybrid_app_enabled_property_info["name"] = hybrid_app_enabled_setting;
		hybrid_app_enabled_property_info["type"] = Variant::Type::BOOL;
		hybrid_app_enabled_property_info["hint"] = PROPERTY_HINT_NONE;
		project_settings->add_property_info(hybrid_app_enabled_property_info);

		String hybrid_app_launch_mode_setting = "xr/hybrid_app/launch_mode";
		if (!project_settings->has_setting(hybrid_app_launch_mode_setting)) {
			project_settings->set_setting(hybrid_app_launch_mode_setting, OpenXRHybridApp::HYBRID_MODE_IMMERSIVE);
		}

		project_settings->set_initial_value(hybrid_app_launch_mode_setting, OpenXRHybridApp::HYBRID_MODE_IMMERSIVE);
		project_settings->set_as_basic(hybrid_app_launch_mode_setting, true);
		Dictionary hybrid_app_launch_mode_property_info;
		hybrid_app_launch_mode_property_info["name"] = hybrid_app_launch_mode_setting;
		hybrid_app_launch_mode_property_info["type"] = Variant::Type::INT;
		hybrid_app_launch_mode_property_info["hint"] = PROPERTY_HINT_ENUM;
		hybrid_app_launch_mode_property_info["hint_string"] = "Start As Immersive:0,Start As Panel:1";
		project_settings->add_property_info(hybrid_app_launch_mode_property_info);
	}

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/htc/passthrough", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/htc/face_tracking", false);

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/passthrough", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/face_tracking", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/body_tracking", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/hand_tracking_aim", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/hand_tracking_mesh", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/hand_tracking_capsules", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/render_model", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/anchor_api", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/anchor_sharing", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/scene_api", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/composition_layer_settings", true);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/dynamic_resolution", true);

	{
		String collision_shape_2d_thickness = "xr/openxr/extensions/meta/scene_api/collision_shape_2d_thickness";
		if (!project_settings->has_setting(collision_shape_2d_thickness)) {
			project_settings->set_setting(collision_shape_2d_thickness, 0.1);
		}

		project_settings->set_initial_value(collision_shape_2d_thickness, 0.1);
		project_settings->set_as_basic(collision_shape_2d_thickness, false);
		Dictionary property_info;
		property_info["name"] = collision_shape_2d_thickness;
		property_info["type"] = Variant::Type::FLOAT;
		property_info["hint"] = PROPERTY_HINT_NONE;
		project_settings->add_property_info(property_info);
	}

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/automatically_request_runtime_permissions", true);
}

extern "C" {
GDExtensionBool GDE_EXPORT plugin_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_plugin_module);
	init_obj.register_terminator(terminate_plugin_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
