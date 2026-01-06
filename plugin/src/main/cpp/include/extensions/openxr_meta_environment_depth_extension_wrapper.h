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
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/local_vector.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the Meta environment depth extension.
class OpenXRMetaEnvironmentDepthExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMetaEnvironmentDepthExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_instance_destroyed() override;

	virtual void _on_session_destroyed() override;

	virtual void _on_pre_render() override;

	virtual uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;

	bool is_environment_depth_supported() {
		return meta_environment_depth_ext && system_depth_properties.supportsEnvironmentDepth && get_graphics_api() != GRAPHICS_API_UNSUPPORTED;
	}

	bool is_hand_removal_supported() {
		return meta_environment_depth_ext && system_depth_properties.supportsHandRemoval && get_graphics_api() != GRAPHICS_API_UNSUPPORTED;
	}

	void start_environment_depth();
	void stop_environment_depth();
	bool is_environment_depth_started();

	void set_hand_removal_enabled(bool p_enable);
	bool get_hand_removal_enabled() const;

	RID get_reprojection_mesh();

	void set_reprojection_render_priority(int p_render_priority);
	int get_reprojection_render_priority() const;

	void set_reprojection_offset_scale(float p_offset_scale);
	float get_reprojection_offset_scale() const;

	void set_reprojection_offset_exponent(float p_offset_exponent);
	float get_reprojection_offset_exponent() const;

	void set_reprojection_bilinear_filtering(bool p_enabled);
	bool get_reprojection_bilinear_filtering() const;

	void get_environment_depth_map_async(const Callable &p_callback);

	void setup_global_uniforms();

	static OpenXRMetaEnvironmentDepthExtensionWrapper *get_singleton();

	OpenXRMetaEnvironmentDepthExtensionWrapper();
	~OpenXRMetaEnvironmentDepthExtensionWrapper();

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
			(const XrEnvironmentDepthImageAcquireInfoMETA *), acquireInfo,
			(XrEnvironmentDepthImageMETA *), environmentDepthImage);

	EXT_PROTO_XRRESULT_FUNC2(xrSetEnvironmentDepthHandRemovalMETA,
			(XrEnvironmentDepthProviderMETA), environmentDepthProvider,
			(const XrEnvironmentDepthHandRemovalSetInfoMETA *), setInfo);

	bool initialize_meta_environment_depth_extension(const XrInstance &instance);
	void cleanup();

	static OpenXRMetaEnvironmentDepthExtensionWrapper *singleton;

	HashMap<String, bool *> request_extensions;
	bool meta_environment_depth_ext = false;
	bool already_setup_global_uniforms = false;

	XrSystemEnvironmentDepthPropertiesMETA system_depth_properties = {
		XR_TYPE_SYSTEM_ENVIRONMENT_DEPTH_PROPERTIES_META, // type
		nullptr, // next
		XR_FALSE, // supportsEnvironmentDepth
		XR_FALSE, // supportsHandRemoval
	};

	enum GraphicsAPI {
		GRAPHICS_API_UNKNOWN,
		GRAPHICS_API_OPENGL,
		GRAPHICS_API_VULKAN,
		GRAPHICS_API_UNSUPPORTED,
	};

	struct {
		XrEnvironmentDepthProviderMETA depth_provider = XR_NULL_HANDLE;
		XrEnvironmentDepthSwapchainMETA depth_swapchain = XR_NULL_HANDLE;
		bool depth_provider_started = false;
		GraphicsAPI graphics_api = GRAPHICS_API_UNKNOWN;
		Vector2 depth_swapchain_texel_size;
		LocalVector<RID> depth_swapchain_textures;
		LocalVector<Callable> depth_map_callbacks;
	} render_state;

	bool depth_provider_started = false;
	bool hand_removal_enabled = false;

	Ref<Shader> reprojection_shader;
	Ref<ShaderMaterial> reprojection_material;
	Ref<ArrayMesh> reprojection_mesh;
	int reprojection_render_priority = -50;
	float reprojection_offset_scale = 0.005;
	float reprojection_offset_exponent = 1.0;
	bool reprojection_bilinear_filtering = true;
	bool reprojection_material_dirty = false;

	GraphicsAPI get_graphics_api();

	void update_reprojection_material(bool p_creation = false);

	void _start_environment_depth_rt();
	void _stop_environment_depth_rt();
	void _set_hand_removal_enabled_rt(bool p_enable);
	void _add_depth_map_callback_rt(const Callable &p_callback);

	bool _create_depth_provider_rt();
	void _destroy_depth_provider_rt();

	void reset_state();
};
