/**************************************************************************/
/*  openxr_fb_space_warp_extension_wrapper.h                              */
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
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <map>

using namespace godot;

class OpenXRFbSpaceWarpExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpaceWarpExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRFbSpaceWarpExtensionWrapper *get_singleton();

	OpenXRFbSpaceWarpExtensionWrapper();
	virtual ~OpenXRFbSpaceWarpExtensionWrapper() override;

	godot::Dictionary _get_requested_extensions() override;

	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	uint64_t _set_projection_views_and_get_next_pointer(int p_view_index, void *p_next_pointer) override;

	void _on_instance_destroyed() override;
	void _on_session_created(uint64_t p_instance) override;
	void _on_session_destroyed() override;
	void _on_state_ready() override;
	void _on_main_swapchains_created() override;
	void _on_pre_render() override;
	void _on_post_draw_viewport(const RID &p_render_target) override;

	bool is_enabled();

	void set_space_warp_enabled(bool p_enable);

	void skip_space_warp_frame();

protected:
	static void _bind_methods();

private:
	void _skip_space_warp_frame();

	void cleanup();

	static OpenXRFbSpaceWarpExtensionWrapper *singleton;

	bool enabled = true;

	XrSystemSpaceWarpPropertiesFB system_space_warp_properties = {
		XR_TYPE_SYSTEM_SPACE_WARP_PROPERTIES_FB, // type
		nullptr, // next
		0, // recommendedMotionVectorImageRectWidth
		0, // recommendedMotionVectorImageRectHeight
	};

	uint64_t motion_vector_swapchain_info = 0;
	uint64_t motion_vector_depth_swapchain_info = 0;

	LocalVector<XrCompositionLayerSpaceWarpInfoFB> space_warp_info;

	std::map<godot::String, bool *> request_extensions;
	bool fb_space_warp_ext = false;

	struct RenderState {
		bool skip_space_warp_frame = false;
		Transform3D previous_transform = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0, 0.0 } };
	} render_state;
};
