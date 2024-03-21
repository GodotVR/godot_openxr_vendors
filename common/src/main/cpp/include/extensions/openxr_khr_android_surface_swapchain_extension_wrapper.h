/**************************************************************************/
/*  openxr_khr_android_surface_swapchain_extension_wrapper.h              */
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

#pragma once

#include <openxr/openxr.h>

struct QuadSurfaceLayer {
	int handle;
	int order;
	XrSwapchain swapchain;
	XrCompositionLayerQuad layer;
};

#ifdef ANDROID
#define XR_USE_PLATFORM_ANDROID
#include <jni.h>
#include <openxr/openxr_platform.h>

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

using namespace godot;

class OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_session_created(uint64_t p_session) override;
	void _on_session_destroyed() override;
	void _on_instance_destroyed() override;

	int _get_composition_layer_count() override;
	uint64_t _get_composition_layer(int p_index) override;
	int _get_composition_layer_order(int p_index) override;

	int allocate_swapchain(std::shared_ptr<QuadSurfaceLayer> layer, uint32_t widthPx, uint32_t heightPx);
	void free_swapchain(std::shared_ptr<QuadSurfaceLayer> layer);

	void start_drawing_layer(std::shared_ptr<QuadSurfaceLayer> layer);
	void stop_drawing_layer(std::shared_ptr<QuadSurfaceLayer> layer);

	bool is_android_surface_swapchain_supported() {
		return khr_android_surface_swapchain_ext && fb_android_surface_swapchain_create_ext;
	}

	static OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper *get_singleton();

	OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper();
	~OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC4(xrCreateSwapchainAndroidSurfaceKHR,
			(XrSession), session,
			(const XrSwapchainCreateInfo *), info,
			(XrSwapchain *), swapchain,
			(jobject *), surface);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroySwapchain,
			(XrSwapchain), swapchain);

	bool initialize_khr_android_surface_swapchain_extension(const XrInstance &instance);
	void cleanup();

	static OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper *singleton;

	HashMap<String, bool *> request_extensions;
	bool khr_android_surface_swapchain_ext = false;
	bool fb_android_surface_swapchain_create_ext = false;
	bool fb_composition_layer_image_layout_ext = false;

	std::vector<std::shared_ptr<QuadSurfaceLayer>> layers;
	XrCompositionLayerImageLayoutFB composition_layout_ext = {};
};

#endif // ANDROID
