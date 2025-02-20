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

static void define_global_bool(ProjectSettings *project_settings, const String &p_name, bool p_default_value) {
	if (!project_settings->has_setting(p_name)) {
		project_settings->set_setting(p_name, p_default_value);
	}

	project_settings->set_initial_value(p_name, true);
	project_settings->set_as_basic(p_name, false);
	Dictionary property_info;
	property_info["name"] = p_name;
	property_info["type"] = Variant::Type::BOOL;
	property_info["hint"] = PROPERTY_HINT_NONE;
	project_settings->add_property_info(property_info);
}

static bool global_bool_get(const String &p_name) {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	if (project_settings == nullptr) {
		return false;
	}

	return project_settings->get_setting_with_override(p_name);
}

static void add_plugin_core_settings() {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return;
    }

    define_global_bool(project_settings, "xr/openxr/openxr/hybrid_app", false);

    // Meta/FB Base Features
    define_global_bool(project_settings, "xr/openxr/extensions/meta/body_tracking", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/face_tracking", true);

    // Meta/FB Hand Tracking Suite
    define_global_bool(project_settings, "xr/openxr/extensions/meta/hand_tracking/aim", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/hand_tracking/capsules", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/hand_tracking/mesh", true);

    // Meta/FB Composition & Rendering
    define_global_bool(project_settings, "xr/openxr/extensions/meta/composition/alpha_blend", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/composition/secure_content", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/composition/layer_settings", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/recommended_layer_resolution", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/android_surface_swapchain", true);

    // Meta/FB Scene & Spatial Features
    define_global_bool(project_settings, "xr/openxr/extensions/meta/spatial/core", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/spatial/sharing", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/spatial/storage_batch", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/spatial/user", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/spatial/mesh", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/scene/capture", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/scene/core", true);

    // Meta/FB Passthrough & Models
    define_global_bool(project_settings, "xr/openxr/extensions/meta/passthrough", true);
    define_global_bool(project_settings, "xr/openxr/extensions/meta/render_model", true);

    // HTC Extensions
    define_global_bool(project_settings, "xr/openxr/extensions/htc/facial_tracking", true);
    define_global_bool(project_settings, "xr/openxr/extensions/htc/passthrough", true);
}

