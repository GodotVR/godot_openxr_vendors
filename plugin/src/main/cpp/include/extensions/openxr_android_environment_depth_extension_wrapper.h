/**************************************************************************/
/*  openxr_android_environment_depth_extension_wrapper.h                  */
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

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/local_vector.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the AndroidXR environment depth extension.
class OpenXRAndroidEnvironmentDepthExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRAndroidEnvironmentDepthExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	enum DepthCameraResolution {
		DEPTH_CAMERA_RESOLUTION_80x80,
		DEPTH_CAMERA_RESOLUTION_160x160,
		DEPTH_CAMERA_RESOLUTION_320x320,
	};

	Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	virtual void _on_instance_created(uint64_t instance) override;
	virtual void _on_instance_destroyed() override;

	virtual void _on_session_created(uint64_t p_instance) override;
	virtual void _on_session_destroyed() override;

	virtual void _on_pre_render() override;

	virtual uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;

	bool is_environment_depth_supported() {
		return android_environment_depth_ext && system_depth_properties.supportsDepthTracking;
	}

	bool start_environment_depth();
	void stop_environment_depth();
	bool is_environment_depth_started();

	Array get_supported_resolutions() const;
	bool set_resolution(DepthCameraResolution p_resolution);

	bool set_smooth(bool p_smooth);

	RID get_reprojection_mesh();

	void set_reprojection_render_priority(int p_render_priority);
	int get_reprojection_render_priority() const;

	void set_reprojection_offset_scale(float p_offset_scale);
	float get_reprojection_offset_scale() const;

	void set_reprojection_offset_exponent(float p_offset_exponent);
	float get_reprojection_offset_exponent() const;

	void setup_global_uniforms();

	static OpenXRAndroidEnvironmentDepthExtensionWrapper *get_singleton();

	OpenXRAndroidEnvironmentDepthExtensionWrapper();
	~OpenXRAndroidEnvironmentDepthExtensionWrapper();

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyDepthSwapchainANDROID, (XrDepthSwapchainANDROID), swapchain);

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC4(xrEnumerateDepthResolutionsANDROID, (XrSession), session, (uint32_t), resolutionCapacityInput, (uint32_t *), resolutionCountOutput, (XrDepthCameraResolutionANDROID *), resolutions);
	EXT_PROTO_XRRESULT_FUNC3(xrCreateDepthSwapchainANDROID, (XrSession), session, (const XrDepthSwapchainCreateInfoANDROID *), createInfo, (XrDepthSwapchainANDROID *), swapchain);
	EXT_PROTO_XRRESULT_FUNC4(xrEnumerateDepthSwapchainImagesANDROID, (XrDepthSwapchainANDROID), depthSwapchain, (uint32_t), depthImageCapacityInput, (uint32_t *), depthImageCountOutput, (XrDepthSwapchainImageANDROID *), depthImages);
	EXT_PROTO_XRRESULT_FUNC3(xrAcquireDepthSwapchainImagesANDROID, (XrDepthSwapchainANDROID), depthSwapchain, (const XrDepthAcquireInfoANDROID *), acquireInfo, (XrDepthAcquireResultANDROID *), acquireResult);

	bool initialize_android_environment_depth_extension(const XrInstance &instance);
	void cleanup();

	struct DepthCameraData {
		void reset();

		XrDepthSwapchainANDROID swapchain = XR_NULL_HANDLE;
		LocalVector<XrDepthSwapchainImageANDROID> xr_images;

		// Cache to avoid create/destroy every frame
		struct Cache {
			RID rid;
			Ref<Image> images[2];
		};
		LocalVector<Cache> godot_texture_cache;
	};
	bool _ensure_depth_swapchain_is_created(DepthCameraResolution p_resolution, bool p_smooth);
	static void _free_swapchain(XrDepthSwapchainANDROID p_swapchain);

	static OpenXRAndroidEnvironmentDepthExtensionWrapper *singleton;

	HashMap<String, bool *> request_extensions;
	bool android_environment_depth_ext = false;
	bool already_setup_global_uniforms = false;
	DepthCameraData depth_camera_data;
	HashSet<DepthCameraResolution> supported_resolutions;
	DepthCameraResolution resolution = DEPTH_CAMERA_RESOLUTION_320x320;
	bool smooth = false;

	// Image data is copied from XR to Ref<Image> twice every frame
	// This enables us to re-use the same buffer of the same size, instead of create/destroy every
	// frame
	PackedByteArray image_data_cache;

	XrSystemDepthTrackingPropertiesANDROID system_depth_properties = {
		XR_TYPE_SYSTEM_DEPTH_TRACKING_PROPERTIES_ANDROID, // type
		nullptr, // next
		XR_FALSE, // supportsDepthTracking
	};

	bool depth_provider_started = false;

	Ref<Shader> reprojection_shader;
	Ref<ShaderMaterial> reprojection_material;
	Ref<ArrayMesh> reprojection_mesh;
	int reprojection_render_priority = -50;
	float reprojection_offset_scale = 0.005;
	float reprojection_offset_exponent = 1.0;
	bool reprojection_material_dirty = false;

	void update_reprojection_material(bool p_creation = false);

	void destroy_depth_provider();
	void _update_mesh();
};

VARIANT_ENUM_CAST(OpenXRAndroidEnvironmentDepthExtensionWrapper::DepthCameraResolution);
