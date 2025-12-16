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

#include "editor/xr_project_setup_dialog.h"
#include "editor_plugin.h"
#include "export/android_xr_export_plugin.h"
#include "export/export_plugin.h"
#include "export/khronos_export_plugin.h"
#include "export/lynx_export_plugin.h"
#include "export/magicleap_export_plugin.h"
#include "export/meta_export_plugin.h"
#include "export/pico_export_plugin.h"

#include "extensions/openxr_android_environment_depth_extension.h"
#include "extensions/openxr_android_eye_tracking_extension.h"
#include "extensions/openxr_android_face_tracking_extension.h"
#include "extensions/openxr_android_light_estimation_extension.h"
#include "extensions/openxr_android_passthrough_camera_state_extension.h"
#include "extensions/openxr_android_performance_metrics_extension.h"
#include "extensions/openxr_android_scene_meshing_extension.h"
#include "extensions/openxr_fb_android_surface_swapchain_create_extension.h"
#include "extensions/openxr_fb_body_tracking_extension.h"
#include "extensions/openxr_fb_color_space_extension.h"
#include "extensions/openxr_fb_composition_layer_alpha_blend_extension.h"
#include "extensions/openxr_fb_composition_layer_depth_test_extension.h"
#include "extensions/openxr_fb_composition_layer_image_layout_extension.h"
#include "extensions/openxr_fb_composition_layer_secure_content_extension.h"
#include "extensions/openxr_fb_composition_layer_settings_extension.h"
#include "extensions/openxr_fb_face_tracking_extension.h"
#include "extensions/openxr_fb_hand_tracking_aim_extension.h"
#include "extensions/openxr_fb_hand_tracking_capsules_extension.h"
#include "extensions/openxr_fb_hand_tracking_mesh_extension.h"
#include "extensions/openxr_fb_passthrough_extension.h"
#include "extensions/openxr_fb_render_model_extension.h"
#include "extensions/openxr_fb_scene_capture_extension.h"
#include "extensions/openxr_fb_scene_extension.h"
#include "extensions/openxr_fb_space_warp_extension.h"
#include "extensions/openxr_fb_spatial_entity_container_extension.h"
#include "extensions/openxr_fb_spatial_entity_extension.h"
#include "extensions/openxr_fb_spatial_entity_query_extension.h"
#include "extensions/openxr_fb_spatial_entity_sharing_extension.h"
#include "extensions/openxr_fb_spatial_entity_storage_batch_extension.h"
#include "extensions/openxr_fb_spatial_entity_storage_extension.h"
#include "extensions/openxr_fb_spatial_entity_user_extension.h"
#include "extensions/openxr_htc_facial_tracking_extension.h"
#include "extensions/openxr_htc_passthrough_extension.h"
#include "extensions/openxr_meta_boundary_visibility_extension.h"
#include "extensions/openxr_meta_colocation_discovery_extension.h"
#include "extensions/openxr_meta_environment_depth_extension.h"
#include "extensions/openxr_meta_headset_id_extension.h"
#include "extensions/openxr_meta_performance_metrics_extension.h"
#include "extensions/openxr_meta_recommended_layer_resolution_extension.h"
#include "extensions/openxr_meta_simultaneous_hands_and_controllers_extension.h"
#include "extensions/openxr_meta_spatial_entity_mesh_extension.h"
#include "extensions/openxr_ml_marker_understanding_extension.h"
#include "extensions/openxr_stationary_reference_space_extension.h"

#include "classes/openxr_android_environment_depth.h"
#include "classes/openxr_android_light_estimation.h"
#include "classes/openxr_android_scene_submesh_data.h"
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
#include "classes/openxr_meta_environment_depth.h"
#include "classes/openxr_meta_passthrough_color_lut.h"
#include "classes/openxr_ml_marker_detector.h"
#include "classes/openxr_ml_marker_detector_april_tag_settings.h"
#include "classes/openxr_ml_marker_detector_aruco_settings.h"
#include "classes/openxr_ml_marker_detector_code_128_settings.h"
#include "classes/openxr_ml_marker_detector_ean_13_settings.h"
#include "classes/openxr_ml_marker_detector_profile_settings.h"
#include "classes/openxr_ml_marker_detector_qr_settings.h"
#include "classes/openxr_ml_marker_detector_settings.h"
#include "classes/openxr_ml_marker_detector_upc_a_settings.h"
#include "classes/openxr_ml_marker_tracker.h"
#include "classes/openxr_ml_marker_understanding_manager.h"
#include "classes/openxr_vendor_performance_metrics.h"
#include "classes/openxr_vendor_performance_metrics_provider.h"

