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
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "export/export_plugin.h"
#include "export/khronos_export_plugin.h"
#include "export/lynx_export_plugin.h"
#include "export/meta_export_plugin.h"
#include "export/pico_export_plugin.h"

#include "extensions/openxr_fb_body_tracking_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_secure_content_extension_wrapper.h"
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
#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"
#include "extensions/openxr_htc_facial_tracking_extension_wrapper.h"
#include "extensions/openxr_meta_spatial_entity_mesh_extension_wrapper.h"

#include "classes/openxr_fb_hand_tracking_mesh.h"
#include "classes/openxr_fb_passthrough_geometry.h"
#include "classes/openxr_fb_render_model.h"
#include "classes/openxr_fb_scene_manager.h"
#include "classes/openxr_fb_spatial_anchor_manager.h"
#include "classes/openxr_fb_spatial_entity.h"
#include "classes/openxr_fb_spatial_entity_query.h"
#include "classes/openxr_meta_passthrough_color_lut.h"

using namespace godot;

void initialize_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE: {
			ClassDB::register_class<OpenXRFbPassthroughExtensionWrapper>();
			OpenXRFbPassthroughExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbRenderModelExtensionWrapper>();
			OpenXRFbRenderModelExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbSceneCaptureExtensionWrapper>();
			OpenXRFbSceneCaptureExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbSpatialEntityExtensionWrapper>();
			OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbSpatialEntityStorageExtensionWrapper>();
			OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbSpatialEntityQueryExtensionWrapper>();
			OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbSpatialEntityContainerExtensionWrapper>();
			OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRMetaSpatialEntityMeshExtensionWrapper>();
			OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbSceneExtensionWrapper>();
			OpenXRFbSceneExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbFaceTrackingExtensionWrapper>();
			OpenXRFbFaceTrackingExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbBodyTrackingExtensionWrapper>();
			OpenXRFbBodyTrackingExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbHandTrackingMeshExtensionWrapper>();
			OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbHandTrackingAimExtensionWrapper>();
			OpenXRFbHandTrackingAimExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbHandTrackingCapsulesExtensionWrapper>();
			OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRFbCompositionLayerSecureContentExtensionWrapper>();
			OpenXRFbCompositionLayerSecureContentExtensionWrapper::get_singleton()->register_extension_wrapper();

			ClassDB::register_class<OpenXRHtcFacialTrackingExtensionWrapper>();
			OpenXRHtcFacialTrackingExtensionWrapper::get_singleton()->register_extension_wrapper();

		} break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			Engine::get_singleton()->register_singleton("OpenXRFbPassthroughExtensionWrapper", OpenXRFbPassthroughExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbRenderModelExtensionWrapper", OpenXRFbRenderModelExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbSceneCaptureExtensionWrapper", OpenXRFbSceneCaptureExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbSpatialEntityExtensionWrapper", OpenXRFbSpatialEntityExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbSpatialEntityStorageExtensionWrapper", OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbSpatialEntityQueryExtensionWrapper", OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbSpatialEntityContainerExtensionWrapper", OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbSceneExtensionWrapper", OpenXRFbSceneExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbHandTrackingAimExtensionWrapper", OpenXRFbHandTrackingAimExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper", OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton());
			Engine::get_singleton()->register_singleton("OpenXRHtcFacialTrackingExtensionWrapper", OpenXRHtcFacialTrackingExtensionWrapper::get_singleton());

			ClassDB::register_class<OpenXRFbRenderModel>();
			ClassDB::register_class<OpenXRFbHandTrackingMesh>();
			ClassDB::register_class<OpenXRFbSceneManager>();
			ClassDB::register_class<OpenXRFbSpatialAnchorManager>();
			ClassDB::register_class<OpenXRFbSpatialEntity>();
			ClassDB::register_class<OpenXRFbSpatialEntityQuery>();
			ClassDB::register_class<OpenXRFbPassthroughGeometry>();
			ClassDB::register_class<OpenXRMetaPassthroughColorLut>();

			OpenXRFbHandTrackingAimExtensionWrapper::get_singleton()->add_project_setting();
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

			ClassDB::register_class<PicoEditorPlugin>();
			EditorPlugins::add_by_type<PicoEditorPlugin>();
		} break;

		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}

void terminate_plugin_module(ModuleInitializationLevel p_level) {
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
