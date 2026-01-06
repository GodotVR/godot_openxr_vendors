/**************************************************************************/
/*  openxr_meta_recommended_layer_resolution_extension_wrapper.cpp        */
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

#include "extensions/openxr_meta_recommended_layer_resolution_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRMetaRecommendedLayerResolutionExtensionWrapper *OpenXRMetaRecommendedLayerResolutionExtensionWrapper::singleton = nullptr;

OpenXRMetaRecommendedLayerResolutionExtensionWrapper *OpenXRMetaRecommendedLayerResolutionExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaRecommendedLayerResolutionExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaRecommendedLayerResolutionExtensionWrapper::OpenXRMetaRecommendedLayerResolutionExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaRecommendedLayerResolutionExtensionWrapper singleton already exists.");

	request_extensions[XR_META_RECOMMENDED_LAYER_RESOLUTION_EXTENSION_NAME] = &meta_recommended_layer_resolution_ext;
	singleton = this;
}

OpenXRMetaRecommendedLayerResolutionExtensionWrapper::~OpenXRMetaRecommendedLayerResolutionExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRMetaRecommendedLayerResolutionExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRMetaRecommendedLayerResolutionExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (meta_recommended_layer_resolution_ext) {
		bool result = initialize_meta_recommended_layer_resolution_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize meta_recommended_layer_resolution extension");
			meta_recommended_layer_resolution_ext = false;
		}
	}
}

void OpenXRMetaRecommendedLayerResolutionExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRMetaRecommendedLayerResolutionExtensionWrapper::_on_pre_render() {
	if (!meta_recommended_layer_resolution_ext || get_openxr_api().is_null()) {
		return;
	}

	XrCompositionLayerProjection *projection_layer = (XrCompositionLayerProjection *)get_openxr_api()->get_projection_layer();
	if (projection_layer == nullptr || projection_layer->space == XR_NULL_HANDLE) {
		return;
	}

	recommended_resolution_get_info.layer = (XrCompositionLayerBaseHeader *)projection_layer;
	recommended_resolution_get_info.predictedDisplayTime = get_openxr_api()->get_predicted_display_time();

	XrResult result = xrGetRecommendedLayerResolutionMETA(SESSION, &recommended_resolution_get_info, &recommended_resolution);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Failed to get recommended layer resolution, error code: ", result);
		return;
	}

	if (!recommended_resolution.isValid) {
		get_openxr_api()->set_render_region(Rect2i());
		return;
	}

	Size2i render_region_size = { recommended_resolution.recommendedImageDimensions.width, recommended_resolution.recommendedImageDimensions.height };
	get_openxr_api()->set_render_region(Rect2i(Point2i(0, 0), render_region_size));
}

void OpenXRMetaRecommendedLayerResolutionExtensionWrapper::_bind_methods() {
}

void OpenXRMetaRecommendedLayerResolutionExtensionWrapper::cleanup() {
	meta_recommended_layer_resolution_ext = false;
}

bool OpenXRMetaRecommendedLayerResolutionExtensionWrapper::initialize_meta_recommended_layer_resolution_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetRecommendedLayerResolutionMETA);

	return true;
}