using namespace godot;

struct ExtensionSingleton {
	String class_name;
	OpenXRExtensionWrapper *extension;
};

static Vector<OpenXRExtensionWrapper *> extensions_registered;
static Vector<ExtensionSingleton> extensions_singletons;

static inline void _register_extension_with_openxr(OpenXRExtensionWrapper *p_extension) {
	p_extension->register_extension_wrapper();
	extensions_registered.push_back(p_extension);
}

static inline void _register_extension_as_singleton(OpenXRExtensionWrapper *p_extension) {
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

			GDREGISTER_ABSTRACT_CLASS(OpenXRVendorPerformanceMetricsProvider);
			GDREGISTER_CLASS(OpenXRVendorPerformanceMetrics);
			GDREGISTER_CLASS(OpenXRMetaPerformanceMetricsExtension);

			GDREGISTER_CLASS(OpenXRAndroidEyeTrackingExtension);
			GDREGISTER_CLASS(OpenXRAndroidFaceTrackingExtension);
			GDREGISTER_CLASS(OpenXRAndroidLightEstimationExtension);
			GDREGISTER_CLASS(OpenXRAndroidSceneMeshing);
			GDREGISTER_CLASS(OpenXRAndroidSceneMeshingExtension);
			GDREGISTER_CLASS(OpenXRAndroidSceneSubmeshData);

			GDREGISTER_CLASS(OpenXRAndroidPassthroughCameraStateExtension);
			GDREGISTER_CLASS(OpenXRAndroidPerformanceMetricsExtension);
			GDREGISTER_CLASS(OpenXRFbPassthroughExtension);
			GDREGISTER_CLASS(OpenXRFbRenderModelExtension);
			GDREGISTER_CLASS(OpenXRFbColorSpaceExtension);
			GDREGISTER_CLASS(OpenXRFbSceneCaptureExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntitySharingExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityStorageExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityStorageBatchExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityQueryExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityContainerExtension);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityUserExtension);
			GDREGISTER_CLASS(OpenXRMetaRecommendedLayerResolutionExtension);
			GDREGISTER_CLASS(OpenXRMetaSimultaneousHandsAndControllersExtension);
			GDREGISTER_CLASS(OpenXRMetaHeadsetIDExtension);
			GDREGISTER_CLASS(OpenXRMetaColocationDiscoveryExtension);
			GDREGISTER_CLASS(OpenXRMetaSpatialEntityMeshExtension);
			GDREGISTER_CLASS(OpenXRFbSceneExtension);
			GDREGISTER_CLASS(OpenXRFbFaceTrackingExtension);
			GDREGISTER_CLASS(OpenXRFbBodyTrackingExtension);
			GDREGISTER_CLASS(OpenXRFbHandTrackingMeshExtension);
			GDREGISTER_CLASS(OpenXRFbHandTrackingAimExtension);
			GDREGISTER_CLASS(OpenXRFbHandTrackingCapsulesExtension);
			GDREGISTER_CLASS(OpenXRFbCompositionLayerSecureContentExtension);
			GDREGISTER_CLASS(OpenXRFbCompositionLayerDepthTestExtension);
			GDREGISTER_CLASS(OpenXRFbCompositionLayerAlphaBlendExtension);
			GDREGISTER_CLASS(OpenXRFbCompositionLayerImageLayoutExtension);
			GDREGISTER_CLASS(OpenXRFbCompositionLayerSettingsExtension);
			GDREGISTER_CLASS(OpenXRFbAndroidSurfaceSwapchainCreateExtension);
			GDREGISTER_CLASS(OpenXRHtcFacialTrackingExtension);
			GDREGISTER_CLASS(OpenXRHtcPassthroughExtension);
			GDREGISTER_CLASS(OpenXRMlMarkerUnderstandingExtension);
			GDREGISTER_CLASS(OpenXRFbSpaceWarpExtension);
			GDREGISTER_CLASS(OpenXRMetaEnvironmentDepthExtension);
			GDREGISTER_CLASS(OpenXRAndroidEnvironmentDepthExtension);

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
			GDREGISTER_CLASS(OpenXRMetaBoundaryVisibilityExtension);
			GDREGISTER_CLASS(OpenXRStationaryReferenceSpaceExtension);
