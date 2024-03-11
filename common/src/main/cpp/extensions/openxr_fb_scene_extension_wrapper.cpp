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

#include "extensions/openxr_fb_scene_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#ifdef META_VENDOR_ENABLED
#include <openxr/fb_scene.h>
#endif

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

using namespace godot;

// List from https://developer.oculus.com/documentation/native/android/mobile-scene-api-ref/
static const char *SUPPORTED_SEMANTIC_LABELS = "CEILING,DOOR_FRAME,FLOOR,INVISIBLE_WALL_FACE,WALL_ART,WALL_FACE,WINDOW_FRAME,COUCH,TABLE,BED,LAMP,PLANT,SCREEN,STORAGE,GLOBAL_MESH,OTHER";

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
	for (auto ext : request_extensions) {
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

const PackedStringArray &OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels() {
	static PackedStringArray semantic_labels = String(SUPPORTED_SEMANTIC_LABELS).split(",");
	return semantic_labels;
}

PackedStringArray OpenXRFbSceneExtensionWrapper::get_semantic_labels(const XrSpace p_space) {
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(p_space, XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB)) {
		return PackedStringArray();
	}

	XrSemanticLabelsSupportFlagsFB flags = XR_SEMANTIC_LABELS_SUPPORT_MULTIPLE_SEMANTIC_LABELS_BIT_FB | XR_SEMANTIC_LABELS_SUPPORT_ACCEPT_DESK_TO_TABLE_MIGRATION_BIT_FB;
#ifdef META_VENDOR_ENABLED
	flags |= XR_SEMANTIC_LABELS_SUPPORT_ACCEPT_INVISIBLE_WALL_FACE_BIT_FB;
#endif

	const XrSemanticLabelsSupportInfoFB semanticLabelsSupportInfo = {
		XR_TYPE_SEMANTIC_LABELS_SUPPORT_INFO_FB,
		nullptr,
		flags,
		SUPPORTED_SEMANTIC_LABELS,
	};

	XrSemanticLabelsFB labels = { XR_TYPE_SEMANTIC_LABELS_FB, &semanticLabelsSupportInfo, 0 };

	// First call.
	xrGetSpaceSemanticLabelsFB(SESSION, p_space, &labels);

	// Second call
	CharString label_data;
	label_data.resize(labels.bufferCountOutput + 1);
	labels.bufferCapacityInput = labels.bufferCountOutput;
	labels.buffer = label_data.ptrw();
	xrGetSpaceSemanticLabelsFB(SESSION, p_space, &labels);

	label_data[label_data.size() - 1] = '\0';
	return String(label_data).split(",");
}

bool OpenXRFbSceneExtensionWrapper::get_room_layout(const XrSpace p_space, RoomLayout &r_room_layout) {
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(p_space, XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB)) {
		return false;
	}

	XrResult result;

	XrRoomLayoutFB room_layout = {
		XR_TYPE_ROOM_LAYOUT_FB, // type
		nullptr, // next
		{}, // floorUuid
		{}, // ceilingUuid
		0, // wallUuidCapacityInput
		0, // wallUuidCountOutput
		nullptr, // wallUuids
	};

	result = xrGetSpaceRoomLayoutFB(SESSION, p_space, &room_layout);
	if (XR_FAILED(result)) {
		WARN_PRINT("xrGetSpaceRoomLayoutFB failed to get wall count!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return false;
	}

	r_room_layout.walls.resize(room_layout.wallUuidCountOutput);
	room_layout.wallUuidCapacityInput = room_layout.wallUuidCountOutput;
	room_layout.wallUuids = r_room_layout.walls.ptrw();

	result = xrGetSpaceRoomLayoutFB(SESSION, p_space, &room_layout);
	if (XR_FAILED(result)) {
		WARN_PRINT("xrGetSpaceRoomLayoutFB failed to get room layout!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return false;
	}

	r_room_layout.ceiling = room_layout.ceilingUuid;
	r_room_layout.floor = room_layout.floorUuid;

	return true;
}

Rect2 OpenXRFbSceneExtensionWrapper::get_bounding_box_2d(const XrSpace p_space) {
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(p_space, XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB)) {
		return Rect2();
	}

	XrRect2Df bounding_box;

	XrResult result = xrGetSpaceBoundingBox2DFB(SESSION, p_space, &bounding_box);
	if (XR_FAILED(result)) {
		WARN_PRINT("xrGetSpaceBoundingBox2DFB failed to bounding box!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return Rect2();
	}

	return Rect2(
		Vector2(bounding_box.offset.x, bounding_box.offset.y),
		Vector2(bounding_box.extent.width, bounding_box.extent.height));
}

AABB OpenXRFbSceneExtensionWrapper::get_bounding_box_3d(const XrSpace p_space) {
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(p_space, XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB)) {
		return AABB();
	}

	XrRect3DfFB bounding_box;

	XrResult result = xrGetSpaceBoundingBox3DFB(SESSION, p_space, &bounding_box);
	if (XR_FAILED(result)) {
		WARN_PRINT("xrGetSpaceBoundingBox3DFB failed to bounding box!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return AABB();
	}

	return AABB(
		Vector3(bounding_box.offset.x, bounding_box.offset.y, bounding_box.offset.z),
		Vector3(bounding_box.extent.width, bounding_box.extent.height, bounding_box.extent.depth));
}

PackedVector2Array OpenXRFbSceneExtensionWrapper::get_boundary_2d(const XrSpace p_space) {
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(p_space, XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB)) {
		return PackedVector2Array();
	}

	XrResult result;

	XrBoundary2DFB boundary = {
		XR_TYPE_BOUNDARY_2D_FB, // type
		nullptr, // next
		0, // vertexCapacityInput
		0, // vertexCapacityOutput
		nullptr, // vertices
	};

	result = xrGetSpaceBoundary2DFB(SESSION, p_space, &boundary);
	if (XR_FAILED(result)) {
		WARN_PRINT("xrGetSpaceBoundary2DFB failed to get vertex count!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return PackedVector2Array();
	}

	LocalVector<XrVector2f> vertices;
	vertices.resize(boundary.vertexCountOutput);
	boundary.vertexCapacityInput = vertices.size();
	boundary.vertices = vertices.ptr();

	result = xrGetSpaceBoundary2DFB(SESSION, p_space, &boundary);
	if (XR_FAILED(result)) {
		WARN_PRINT("xrGetSpaceBoundary2DFB failed to get boundary!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		return PackedVector2Array();
	}

	PackedVector2Array ret;
	ret.resize(boundary.vertexCountOutput);
	for (int i = 0; i < boundary.vertexCountOutput; i++) {
		XrVector2f vertex = vertices[i];
		ret[i] = Vector2(vertex.x, vertex.y);
	}

	return ret;
}

bool OpenXRFbSceneExtensionWrapper::initialize_fb_scene_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceBoundingBox2DFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceBoundingBox3DFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceSemanticLabelsFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceBoundary2DFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceRoomLayoutFB);

	return true;
}
