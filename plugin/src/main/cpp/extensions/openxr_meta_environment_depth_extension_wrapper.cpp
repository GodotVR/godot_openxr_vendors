/**************************************************************************/
/*  openxr_meta_environment_depth_extension_wrapper.cpp                   */
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
/* the following condiftions:                                             */
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

#include <vector>

#include "extensions/openxr_meta_environment_depth_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#ifdef ANDROID
#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_OPENGL_ES

// Various platform includes
#include <jni.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <openxr/openxr_platform.h>
#endif

using namespace godot;

OpenXRMetaEnvironmentDepthExtensionWrapper *OpenXRMetaEnvironmentDepthExtensionWrapper::singleton = nullptr;

OpenXRMetaEnvironmentDepthExtensionWrapper *OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaEnvironmentDepthExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaEnvironmentDepthExtensionWrapper::OpenXRMetaEnvironmentDepthExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaEnvironmentDepthExtensionWrapper singleton already exists.");

	request_extensions[XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME] = &meta_environment_depth_ext;
	singleton = this;
}

OpenXRMetaEnvironmentDepthExtensionWrapper::~OpenXRMetaEnvironmentDepthExtensionWrapper() {
	cleanup();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_environment_depth_supported"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_supported);
	ClassDB::bind_method(D_METHOD("initialize_depth"), &OpenXRMetaEnvironmentDepthExtensionWrapper::initialize_depth);
	ClassDB::bind_method(D_METHOD("get_current_depth_texture"), &OpenXRMetaEnvironmentDepthExtensionWrapper::get_current_depth_texture);
	ClassDB::bind_method(D_METHOD("get_near_z"), &OpenXRMetaEnvironmentDepthExtensionWrapper::get_near_z);
	ClassDB::bind_method(D_METHOD("get_far_z"), &OpenXRMetaEnvironmentDepthExtensionWrapper::get_far_z);
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::cleanup() {
	meta_environment_depth_ext = false;
}

Dictionary OpenXRMetaEnvironmentDepthExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (meta_environment_depth_ext) {
		bool result = initialize_meta_environment_depth_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize XR_META_environment_depth extension");
			meta_environment_depth_ext = false;
		}
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::initialize_meta_environment_depth_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStartEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateEnvironmentDepthSwapchainImagesMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetEnvironmentDepthSwapchainStateMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrAcquireEnvironmentDepthImageMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetEnvironmentDepthHandRemovalMETA);
	return true;
}

// Note: There is a bug in the Meta OpenXR runtime that causes xrCreateEnvironmentDepthSwapchainMETA
// to abort if called before rendering begins (only on the OpenGL side, the Vulkan version works fine)
// Until that is fixed, we should wait until the first frame is rendered before calling this method.
// Note: Requires com.oculus.permission.USE_SCENE - add a warning or error if it's not granted
bool OpenXRMetaEnvironmentDepthExtensionWrapper::initialize_depth() {
	if (!meta_environment_depth_ext) {
		WARN_PRINT("Environment depth unsupported");
		return false;
	}

	if (depth_initialized) {
		WARN_PRINT("Environment depth already initialized");
		return false;
	}

	// Create the environment depth provider.
	const XrEnvironmentDepthProviderCreateInfoMETA environmentDepthProviderCreateInfo {
			XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META, // type
			nullptr, // next
			0, // flags
	};
	XrResult result =	xrCreateEnvironmentDepthProviderMETA(
			SESSION, &environmentDepthProviderCreateInfo, &environment_depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create depth provider, error code: ", result);
	}

	// Create the depth swapchain.
	XrEnvironmentDepthSwapchainCreateInfoMETA environmentDepthSwapchainCreateInfo {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META, // type
		nullptr, // next
		0, // flags
	};
	result =	xrCreateEnvironmentDepthSwapchainMETA(
			environment_depth_provider,
			&environmentDepthSwapchainCreateInfo,
			&environment_depth_swapchain);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create depth swapchain, error code: ", result);
	}

	XrEnvironmentDepthSwapchainStateMETA environmentDepthSwapchainState {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_STATE_META,
    nullptr, // next
    0, // width
    0, // height
	};

	result = xrGetEnvironmentDepthSwapchainStateMETA(
		environment_depth_swapchain, &environmentDepthSwapchainState);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to get swapchain state, error code: ", result);
	}

	uint32_t environmentDepthSwapChainLength = 0;
	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(
			environment_depth_swapchain, 0, &environmentDepthSwapChainLength, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to enumerate swapchain images [1], error code: ", result);
	}

	environment_depth_textures.clear();
	environment_depth_textures.resize(environmentDepthSwapChainLength);

	// Populate the swapchain image array.
