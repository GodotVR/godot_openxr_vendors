/**************************************************************************/
/*  openxr_fb_composition_layer_image_layout_extension_wrapper.h          */
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

#ifndef OPENXR_FB_COMPOSITION_LAYER_IMAGE_LAYOUT_EXTENSION_WRAPPER_H
#define OPENXR_FB_COMPOSITION_LAYER_IMAGE_LAYOUT_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

// Wrapper for XR_FB_composition_layer_image_layout extension.
class OpenXRFbCompositionLayerImageLayoutExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbCompositionLayerImageLayoutExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	static OpenXRFbCompositionLayerImageLayoutExtensionWrapper *get_singleton();

	virtual uint64_t _set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) override;
	virtual void _on_viewport_composition_layer_destroyed(const void *p_layer) override;
	virtual TypedArray<Dictionary> _get_viewport_composition_layer_extension_properties() override;
	virtual Dictionary _get_viewport_composition_layer_extension_property_defaults() override;

	bool is_enabled() const;

	OpenXRFbCompositionLayerImageLayoutExtensionWrapper();
	~OpenXRFbCompositionLayerImageLayoutExtensionWrapper();

protected:
	static void _bind_methods();

private:
	void cleanup();

	static OpenXRFbCompositionLayerImageLayoutExtensionWrapper *singleton;

	HashMap<String, bool *> request_extensions;

	bool fb_composition_layer_image_layout = false;

	HashMap<const XrCompositionLayerBaseHeader *, XrCompositionLayerImageLayoutFB> layer_structs;
};

#endif // OPENXR_FB_COMPOSITION_LAYER_IMAGE_LAYOUT_EXTENSION_WRAPPER_H
