/**************************************************************************/
/*  openxr_fb_space_warp_extension_wrapper.cpp                            */
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

#include "extensions/openxr_fb_space_warp_extension_wrapper.h"

#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define GL_RGBA16F 0x881A
#define GL_DEPTH24_STENCIL8 0x88F0

#define VK_FORMAT_R16G16B16A16_SFLOAT 97
#define VK_FORMAT_D24_UNORM_S8_UINT 129

OpenXRFbSpaceWarpExtensionWrapper *OpenXRFbSpaceWarpExtensionWrapper::singleton = nullptr;

OpenXRFbSpaceWarpExtensionWrapper *OpenXRFbSpaceWarpExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpaceWarpExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpaceWarpExtensionWrapper::OpenXRFbSpaceWarpExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpaceWarpExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPACE_WARP_EXTENSION_NAME] = &fb_space_warp_ext;
	singleton = this;
}

OpenXRFbSpaceWarpExtensionWrapper::~OpenXRFbSpaceWarpExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRFbSpaceWarpExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

uint64_t OpenXRFbSpaceWarpExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (fb_space_warp_ext) {
		system_space_warp_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_space_warp_properties);
	} else {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}
}

uint64_t OpenXRFbSpaceWarpExtensionWrapper::_set_projection_views_and_get_next_pointer(int p_view_index, void *p_next_pointer) {
	if (is_enabled()) {
		space_warp_info[p_view_index].next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&space_warp_info[p_view_index]);
	} else {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_session_created(uint64_t p_instance) {
	if (!fb_space_warp_ext) {
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	String rendering_method = (String)project_settings->get_setting_with_override("rendering/renderer/rendering_method");
	if (rendering_method == "forward_plus") {
		UtilityFunctions::print_verbose("Disabling XR_FB_space_warp extension; this extension is not supported in the forward plus renderer");
		cleanup();
		return;
	}

	get_openxr_api()->register_projection_views_extension(this);
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_session_destroyed() {
	if (!fb_space_warp_ext) {
		return;
	}

	get_openxr_api()->unregister_projection_views_extension(this);
	space_warp_info.clear();
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_state_ready() {
	if (!is_enabled()) {
		return;
	}

	Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	int view_count = openxr_interface->get_view_count();
	int width = system_space_warp_properties.recommendedMotionVectorImageRectWidth;
	int height = system_space_warp_properties.recommendedMotionVectorImageRectHeight;

	String rendering_driver_name = RenderingServer::get_singleton()->get_current_rendering_driver_name();
	int swapchain_format, depth_swapchain_format = 0;

	if (rendering_driver_name.contains("opengl")) {
		swapchain_format = GL_RGBA16F;
		depth_swapchain_format = GL_DEPTH24_STENCIL8;
	} else if (rendering_driver_name == "vulkan") {
		swapchain_format = VK_FORMAT_R16G16B16A16_SFLOAT;
		depth_swapchain_format = VK_FORMAT_D24_UNORM_S8_UINT;
	} else {
		UtilityFunctions::print_verbose("Disabling XR_FB_space_warp extension; rendering driver is not supported: ", rendering_driver_name);
		cleanup();
		return;
	}

	motion_vector_swapchain_info = get_openxr_api()->openxr_swapchain_create(0, XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT, swapchain_format, width, height, 1, view_count);
	motion_vector_depth_swapchain_info = get_openxr_api()->openxr_swapchain_create(0, XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_swapchain_format, width, height, 1, view_count);
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_main_swapchains_created() {
	if (!fb_space_warp_ext) {
		return;
	}

	Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	int view_count = openxr_interface->get_view_count();
	space_warp_info.resize(view_count);

	for (int i = 0; i < space_warp_info.size(); i++) {
		XrCompositionLayerSpaceWarpInfoFB &info = space_warp_info[i];

		info.type = XR_TYPE_COMPOSITION_LAYER_SPACE_WARP_INFO_FB;

		info.next = nullptr;

		info.layerFlags = 0;

		info.motionVectorSubImage.swapchain = (XrSwapchain)get_openxr_api()->openxr_swapchain_get_swapchain(motion_vector_swapchain_info);
		info.motionVectorSubImage.imageRect.offset.x = 0;
		info.motionVectorSubImage.imageRect.offset.y = 0;
		info.motionVectorSubImage.imageRect.extent.width = system_space_warp_properties.recommendedMotionVectorImageRectWidth;
		info.motionVectorSubImage.imageRect.extent.height = system_space_warp_properties.recommendedMotionVectorImageRectHeight;
		info.motionVectorSubImage.imageArrayIndex = i;

		info.appSpaceDeltaPose = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 0.0 } };

		info.depthSubImage.swapchain = (XrSwapchain)get_openxr_api()->openxr_swapchain_get_swapchain(motion_vector_depth_swapchain_info);
		info.depthSubImage.imageRect.offset.x = 0;
		info.depthSubImage.imageRect.offset.y = 0;
		info.depthSubImage.imageRect.extent.width = system_space_warp_properties.recommendedMotionVectorImageRectWidth;
		info.depthSubImage.imageRect.extent.height = system_space_warp_properties.recommendedMotionVectorImageRectHeight;
		info.depthSubImage.imageArrayIndex = i;

		info.minDepth = 0.0;
		info.maxDepth = 1.0;

		info.farZ = get_openxr_api()->get_render_state_z_near();
		info.nearZ = get_openxr_api()->get_render_state_z_far();
	}
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_pre_render() {
	if (!is_enabled()) {
		get_openxr_api()->set_velocity_texture(RID());
		get_openxr_api()->set_velocity_depth_texture(RID());
		return;
	}

	get_openxr_api()->openxr_swapchain_acquire(motion_vector_swapchain_info);
	get_openxr_api()->openxr_swapchain_acquire(motion_vector_depth_swapchain_info);

	RID motion_vector_swapchain_image = get_openxr_api()->openxr_swapchain_get_image(motion_vector_swapchain_info);
	get_openxr_api()->set_velocity_texture(motion_vector_swapchain_image);
	RID motion_vector_depth_swapchain_image = get_openxr_api()->openxr_swapchain_get_image(motion_vector_depth_swapchain_info);
	get_openxr_api()->set_velocity_depth_texture(motion_vector_depth_swapchain_image);

	int target_width = system_space_warp_properties.recommendedMotionVectorImageRectWidth;
	int target_height = system_space_warp_properties.recommendedMotionVectorImageRectHeight;
	Size2i render_target_size = { target_width, target_height };
	get_openxr_api()->set_velocity_target_size(render_target_size);

	Transform3D world_transform = XRServer::get_singleton()->get_world_origin();
	Transform3D delta_transform = render_state.previous_transform.affine_inverse() * world_transform;
	Quaternion delta_quat = delta_transform.basis.get_quaternion();
	Vector3 delta_origin = delta_transform.origin;

	Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	int view_count = openxr_interface->get_view_count();
	for (int i = 0; i < view_count; i++) {
		space_warp_info[i].layerFlags = render_state.skip_space_warp_frame ? XR_COMPOSITION_LAYER_SPACE_WARP_INFO_FRAME_SKIP_BIT_FB : 0;
		space_warp_info[i].appSpaceDeltaPose = { { delta_quat.x, delta_quat.y, delta_quat.z, delta_quat.w }, { delta_origin.x, delta_origin.y, delta_origin.z } };
		space_warp_info[i].farZ = get_openxr_api()->get_render_state_z_near();
		space_warp_info[i].nearZ = get_openxr_api()->get_render_state_z_far();
	}

	render_state.skip_space_warp_frame = false;
	render_state.previous_transform = world_transform;
}

void OpenXRFbSpaceWarpExtensionWrapper::_on_post_draw_viewport(const RID &p_render_target) {
	if (!is_enabled()) {
		return;
	}

	get_openxr_api()->openxr_swapchain_release(motion_vector_swapchain_info);
	get_openxr_api()->openxr_swapchain_release(motion_vector_depth_swapchain_info);
}

bool OpenXRFbSpaceWarpExtensionWrapper::is_enabled() {
	return fb_space_warp_ext && enabled;
}

void OpenXRFbSpaceWarpExtensionWrapper::set_space_warp_enabled(bool p_enable) {
	enabled = p_enable;
}

void OpenXRFbSpaceWarpExtensionWrapper::_skip_space_warp_frame() {
	render_state.skip_space_warp_frame = true;
}

void OpenXRFbSpaceWarpExtensionWrapper::skip_space_warp_frame() {
	if (!is_enabled()) {
		return;
	}

	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbSpaceWarpExtensionWrapper::_skip_space_warp_frame));
}

void OpenXRFbSpaceWarpExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_space_warp_enabled", "enable"), &OpenXRFbSpaceWarpExtensionWrapper::set_space_warp_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbSpaceWarpExtensionWrapper::is_enabled);
	ClassDB::bind_method(D_METHOD("skip_space_warp_frame"), &OpenXRFbSpaceWarpExtensionWrapper::skip_space_warp_frame);
}

void OpenXRFbSpaceWarpExtensionWrapper::cleanup() {
	if (fb_space_warp_ext) {
		get_openxr_api()->unregister_projection_views_extension(this);
	}

	fb_space_warp_ext = false;
	enabled = false;
}