#ifdef ANDROID
	std::vector<XrSwapchainImageOpenGLESKHR> environmentDepthImages(environmentDepthSwapChainLength);
	for (uint32_t i = 0; i < environmentDepthSwapChainLength; ++i) {
			environmentDepthImages[i] = {
				XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR, // type
				nullptr, // next
				0 // texId
			};
	}

	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(
			environment_depth_swapchain,
			environmentDepthSwapChainLength,
			&environmentDepthSwapChainLength,
			(XrSwapchainImageBaseHeader*)environmentDepthImages.data());
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to enumerate swapchain images [2], error code: ", result);
	}

	for (uint32_t i = 0; i < environmentDepthSwapChainLength; ++i) {
		// TODO: The OpenGL texture that will contain depth is now stored in environmentDepthImages[i].image,
		// figure out how to make that available for depth occlusion in godot 3D renderer

		// Best guess based on XrPassthroughOcclusion sample...
		RID godotTextureId = RenderingServer::get_singleton()->texture_create_from_native_handle(
			RenderingServer::TextureType::TEXTURE_TYPE_LAYERED,
			Image::Format::FORMAT_RGB8, // This is really a GL_DEPTH_COMPONENT24
			environmentDepthImages[i].image,
			environmentDepthSwapchainState.width,
			environmentDepthSwapchainState.height,
			0,
			2,
			RenderingServer::TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY
		);
		UtilityFunctions::print("Mapping Swapchain ", i, " - GL_Tex ", environmentDepthImages[i].image, " - RID ", godotTextureId, " with size ", environmentDepthSwapchainState.width, "x", environmentDepthSwapchainState.height);
		environment_depth_textures[i] = godotTextureId;
	}
#endif

	result = xrStartEnvironmentDepthProviderMETA(environment_depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to start depth provider, error code: ", result);
	}

	depth_initialized = true;
	return true;
}

// Note: This appeared to be the right callback to run between xrBeginFrame and xrEndFrame (_process ran outside of that?)
void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_pre_draw_viewport(const RID &p_viewport) {
	if (!depth_initialized) {
		UtilityFunctions::print("Not initialized!");
		return;
	};

	XrEnvironmentDepthImageAcquireInfoMETA environmentDepthAcquireInfo{
			XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META};
	environmentDepthAcquireInfo.space = (XrSpace) get_openxr_api()->get_play_space();
	environmentDepthAcquireInfo.displayTime = get_openxr_api()->get_predicted_display_time();

	// Default init
	XrEnvironmentDepthImageMETA environmentDepthImage{XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META};
	environmentDepthImage.views[0].type = XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META;
	environmentDepthImage.views[1].type = XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META;

	const XrResult acquireResult = xrAcquireEnvironmentDepthImageMETA(
			environment_depth_provider, &environmentDepthAcquireInfo, &environmentDepthImage);

	// Cache the latest result
	if (acquireResult == XR_SUCCESS) {
		near_z = environmentDepthImage.nearZ;
		far_z = environmentDepthImage.farZ;
		current_depth_texture = environment_depth_textures[environmentDepthImage.swapchainIndex];
	} else {
		UtilityFunctions::print("Failed to get next depth! ", acquireResult);
	}
}
