/**************************************************************************/
/*  openxr_fb_scene_extension_wrapper.cpp                                 */
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

#include "include/openxr_fb_scene_extension_wrapper.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/object.hpp>

#include "include/openxr_fb_spatial_entity_extension_wrapper.h"

using namespace godot;

OpenXRFbSceneExtensionWrapper *OpenXRFbSceneExtensionWrapper::singleton = nullptr;

OpenXRFbSceneExtensionWrapper *OpenXRFbSceneExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSceneExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSceneExtensionWrapper::OpenXRFbSceneExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSceneExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SCENE_EXTENSION_NAME] = &fb_scene_ext;
	singleton = this;
}

OpenXRFbSceneExtensionWrapper::~OpenXRFbSceneExtensionWrapper() {
	cleanup();
}

void OpenXRFbSceneExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_scene_supported"), &OpenXRFbSceneExtensionWrapper::is_scene_supported);
}

void OpenXRFbSceneExtensionWrapper::cleanup() {
	fb_scene_ext = false;
}

Dictionary OpenXRFbSceneExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext: request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSceneExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_scene_ext) {
		bool result = initialize_fb_scene_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_scene extension");
			fb_scene_ext = false;
		}
	}
}

void OpenXRFbSceneExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

std::optional<String> OpenXRFbSceneExtensionWrapper::get_semantic_labels(const XrSpace& space) {
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(space, XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB)) {
		return std::nullopt;
	}

	// List from https://developer.oculus.com/documentation/native/android/mobile-scene-api-ref/
	static const CharString recognizedLabels =
			"CEILING,DOOR_FRAME,FLOOR,INVISIBLE_WALL_FACE,WALL_ART,WALL_FACE,WINDOW_FRAME,COUCH,TABLE,BED,LAMP,PLANT,SCREEN,STORAGE,GLOBAL_MESH,OTHER";
	const XrSemanticLabelsSupportInfoFB semanticLabelsSupportInfo = {
		XR_TYPE_SEMANTIC_LABELS_SUPPORT_INFO_FB,
		nullptr,
		XR_SEMANTIC_LABELS_SUPPORT_MULTIPLE_SEMANTIC_LABELS_BIT_FB | XR_SEMANTIC_LABELS_SUPPORT_ACCEPT_DESK_TO_TABLE_MIGRATION_BIT_FB |
		XR_SEMANTIC_LABELS_SUPPORT_ACCEPT_INVISIBLE_WALL_FACE_BIT_FB,
		recognizedLabels.ptr(),
	};

	XrSemanticLabelsFB labels = {XR_TYPE_SEMANTIC_LABELS_FB, &semanticLabelsSupportInfo, 0};

	// First call.
	xrGetSpaceSemanticLabelsFB(SESSION, space, &labels);
	// Second call
	Vector<char> labelData;
	labelData.resize(labels.bufferCountOutput);
	labels.bufferCapacityInput = labelData.size();
	labels.buffer = labelData.ptrw();
	xrGetSpaceSemanticLabelsFB(SESSION, space, &labels);

	// std::string necessary since buffer may not be null terminated
	return String(std::string(labels.buffer, labels.bufferCountOutput).c_str());
}

void OpenXRFbSceneExtensionWrapper::get_shapes(const XrSpace& space, XrSceneObjectInternal& object) {
	if (OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(space, XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB)) {
		//  Grab both the bounding box 2D and the boundary
		XrRect2Df boundingBox2D;
		if (XR_SUCCEEDED(xrGetSpaceBoundingBox2DFB(SESSION, space, &boundingBox2D))) {
			object.boundingBox2D = boundingBox2D;
		}

		XrBoundary2DFB boundary2D = {XR_TYPE_BOUNDARY_2D_FB, nullptr, 0};
		if (XR_SUCCEEDED(xrGetSpaceBoundary2DFB(SESSION, space, &boundary2D))) {
			Vector<XrVector2f> vertices;
			vertices.resize(boundary2D.vertexCountOutput);
			boundary2D.vertexCapacityInput = vertices.size();
			boundary2D.vertices = vertices.ptrw();
			if (XR_SUCCEEDED(xrGetSpaceBoundary2DFB(SESSION, space, &boundary2D))) {
				object.boundary2D = vertices;
			}
		}
	}

	if (OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(space, XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB)) {
		XrRect3DfFB boundingBox3D;
		if (XR_SUCCEEDED(xrGetSpaceBoundingBox3DFB(SESSION, space, &boundingBox3D))) {
			object.boundingBox3D = boundingBox3D;
		}
	}

	// TODO: Need to enable the extension for this
	// if (is_component_enabled(space, XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META)) {
	// 	WARN_PRINT("Found component with XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META");
	// }
}

bool OpenXRFbSceneExtensionWrapper::initialize_fb_scene_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceBoundingBox2DFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceBoundingBox3DFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceSemanticLabelsFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceBoundary2DFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceRoomLayoutFB);

	return true;
}