#endif // META_HEADERS_ENABLED

			if (_get_bool_project_setting("xr/openxr/extensions/meta/passthrough")) {
				_register_extension_with_openxr(OpenXRFbPassthroughExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/render_model")) {
				_register_extension_with_openxr(OpenXRFbRenderModelExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/color_space")) {
				_register_extension_with_openxr(OpenXRFbColorSpaceExtension::get_singleton());
			}

			// Three settings to match the three permissions for the Android manifest.
			bool meta_anchor_api = _get_bool_project_setting("xr/openxr/extensions/meta/anchor_api");
			bool meta_anchor_sharing = _get_bool_project_setting("xr/openxr/extensions/meta/anchor_sharing");
			bool meta_scene_api = _get_bool_project_setting("xr/openxr/extensions/meta/scene_api");
			if (meta_anchor_api || meta_anchor_sharing || meta_scene_api) {
				// There are needed by all of them.
				_register_extension_with_openxr(OpenXRFbSpatialEntityExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityStorageExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityStorageBatchExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityQueryExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbSpatialEntityContainerExtension::get_singleton());

				if (meta_scene_api) {
					_register_extension_with_openxr(OpenXRFbSceneExtension::get_singleton());
					_register_extension_with_openxr(OpenXRFbSceneCaptureExtension::get_singleton());
					_register_extension_with_openxr(OpenXRMetaSpatialEntityMeshExtension::get_singleton());
				}

				if (meta_anchor_sharing) {
					_register_extension_with_openxr(OpenXRFbSpatialEntitySharingExtension::get_singleton());
					_register_extension_with_openxr(OpenXRFbSpatialEntityUserExtension::get_singleton());
				}
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/dynamic_resolution")) {
				_register_extension_with_openxr(OpenXRMetaRecommendedLayerResolutionExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/face_tracking")) {
				_register_extension_with_openxr(OpenXRFbFaceTrackingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/body_tracking")) {
				_register_extension_with_openxr(OpenXRFbBodyTrackingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/headset_id")) {
				_register_extension_with_openxr(OpenXRMetaHeadsetIDExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/vendor_performance_metrics")) {
				_register_extension_with_openxr(OpenXRMetaPerformanceMetricsExtension::get_singleton());
				_register_extension_with_openxr(OpenXRAndroidPerformanceMetricsExtension::get_singleton());
			}

			// All of the hand tracking extensions depend on the Godot hand tracking setting being set first.
			if (_get_bool_project_setting("xr/openxr/extensions/hand_tracking")) {
				if (_get_bool_project_setting("xr/openxr/extensions/meta/hand_tracking_mesh")) {
					_register_extension_with_openxr(OpenXRFbHandTrackingMeshExtension::get_singleton());
				}

				if (_get_bool_project_setting("xr/openxr/extensions/meta/hand_tracking_aim")) {
					_register_extension_with_openxr(OpenXRFbHandTrackingAimExtension::get_singleton());
				}

				if (_get_bool_project_setting("xr/openxr/extensions/meta/hand_tracking_capsules")) {
					_register_extension_with_openxr(OpenXRFbHandTrackingCapsulesExtension::get_singleton());
				}

				if (_get_bool_project_setting("xr/openxr/extensions/meta/simultaneous_hands_and_controllers")) {
					_register_extension_with_openxr(OpenXRMetaSimultaneousHandsAndControllersExtension::get_singleton());
				}
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/composition_layer_settings")) {
				_register_extension_with_openxr(OpenXRFbCompositionLayerSecureContentExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerAlphaBlendExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerSettingsExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbAndroidSurfaceSwapchainCreateExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerDepthTestExtension::get_singleton());
				_register_extension_with_openxr(OpenXRFbCompositionLayerImageLayoutExtension::get_singleton());
			}

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
			if (_get_bool_project_setting("xr/openxr/extensions/meta/boundary_visibility")) {
				_register_extension_with_openxr(OpenXRMetaBoundaryVisibilityExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/stationary_reference_space")) {
				_register_extension_with_openxr(OpenXRStationaryReferenceSpaceExtension::get_singleton());
			}
#endif // META_HEADERS_ENABLED

			if (_get_bool_project_setting("xr/openxr/extensions/meta/colocation_discovery")) {
				_register_extension_with_openxr(OpenXRMetaColocationDiscoveryExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/application_space_warp")) {
				_register_extension_with_openxr(OpenXRFbSpaceWarpExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/meta/environment_depth")) {
				_register_extension_with_openxr(OpenXRMetaEnvironmentDepthExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/htc/face_tracking")) {
				_register_extension_with_openxr(OpenXRHtcFacialTrackingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/htc/passthrough")) {
				_register_extension_with_openxr(OpenXRHtcPassthroughExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/magic_leap/marker_understanding")) {
				_register_extension_with_openxr(OpenXRMlMarkerUnderstandingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/androidxr/passthrough_camera_state")) {
				_register_extension_with_openxr(OpenXRAndroidPassthroughCameraStateExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/androidxr/face_tracking")) {
				_register_extension_with_openxr(OpenXRAndroidFaceTrackingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/androidxr/eye_tracking")) {
				_register_extension_with_openxr(OpenXRAndroidEyeTrackingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/androidxr/light_estimation")) {
				_register_extension_with_openxr(OpenXRAndroidLightEstimationExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/androidxr/scene_meshing")) {
				_register_extension_with_openxr(OpenXRAndroidSceneMeshingExtension::get_singleton());
			}

			if (_get_bool_project_setting("xr/openxr/extensions/androidxr/environment_depth")) {
				_register_extension_with_openxr(OpenXRAndroidEnvironmentDepthExtension::get_singleton());
			}
		} break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			_register_extension_as_singleton(OpenXRFbPassthroughExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbRenderModelExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbColorSpaceExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSceneCaptureExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityStorageExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityQueryExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpatialEntityContainerExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSceneExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbHandTrackingAimExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbHandTrackingCapsulesExtension::get_singleton());
			_register_extension_as_singleton(OpenXRMetaSimultaneousHandsAndControllersExtension::get_singleton());
			_register_extension_as_singleton(OpenXRMetaHeadsetIDExtension::get_singleton());
			_register_extension_as_singleton(OpenXRMetaColocationDiscoveryExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbBodyTrackingExtension::get_singleton());
			_register_extension_as_singleton(OpenXRHtcFacialTrackingExtension::get_singleton());
			_register_extension_as_singleton(OpenXRHtcPassthroughExtension::get_singleton());
			_register_extension_as_singleton(OpenXRMlMarkerUnderstandingExtension::get_singleton());
			_register_extension_as_singleton(OpenXRAndroidEyeTrackingExtension::get_singleton());
			_register_extension_as_singleton(OpenXRAndroidFaceTrackingExtension::get_singleton());
			_register_extension_as_singleton(OpenXRAndroidLightEstimationExtension::get_singleton());
			_register_extension_as_singleton(OpenXRAndroidPassthroughCameraStateExtension::get_singleton());
			_register_extension_as_singleton(OpenXRAndroidSceneMeshingExtension::get_singleton());
			_register_extension_as_singleton(OpenXRFbSpaceWarpExtension::get_singleton());
			_register_extension_as_singleton(OpenXRMetaEnvironmentDepthExtension::get_singleton());
			_register_extension_as_singleton(OpenXRAndroidEnvironmentDepthExtension::get_singleton());

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
			_register_extension_as_singleton(OpenXRMetaBoundaryVisibilityExtension::get_singleton());
			_register_extension_as_singleton(OpenXRStationaryReferenceSpaceExtension::get_singleton());
#endif // META_HEADERS_ENABLED

			GDREGISTER_CLASS(OpenXRAndroidLightEstimation);

			GDREGISTER_CLASS(OpenXRFbRenderModel);
			GDREGISTER_CLASS(OpenXRFbHandTrackingMesh);
			GDREGISTER_CLASS(OpenXRFbSceneManager);
			GDREGISTER_CLASS(OpenXRFbSpatialAnchorManager);
			GDREGISTER_CLASS(OpenXRFbSpatialEntity);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityBatch);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityQuery);
			GDREGISTER_CLASS(OpenXRFbSpatialEntityUser);
			GDREGISTER_CLASS(OpenXRFbPassthroughGeometry);
			GDREGISTER_CLASS(OpenXRMetaPassthroughColorLut);
			GDREGISTER_CLASS(OpenXRMetaEnvironmentDepth);
			GDREGISTER_CLASS(OpenXRAndroidEnvironmentDepth);

			GDREGISTER_CLASS(OpenXRMlMarkerTracker);
			GDREGISTER_CLASS(OpenXRMlMarkerDetector);
			GDREGISTER_ABSTRACT_CLASS(OpenXRMlMarkerDetectorSettings);
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorProfileSettings);
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorAprilTagSettings);
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorArucoSettings);
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorCode128Settings)
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorEan13Settings);
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorQrSettings);
			GDREGISTER_CLASS(OpenXRMlMarkerDetectorUpcASettings);
			GDREGISTER_CLASS(OpenXRMlMarkerUnderstandingManager);

			GDREGISTER_CLASS(OpenXRHybridApp);
			Engine::get_singleton()->register_singleton("OpenXRHybridApp", OpenXRHybridApp::get_singleton());

			Engine::get_singleton()->register_singleton("OpenXRVendorPerformanceMetrics", OpenXRVendorPerformanceMetrics::get_singleton());
		} break;

		case MODULE_INITIALIZATION_LEVEL_EDITOR: {
			GDREGISTER_INTERNAL_CLASS(OpenXRVendorsEditorPlugin);
			GDREGISTER_INTERNAL_CLASS(OpenXRVendorsEditorExportPlugin);

			GDREGISTER_INTERNAL_CLASS(XrProjectSetupDialog);

			GDREGISTER_INTERNAL_CLASS(AndroidXREditorExportPlugin);
			GDREGISTER_INTERNAL_CLASS(KhronosEditorExportPlugin);
			GDREGISTER_INTERNAL_CLASS(LynxEditorExportPlugin);
			GDREGISTER_INTERNAL_CLASS(MagicleapEditorExportPlugin);
			GDREGISTER_INTERNAL_CLASS(MetaEditorExportPlugin);
			GDREGISTER_INTERNAL_CLASS(PicoEditorExportPlugin);

			EditorPlugins::add_by_type<OpenXRVendorsEditorPlugin>();

			Callable meta_environment_depth_setup_global_uniforms = callable_mp(OpenXRMetaEnvironmentDepthExtension::get_singleton(), &OpenXRMetaEnvironmentDepthExtension::setup_global_uniforms);
			meta_environment_depth_setup_global_uniforms.call_deferred();
			ProjectSettings::get_singleton()->connect("settings_changed", meta_environment_depth_setup_global_uniforms);

			Callable android_environment_depth_setup_global_uniforms = callable_mp(OpenXRAndroidEnvironmentDepthExtension::get_singleton(), &OpenXRAndroidEnvironmentDepthExtension::setup_global_uniforms);
			android_environment_depth_setup_global_uniforms.call_deferred();
			ProjectSettings::get_singleton()->connect("settings_changed", android_environment_depth_setup_global_uniforms);
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

			Engine::get_singleton()->unregister_singleton("OpenXRVendorPerformanceMetrics");
			memdelete(OpenXRVendorPerformanceMetrics::get_singleton());
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

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/vendor_performance_metrics", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/vendor_performance_metrics/capture_on_startup", true);

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/htc/passthrough", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/htc/face_tracking", false);

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/passthrough", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/face_tracking", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/body_tracking", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/hand_tracking_aim", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/hand_tracking_mesh", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/hand_tracking_capsules", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/simultaneous_hands_and_controllers", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/render_model", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/anchor_api", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/anchor_sharing", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/scene_api", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/color_space", true);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/composition_layer_settings", true);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/dynamic_resolution", true);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/headset_id", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/colocation_discovery", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/application_space_warp", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/environment_depth", false);

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/magic_leap/marker_understanding", false);

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/androidxr/eye_tracking", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/androidxr/face_tracking", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/androidxr/light_estimation", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/androidxr/passthrough_camera_state", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/androidxr/scene_meshing", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/androidxr/environment_depth", false);

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/meta/boundary_visibility", false);

	_add_bool_project_setting(project_settings, "xr/openxr/extensions/stationary_reference_space", false);
	_add_bool_project_setting(project_settings, "xr/openxr/extensions/stationary_reference_space/enable_on_startup", false);
#endif // META_HEADERS_ENABLED

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

	{
		String starting_color_space = "xr/openxr/extensions/meta/color_space/starting_color_space";
		if (!project_settings->has_setting(starting_color_space)) {
			project_settings->set_setting(starting_color_space, 0);
		}

		project_settings->set_initial_value(starting_color_space, 0);
		project_settings->set_as_basic(starting_color_space, false);
		Dictionary property_info;
		property_info["name"] = starting_color_space;
		property_info["type"] = Variant::Type::INT;
		property_info["hint"] = PROPERTY_HINT_ENUM;
		property_info["hint_string"] = "Runtime Default:0,Unmanaged:1,REC2020:2,REC709:3,Rift CV1:4,Rift S:5,Quest:6,P3:7,Adobe RGB:8";
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
