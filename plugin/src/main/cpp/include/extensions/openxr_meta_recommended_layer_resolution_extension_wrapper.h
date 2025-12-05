/**************************************************************************/
/*  openxr_meta_recommended_layer_resolution_extension_wrapper.h          */
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

#ifndef OPENXR_META_RECOMMENDED_LAYER_RESOLUTION_EXTENSION_WRAPPER_H
#define OPENXR_META_RECOMMENDED_LAYER_RESOLUTION_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <map>

#include "util.h"

using namespace godot;

class OpenXRMetaRecommendedLayerResolutionExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMetaRecommendedLayerResolutionExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRMetaRecommendedLayerResolutionExtensionWrapper *get_singleton();

	OpenXRMetaRecommendedLayerResolutionExtensionWrapper();
	virtual ~OpenXRMetaRecommendedLayerResolutionExtensionWrapper() override;

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;
	void _on_pre_render() override;

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrGetRecommendedLayerResolutionMETA,
			(XrSession), session,
			(const XrRecommendedLayerResolutionGetInfoMETA *), info,
			(XrRecommendedLayerResolutionMETA *), resolution)

	bool initialize_meta_recommended_layer_resolution_extension(XrInstance p_instance);

	void cleanup();

	static OpenXRMetaRecommendedLayerResolutionExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool meta_recommended_layer_resolution_ext = false;

	XrRecommendedLayerResolutionGetInfoMETA recommended_resolution_get_info = {
		XR_TYPE_RECOMMENDED_LAYER_RESOLUTION_GET_INFO_META, // type
		nullptr, // next
		nullptr, // layer
		0, // predictedDisplayTime
	};

	XrRecommendedLayerResolutionMETA recommended_resolution = {
		XR_TYPE_RECOMMENDED_LAYER_RESOLUTION_META, // type
		nullptr, // next
		{ 0, 0 }, // recommendedImageDimensions,
		false, // isValid
	};
};

#endif // OPENXR_META_RECOMMENDED_LAYER_RESOLUTION_EXTENSION_WRAPPER_H
