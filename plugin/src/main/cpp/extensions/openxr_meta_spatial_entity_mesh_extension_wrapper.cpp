/**************************************************************************/
/*  openxr_fb_spatial_entity_extension_wrapper.cpp                        */
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

#include "extensions/openxr_meta_spatial_entity_mesh_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

using namespace godot;

OpenXRMetaSpatialEntityMeshExtensionWrapper *OpenXRMetaSpatialEntityMeshExtensionWrapper::singleton = nullptr;

OpenXRMetaSpatialEntityMeshExtensionWrapper *OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaSpatialEntityMeshExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaSpatialEntityMeshExtensionWrapper::OpenXRMetaSpatialEntityMeshExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaSpatialEntityMeshExtensionWrapper singleton already exists.");

	request_extensions[XR_META_SPATIAL_ENTITY_MESH_EXTENSION_NAME] = &meta_spatial_entity_mesh_ext;
	singleton = this;
}

OpenXRMetaSpatialEntityMeshExtensionWrapper::~OpenXRMetaSpatialEntityMeshExtensionWrapper() {
	cleanup();
}

void OpenXRMetaSpatialEntityMeshExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_mesh_supported"), &OpenXRMetaSpatialEntityMeshExtensionWrapper::is_spatial_entity_mesh_supported);
}

void OpenXRMetaSpatialEntityMeshExtensionWrapper::cleanup() {
	meta_spatial_entity_mesh_ext = false;
}

Dictionary OpenXRMetaSpatialEntityMeshExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRMetaSpatialEntityMeshExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (meta_spatial_entity_mesh_ext) {
		bool result = initialize_meta_spatial_entity_mesh_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity extension");
			meta_spatial_entity_mesh_ext = false;
		}
	}
}

void OpenXRMetaSpatialEntityMeshExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRMetaSpatialEntityMeshExtensionWrapper::initialize_meta_spatial_entity_mesh_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceTriangleMeshMETA);

	return true;
}

bool OpenXRMetaSpatialEntityMeshExtensionWrapper::get_triangle_mesh(const XrSpace &p_space, TriangleMesh &r_triangle_mesh) {
	if (!meta_spatial_entity_mesh_ext) {
		return false;
	}
	if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(p_space, XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META)) {
		return false;
	}

	XrSpaceTriangleMeshGetInfoMETA info = {
		XR_TYPE_SPACE_TRIANGLE_MESH_GET_INFO_META, // type
		nullptr, // next
	};

	XrSpaceTriangleMeshMETA mesh_data = {
		XR_TYPE_SPACE_TRIANGLE_MESH_META, // type
		nullptr, // next
		0, // vertexCapacityInput
		0, // vertexCoutOutput
		nullptr, // vertices
		0, // indexCapacityInput
		0, // indexCountOutput
		nullptr, // indices
	};

	XrResult result;

	result = xrGetSpaceTriangleMeshMETA(p_space, &info, &mesh_data);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to retrieve vertex and index count from xrGetSpaceTriangleMeshMETA, error code: ", result);
		return false;
	}

	mesh_data.vertexCapacityInput = mesh_data.vertexCountOutput;
	r_triangle_mesh.vertices.resize(mesh_data.vertexCapacityInput);
	mesh_data.vertices = r_triangle_mesh.vertices.ptrw();

	mesh_data.indexCapacityInput = mesh_data.indexCountOutput;
	r_triangle_mesh.indices.resize(mesh_data.indexCapacityInput);
	mesh_data.indices = r_triangle_mesh.indices.ptrw();

	result = xrGetSpaceTriangleMeshMETA(p_space, &info, &mesh_data);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to retrieve triangle mesh data from xrGetSpaceTriangleMeshMETA, error code: ", result);
		return false;
	}

	return true;
}
