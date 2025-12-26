/**************************************************************************/
/*  openxr_android_scene_meshing_extension_wrapper.cpp                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "extensions/openxr_android_scene_meshing_extension_wrapper.h"
#include "godot_cpp/core/class_db.hpp"
#include <androidxr/androidxr.h>

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidSceneMeshingExtensionWrapper *OpenXRAndroidSceneMeshingExtensionWrapper::singleton = nullptr;

OpenXRAndroidSceneMeshingExtensionWrapper *OpenXRAndroidSceneMeshingExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidSceneMeshingExtensionWrapper());
	}
	return singleton;
}

OpenXRAndroidSceneMeshingExtensionWrapper::OpenXRAndroidSceneMeshingExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidSceneMeshingExtensionWrapper singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_SCENE_MESHING_EXTENSION_NAME] = &available;
}

OpenXRAndroidSceneMeshingExtensionWrapper::~OpenXRAndroidSceneMeshingExtensionWrapper() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidSceneMeshingExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRAndroidSceneMeshingExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	scene_meshing_properties.type = XR_TYPE_SYSTEM_SCENE_MESHING_PROPERTIES_ANDROID;
	scene_meshing_properties.next = p_next_pointer;
	scene_meshing_properties.supportsSceneMeshing = false;
	return reinterpret_cast<uint64_t>(&scene_meshing_properties);
}

void OpenXRAndroidSceneMeshingExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	// do not check scene_meshing_properties since _set_system_properties_and_get_next_pointer() is called after _on_instance_created()
	if (!available) {
		return;
	}

	if (!_initialize_androidxr_scene_meshing_extension()) {
		UtilityFunctions::printerr("Failed to initialize scene meshing extension");
		available = false;
		return;
	}
}

void OpenXRAndroidSceneMeshingExtensionWrapper::_on_session_created(uint64_t instance) {
	available = available && XR_TRUE == scene_meshing_properties.supportsSceneMeshing;
}

Array OpenXRAndroidSceneMeshingExtensionWrapper::get_supported_semantic_label_sets() {
	Array ret;
	if (!available) {
		return ret;
	}

	uint32_t supported_semantic_label_sets_output_count = 0;
	XrInstance xr_instance = (XrInstance)get_openxr_api()->get_instance();
	XrSystemId xr_system_id = (XrSystemId)get_openxr_api()->get_system_id();
	XrResult result = xrEnumerateSupportedSemanticLabelSetsANDROID(xr_instance, xr_system_id, 0, &supported_semantic_label_sets_output_count, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate semantic label sets; ", get_openxr_api()->get_error_string(result));
		available = false;
		return ret;
	}

	if (supported_semantic_label_sets_output_count == 0) {
		WARN_PRINT("OpenXR: enumerate semantic label sets returned zero");
		available = false;
		return ret;
	}

	LocalVector<XrSceneMeshSemanticLabelSetANDROID> semantic_label_sets;
	semantic_label_sets.resize(supported_semantic_label_sets_output_count);
	result = xrEnumerateSupportedSemanticLabelSetsANDROID(xr_instance, xr_system_id, supported_semantic_label_sets_output_count, &supported_semantic_label_sets_output_count, semantic_label_sets.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate semantic label sets; ", get_openxr_api()->get_error_string(result));
		available = false;
		return ret;
	}

	if (semantic_label_sets.size() != supported_semantic_label_sets_output_count) {
		WARN_PRINT("OpenXR: enumerate semantic label sets returned a different count");
		if (semantic_label_sets.size() < supported_semantic_label_sets_output_count) {
			UtilityFunctions::printerr("OpenXR: somehow received more semantic label sets on the second query; ", get_openxr_api()->get_error_string(result));
			available = false;
			return ret;
		}

		if (supported_semantic_label_sets_output_count == 0) {
			UtilityFunctions::printerr("OpenXR: second enumerate semantic label sets returned zero");
			available = false;
			return ret;
		}

		semantic_label_sets.resize(supported_semantic_label_sets_output_count);
	}

	for (XrSceneMeshSemanticLabelSetANDROID semantic_label_set : semantic_label_sets) {
		switch (semantic_label_set) {
			case XR_SCENE_MESH_SEMANTIC_LABEL_SET_NONE_ANDROID:
				ret.push_back(OpenXRAndroidSceneMeshing::SEMANTIC_LABEL_SET_NONE);
				break;
			case XR_SCENE_MESH_SEMANTIC_LABEL_SET_DEFAULT_ANDROID:
				ret.push_back(OpenXRAndroidSceneMeshing::SEMANTIC_LABEL_SET_DEFAULT);
				break;
			default:
				UtilityFunctions::printerr("OpenXR: received an unknown semantic label set from the xr runtime: ", semantic_label_set);
				break;
		}
	}

	return ret;
}

void OpenXRAndroidSceneMeshingExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_supported_semantic_label_sets"), &OpenXRAndroidSceneMeshingExtensionWrapper::get_supported_semantic_label_sets);
}

bool OpenXRAndroidSceneMeshingExtensionWrapper::_initialize_androidxr_scene_meshing_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSupportedSemanticLabelSetsANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSceneMeshingTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySceneMeshingTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSceneMeshSnapshotANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySceneMeshSnapshotANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetAllSubmeshStatesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSubmeshDataANDROID);
	return true;
}
