/**************************************************************************/
/*  openxr_fb_render_model_extension_wrapper.cpp                          */
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

#include "extensions/openxr_fb_render_model_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbRenderModelExtensionWrapper *OpenXRFbRenderModelExtensionWrapper::singleton = nullptr;

OpenXRFbRenderModelExtensionWrapper *OpenXRFbRenderModelExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbRenderModelExtensionWrapper());
	}
	return singleton;
}

OpenXRFbRenderModelExtensionWrapper::OpenXRFbRenderModelExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbRenderModelExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_RENDER_MODEL_EXTENSION_NAME] = &fb_render_model_ext;
	singleton = this;
}

OpenXRFbRenderModelExtensionWrapper::~OpenXRFbRenderModelExtensionWrapper() {
	cleanup();
}

void OpenXRFbRenderModelExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbRenderModelExtensionWrapper::is_enabled);
}

void OpenXRFbRenderModelExtensionWrapper::cleanup() {
	fb_render_model_ext = false;
}

uint64_t OpenXRFbRenderModelExtensionWrapper::_set_system_properties_and_get_next_pointer(void *next_pointer) {
	system_render_model_properties.type = XR_TYPE_SYSTEM_RENDER_MODEL_PROPERTIES_FB;
	system_render_model_properties.next = next_pointer;
	system_render_model_properties.supportsRenderModelLoading = false;
	return reinterpret_cast<uint64_t>(&system_render_model_properties);
}

godot::Dictionary OpenXRFbRenderModelExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbRenderModelExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_render_model_ext) {
		bool result = initialize_fb_render_model_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_render_model extension");
			fb_render_model_ext = false;
		}
	}
}

void OpenXRFbRenderModelExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbRenderModelExtensionWrapper::_on_state_ready() {
	openxr_session_active = true;
}

void OpenXRFbRenderModelExtensionWrapper::_on_state_stopping() {
	openxr_session_active = false;
	paths_fetched = false;
}

bool OpenXRFbRenderModelExtensionWrapper::is_enabled() const {
	return fb_render_model_ext && system_render_model_properties.supportsRenderModelLoading;
}

void OpenXRFbRenderModelExtensionWrapper::fetch_paths() {
	XrResult result;
	uint32_t path_count = 0;

	result = xrEnumerateRenderModelPathsFB(SESSION, 0, &path_count, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get path count using FB_render_model extension, error code: ", result);
		return;
	}

	XrRenderModelPathInfoFB *paths = reinterpret_cast<XrRenderModelPathInfoFB *>(memalloc(sizeof(XrRenderModelPathInfoFB) * path_count));
	for (int i = 0; i < path_count; i++) {
		paths[i].type = XR_TYPE_RENDER_MODEL_PATH_INFO_FB;
		paths[i].next = nullptr;
	}

	result = xrEnumerateRenderModelPathsFB(SESSION, path_count, &path_count, paths);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get paths using FB_render_model extension, error code: ", result);
		return;
	}

	memfree(paths);

	paths_fetched = true;
}

PackedByteArray OpenXRFbRenderModelExtensionWrapper::get_buffer(const String &p_path) {
	if (!is_enabled()) {
		return PackedByteArray();
	}

	if (!paths_fetched) {
		fetch_paths();
	}

	XrResult result;
	XrPath xr_path = _string_to_xr_path(p_path);

	// get render model properites
	XrRenderModelCapabilitiesRequestFB model_capabilities = {
		XR_TYPE_RENDER_MODEL_CAPABILITIES_REQUEST_FB,
		nullptr,
		XR_RENDER_MODEL_SUPPORTS_GLTF_2_0_SUBSET_2_BIT_FB
	};

	XrRenderModelPropertiesFB model_properties = { XR_TYPE_RENDER_MODEL_PROPERTIES_FB, &model_capabilities };
	result = xrGetRenderModelPropertiesFB(SESSION, xr_path, &model_properties);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get XrRenderModelPropertiesFB from XrPath, error code: ", result);
		return PackedByteArray();
	}

	// load render model
	XrRenderModelBufferFB model_buffer = { XR_TYPE_RENDER_MODEL_BUFFER_FB, nullptr };
	XrRenderModelLoadInfoFB model_info = { XR_TYPE_RENDER_MODEL_LOAD_INFO_FB, nullptr };
	model_info.modelKey = model_properties.modelKey;
	result = xrLoadRenderModelFB(SESSION, &model_info, &model_buffer);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get XrRenderModelBufferFB buffer count output, error code ", result);
		return PackedByteArray();
	}

	model_buffer.bufferCapacityInput = model_buffer.bufferCountOutput;
	PackedByteArray ret;
	ret.resize(model_buffer.bufferCapacityInput);
	model_buffer.buffer = ret.ptrw();
	result = xrLoadRenderModelFB(SESSION, &model_info, &model_buffer);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to load XrRenderModelBufferFB buffer, error code ", result);
		return PackedByteArray();
	}

	return ret;
}

XrPath OpenXRFbRenderModelExtensionWrapper::_string_to_xr_path(const String &p_path) {
	XrPath xr_path;
	XrResult result = xrStringToPath((XrInstance)get_openxr_api()->get_instance(), p_path.utf8().get_data(), &xr_path);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to convert string path to XrPath, error code: ", result);
	}
	return xr_path;
}

bool OpenXRFbRenderModelExtensionWrapper::initialize_fb_render_model_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateRenderModelPathsFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetRenderModelPropertiesFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrLoadRenderModelFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrStringToPath);

	return true;
}
