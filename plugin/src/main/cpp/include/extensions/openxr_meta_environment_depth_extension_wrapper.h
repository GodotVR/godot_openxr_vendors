/**************************************************************************/
/*  openxr_meta_environment_depth_extension_wrapper.h                     */
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
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the XR_META_environment_depth extension.
class OpenXRMetaEnvironmentDepthExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMetaEnvironmentDepthExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;
	void _on_pre_draw_viewport(const RID &p_viewport) override;

	bool is_environment_depth_supported() {
		return meta_environment_depth_ext;
	}

	RID get_current_depth_texture() {
		return current_depth_texture;
	}

	float get_near_z() {
		return near_z;
	}

	float get_far_z() {
		return far_z;
	}

	static OpenXRMetaEnvironmentDepthExtensionWrapper *get_singleton();

	OpenXRMetaEnvironmentDepthExtensionWrapper();
	~OpenXRMetaEnvironmentDepthExtensionWrapper();

	// Call this to initialize everything. Returns true on success, false on failure
	bool initialize_depth();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateEnvironmentDepthProviderMETA,
			(XrSession), session,
			(const XrEnvironmentDepthProviderCreateInfoMETA *), createInfo,
			(XrEnvironmentDepthProviderMETA *), environmentDepthProvider);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyEnvironmentDepthProviderMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider);

	EXT_PROTO_XRRESULT_FUNC1(xrStartEnvironmentDepthProviderMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider);

	EXT_PROTO_XRRESULT_FUNC1(xrStopEnvironmentDepthProviderMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider);

	EXT_PROTO_XRRESULT_FUNC3(xrCreateEnvironmentDepthSwapchainMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider,
			(const XrEnvironmentDepthSwapchainCreateInfoMETA *), createInfo,
			(XrEnvironmentDepthSwapchainMETA *), swapchain);

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyEnvironmentDepthSwapchainMETA,
			(XrEnvironmentDepthSwapchainMETA), swapchain);

	EXT_PROTO_XRRESULT_FUNC4(xrEnumerateEnvironmentDepthSwapchainImagesMETA,
			(XrEnvironmentDepthSwapchainMETA), swapchain,
			(uint32_t), imageCapacityInput,
			(uint32_t *), imageCountOutput,
			(XrSwapchainImageBaseHeader *), images);

	EXT_PROTO_XRRESULT_FUNC2(xrGetEnvironmentDepthSwapchainStateMETA,
			(XrEnvironmentDepthSwapchainMETA), swapchain,
			(XrEnvironmentDepthSwapchainStateMETA *), state);

	EXT_PROTO_XRRESULT_FUNC3(xrAcquireEnvironmentDepthImageMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider,
			(const XrEnvironmentDepthImageAcquireInfoMETA*), acquireInfo,
			(XrEnvironmentDepthImageMETA *), environmentDepthImage);

	EXT_PROTO_XRRESULT_FUNC2(xrSetEnvironmentDepthHandRemovalMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider,
			(const XrEnvironmentDepthHandRemovalSetInfoMETA *), setInfo);

	bool initialize_meta_environment_depth_extension(const XrInstance &instance);

	HashMap<String, bool *> request_extensions;

	void cleanup();

	static OpenXRMetaEnvironmentDepthExtensionWrapper *singleton;

	bool meta_environment_depth_ext = false;
	bool depth_initialized = false;
	XrEnvironmentDepthProviderMETA environment_depth_provider;
	XrEnvironmentDepthSwapchainMETA environment_depth_swapchain;
	std::vector<RID> environment_depth_textures;

	// Latest data, to be polled every frame
	RID current_depth_texture;
	float near_z;
	float far_z;
};
