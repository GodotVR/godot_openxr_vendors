/**************************************************************************/
/*  openxr_fb_spatial_entity_container_extension_wrapper.cpp              */
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

#include "extensions/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbSpatialEntityContainerExtensionWrapper *OpenXRFbSpatialEntityContainerExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityContainerExtensionWrapper *OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityContainerExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityContainerExtensionWrapper::OpenXRFbSpatialEntityContainerExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntityContainerExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_CONTAINER_EXTENSION_NAME] = &fb_spatial_entity_container_ext;
	singleton = this;
}

OpenXRFbSpatialEntityContainerExtensionWrapper::~OpenXRFbSpatialEntityContainerExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityContainerExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_container_supported"), &OpenXRFbSpatialEntityContainerExtensionWrapper::is_spatial_entity_container_supported);
}

void OpenXRFbSpatialEntityContainerExtensionWrapper::cleanup() {
	fb_spatial_entity_container_ext = false;
}

Dictionary OpenXRFbSpatialEntityContainerExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityContainerExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_container_ext) {
		bool result = initialize_fb_spatial_entity_container_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity_container extension");
			fb_spatial_entity_container_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityContainerExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityContainerExtensionWrapper::initialize_fb_spatial_entity_container_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceContainerFB);
	return true;
}

Vector<XrUuidEXT> OpenXRFbSpatialEntityContainerExtensionWrapper::get_contained_uuids(const XrSpace &space) {
	XrSpaceContainerFB spaceContainer = { XR_TYPE_SPACE_CONTAINER_FB };
	xrGetSpaceContainerFB(SESSION, space, &spaceContainer);
	Vector<XrUuidEXT> uuids;
	uuids.resize(spaceContainer.uuidCountOutput);
	spaceContainer.uuidCapacityInput = uuids.size();
	spaceContainer.uuids = uuids.ptrw();
	xrGetSpaceContainerFB(SESSION, space, &spaceContainer);
	return uuids;
}
