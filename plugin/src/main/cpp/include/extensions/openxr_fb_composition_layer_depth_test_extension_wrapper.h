/**************************************************************************/
/*  openxr_fb_composition_layer_depth_test_extension_wrapper.h            */
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

using namespace godot;

// Wrapper for the XR_FB_composition_layer_depth_test extension. This asks the compositor to
// perform a depth test on the submitted layers to provide occlusion among them
class OpenXRFbCompositionLayerDepthTestExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbCompositionLayerDepthTestExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual uint64_t _set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) override;
	virtual TypedArray<Dictionary> _get_viewport_composition_layer_extension_properties() override;
	virtual Dictionary _get_viewport_composition_layer_extension_property_defaults() override;
	virtual void _on_viewport_composition_layer_destroyed(const void *p_layer) override;

	bool is_composition_layer_depth_test_supported() {
		return fb_composition_layer_depth_test_ext;
	}

	static OpenXRFbCompositionLayerDepthTestExtensionWrapper *get_singleton();

	OpenXRFbCompositionLayerDepthTestExtensionWrapper();
	~OpenXRFbCompositionLayerDepthTestExtensionWrapper();

protected:
	static void _bind_methods();

private:
	HashMap<String, bool *> request_extensions;
	HashMap<const XrCompositionLayerBaseHeader *, XrCompositionLayerDepthTestFB> layer_structs;

	void cleanup();

	static OpenXRFbCompositionLayerDepthTestExtensionWrapper *singleton;

	bool fb_composition_layer_depth_test_ext = false;
};