void add_plugin_project_settings() {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	if (project_settings == nullptr) {
		return;
	}

	{
		// Add the 'automatically_request_runtime_permissions' project setting
		String request_permissions_setting = "xr/openxr/extensions/automatically_request_runtime_permissions";
		if (!project_settings->has_setting(request_permissions_setting)) {
			// Default value is `true` to match prior plugin behavior
			project_settings->set_setting(request_permissions_setting, true);
		}

		project_settings->set_initial_value(request_permissions_setting, true);
		project_settings->set_as_basic(request_permissions_setting, false);
		Dictionary property_info;
		property_info["name"] = request_permissions_setting;
		property_info["type"] = Variant::Type::BOOL;
		property_info["hint"] = PROPERTY_HINT_NONE;
		project_settings->add_property_info(property_info);
	}

	{
		String collision_shape_2d_thickness = "xr/openxr/extensions/meta_scene_api/collision_shape_2d_thickness";
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

	OpenXRFbHandTrackingAimExtensionWrapper::add_project_setting(project_settings);
	OpenXRMetaRecommendedLayerResolutionExtensionWrapper::add_project_setting(project_settings);
}

void initialize_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE: {
			add_plugin_core_settings();

		} break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			add_plugin_project_settings();

			// Register classes
			// Always register these classes, even if the extensions are disabled
			ClassDB::register_class<OpenXRFbSpatialEntitySharingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityStorageBatchExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityUserExtensionWrapper>();
			ClassDB::register_class<OpenXRMetaRecommendedLayerResolutionExtensionWrapper>();
			ClassDB::register_class<OpenXRMetaSpatialEntityMeshExtensionWrapper>();
			ClassDB::register_class<OpenXRFbFaceTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingMeshExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerSecureContentExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerAlphaBlendExtensionWrapper>();
			ClassDB::register_class<OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper>();

			ClassDB::register_class<OpenXRFbHandTrackingMesh>();
			ClassDB::register_class<OpenXRFbPassthroughGeometry>();
			ClassDB::register_class<OpenXRFbRenderModel>();
			ClassDB::register_class<OpenXRFbSceneManager>();
			ClassDB::register_class<OpenXRFbSpatialAnchorManager>();
			ClassDB::register_class<OpenXRFbSpatialEntity>();
			ClassDB::register_class<OpenXRFbSpatialEntityBatch>();
			ClassDB::register_class<OpenXRFbSpatialEntityQuery>();
			ClassDB::register_class<OpenXRFbSpatialEntityUser>();
			ClassDB::register_class<OpenXRMetaPassthroughColorLut>();
			ClassDB::register_class<OpenXRHybridApp>();

			Engine* eng = Engine::get_singleton();

			if (global_bool_get("xr/openxr/extensions/fb_passthrough")) {
				eng->register_singleton("OpenXRFbPassthroughExtensionWrapper", OpenXRFbPassthroughExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_render_model")) {
				eng->register_singleton("OpenXRFbRenderModelExtensionWrapper", OpenXRFbRenderModelExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_scene_capture")) {
				eng->register_singleton("OpenXRFbSceneCaptureExtensionWrapper", OpenXRFbSceneCaptureExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_spatial")) {
				eng->register_singleton("OpenXRFbSpatialEntityExtensionWrapper", OpenXRFbSpatialEntityExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSpatialEntityStorageExtensionWrapper", OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSpatialEntityQueryExtensionWrapper", OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSpatialEntityContainerExtensionWrapper", OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_scene")) {
				eng->register_singleton("OpenXRFbSceneExtensionWrapper", OpenXRFbSceneExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_hand_tracking_aim")) {
				eng->register_singleton("OpenXRFbHandTrackingAimExtensionWrapper", OpenXRFbHandTrackingAimExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_hand_tracking_capsules")) {
				eng->register_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper", OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/fb_composition_layer_settings")) {
				eng->register_singleton("OpenXRFbCompositionLayerSettingsExtensionWrapper", OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/htc_facial_tracking")) {
				eng->register_singleton("OpenXRHtcFacialTrackingExtensionWrapper", OpenXRHtcFacialTrackingExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/htc_passthrough")) {
				eng->register_singleton("OpenXRHtcPassthroughExtensionWrapper", OpenXRHtcPassthroughExtensionWrapper::get_singleton());
			}

			if (global_bool_get("xr/openxr/hybrid_app")) {
				eng->register_singleton("OpenXRHybridApp", OpenXRHybridApp::get_singleton());
			}

			if (global_bool_get("xr/openxr/extensions/meta/spatial/sharing")) {
				eng->register_singleton("OpenXRFbSpatialEntitySharingExtensionWrapper", 
					OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/spatial/storage_batch")) {
				eng->register_singleton("OpenXRFbSpatialEntityStorageBatchExtensionWrapper", 
					OpenXRFbSpatialEntityStorageBatchExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/spatial/user")) {
				eng->register_singleton("OpenXRFbSpatialEntityUserExtensionWrapper", 
					OpenXRFbSpatialEntityUserExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/recommended_layer_resolution")) {
				eng->register_singleton("OpenXRMetaRecommendedLayerResolutionExtensionWrapper", 
					OpenXRMetaRecommendedLayerResolutionExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/spatial/mesh")) {
				eng->register_singleton("OpenXRMetaSpatialEntityMeshExtensionWrapper", 
					OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/face_tracking")) {
				eng->register_singleton("OpenXRFbFaceTrackingExtensionWrapper", 
					OpenXRFbFaceTrackingExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/hand_tracking/mesh")) {
				eng->register_singleton("OpenXRFbHandTrackingMeshExtensionWrapper", 
					OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/composition/secure_content")) {
				eng->register_singleton("OpenXRFbCompositionLayerSecureContentExtensionWrapper", 
					OpenXRFbCompositionLayerSecureContentExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/composition/alpha_blend")) {
				eng->register_singleton("OpenXRFbCompositionLayerAlphaBlendExtensionWrapper", 
					OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::get_singleton());
			}
			
			if (global_bool_get("xr/openxr/extensions/meta/android_surface_swapchain")) {
				eng->register_singleton("OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper", 
					OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::get_singleton());
			}
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

static void cleanup_singletons() {
	Engine* eng = Engine::get_singleton();
	if (eng == nullptr) {
		return;
	}

	// Core/Hybrid App
	if (eng->has_singleton("OpenXRHybridApp")) {
		eng->unregister_singleton("OpenXRHybridApp");
		memdelete(OpenXRHybridApp::get_singleton());
	}

	// Meta/FB Passthrough & Models
	if (eng->has_singleton("OpenXRFbPassthroughExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbPassthroughExtensionWrapper");
		memdelete(OpenXRFbPassthroughExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbRenderModelExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbRenderModelExtensionWrapper");
		memdelete(OpenXRFbRenderModelExtensionWrapper::get_singleton());
	}

	// Meta/FB Scene Capture
	if (eng->has_singleton("OpenXRFbSceneCaptureExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSceneCaptureExtensionWrapper");
		memdelete(OpenXRFbSceneCaptureExtensionWrapper::get_singleton());
	}

	// Meta/FB Spatial
	if (eng->has_singleton("OpenXRFbSpatialEntityExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntityExtensionWrapper");
		memdelete(OpenXRFbSpatialEntityExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbSpatialEntityStorageExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntityStorageExtensionWrapper");
		memdelete(OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbSpatialEntityQueryExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntityQueryExtensionWrapper");
		memdelete(OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbSpatialEntityContainerExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntityContainerExtensionWrapper");
		memdelete(OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbSpatialEntitySharingExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntitySharingExtensionWrapper");
		memdelete(OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbSpatialEntityStorageBatchExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntityStorageBatchExtensionWrapper");
		memdelete(OpenXRFbSpatialEntityStorageBatchExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbSpatialEntityUserExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSpatialEntityUserExtensionWrapper");
		memdelete(OpenXRFbSpatialEntityUserExtensionWrapper::get_singleton());
	}

	// Meta/FB Scene
	if (eng->has_singleton("OpenXRFbSceneExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbSceneExtensionWrapper");
		memdelete(OpenXRFbSceneExtensionWrapper::get_singleton());
	}

	// Meta/FB Hand Tracking
	if (eng->has_singleton("OpenXRFbHandTrackingAimExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbHandTrackingAimExtensionWrapper");
		memdelete(OpenXRFbHandTrackingAimExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper");
		memdelete(OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbHandTrackingMeshExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbHandTrackingMeshExtensionWrapper");
		memdelete(OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton());
	}

	// Meta/FB Face Tracking
	if (eng->has_singleton("OpenXRFbFaceTrackingExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbFaceTrackingExtensionWrapper");
		memdelete(OpenXRFbFaceTrackingExtensionWrapper::get_singleton());
	}

	// Meta/FB Composition Layer
	if (eng->has_singleton("OpenXRFbCompositionLayerSettingsExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbCompositionLayerSettingsExtensionWrapper");
		memdelete(OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbCompositionLayerSecureContentExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbCompositionLayerSecureContentExtensionWrapper");
		memdelete(OpenXRFbCompositionLayerSecureContentExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRFbCompositionLayerAlphaBlendExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbCompositionLayerAlphaBlendExtensionWrapper");
		memdelete(OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::get_singleton());
	}

	// Meta Recommended Layer Resolution
	if (eng->has_singleton("OpenXRMetaRecommendedLayerResolutionExtensionWrapper")) {
		eng->unregister_singleton("OpenXRMetaRecommendedLayerResolutionExtensionWrapper");
		memdelete(OpenXRMetaRecommendedLayerResolutionExtensionWrapper::get_singleton());
	}

	// Meta Spatial Entity Mesh
	if (eng->has_singleton("OpenXRMetaSpatialEntityMeshExtensionWrapper")) {
		eng->unregister_singleton("OpenXRMetaSpatialEntityMeshExtensionWrapper");
		memdelete(OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton());
	}

	// HTC Extensions
	if (eng->has_singleton("OpenXRHtcFacialTrackingExtensionWrapper")) {
		eng->unregister_singleton("OpenXRHtcFacialTrackingExtensionWrapper");
		memdelete(OpenXRHtcFacialTrackingExtensionWrapper::get_singleton());
	}

	if (eng->has_singleton("OpenXRHtcPassthroughExtensionWrapper")) {
		eng->unregister_singleton("OpenXRHtcPassthroughExtensionWrapper");
		memdelete(OpenXRHtcPassthroughExtensionWrapper::get_singleton());
	}

	// Android Surface
	if (eng->has_singleton("OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper")) {
		eng->unregister_singleton("OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper");
		memdelete(OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::get_singleton());
	}
}

void terminate_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE:
			break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE:
			cleanup_singletons();
		break;

		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			break;

		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
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
