/**************************************************************************/
/*  openxr_fb_composition_layer_settings_extension.h              */
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

#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

// Wrapper for XR_FB_composition_layer_settings extension.
class OpenXRFbCompositionLayerSettingsExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRFbCompositionLayerSettingsExtension, OpenXRExtensionWrapper);

public:
	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	static OpenXRFbCompositionLayerSettingsExtension *get_singleton();

	enum SupersamplingMode {
		SUPERSAMPLING_MODE_DISABLED,
		SUPERSAMPLING_MODE_NORMAL,
		SUPERSAMPLING_MODE_QUALITY,
	};

	enum SharpeningMode {
		SHARPENING_MODE_DISABLED,
		SHARPENING_MODE_NORMAL,
		SHARPENING_MODE_QUALITY,
	};

	virtual uint64_t _set_viewport_composition_layer_and_get_next_pointer(const void *p_layer, const Dictionary &p_property_values, void *p_next_pointer) override;
	virtual void _on_viewport_composition_layer_destroyed(const void *p_layer) override;
	virtual TypedArray<Dictionary> _get_viewport_composition_layer_extension_properties() override;
	virtual Dictionary _get_viewport_composition_layer_extension_property_defaults() override;
	virtual uint64_t _set_projection_layer_and_get_next_pointer(void *p_next_pointer) override;

	virtual void _on_session_created(uint64_t p_session) override;
	virtual void _on_session_destroyed() override;
	virtual void _on_state_ready() override;

	bool is_enabled() const;

	void set_projection_layer_auto_filter_enabled(bool p_enabled);
	bool is_projection_layer_auto_filter_enabled() const;

	void set_projection_layer_supersampling_mode(SupersamplingMode p_supersampling_mode);
	SupersamplingMode get_projection_layer_supersampling_mode() const;

	void set_projection_layer_sharpening_mode(SharpeningMode p_sharpening_mode);
	SharpeningMode get_projection_layer_sharpening_mode() const;

	OpenXRFbCompositionLayerSettingsExtension();
	~OpenXRFbCompositionLayerSettingsExtension();

protected:
	static void _bind_methods();

private:
	void cleanup();
	XrCompositionLayerSettingsFlagsFB _from_sharpening_mode(SharpeningMode p_sharpening_mode) const;
	XrCompositionLayerSettingsFlagsFB _from_supersampling_mode(SupersamplingMode p_supersampling_mode) const;

	static OpenXRFbCompositionLayerSettingsExtension *singleton;

	HashMap<String, bool *> request_extensions;

	bool fb_composition_layer_settings = false;
	bool meta_automatic_layer_filter = false;

	HashMap<const XrCompositionLayerBaseHeader *, XrCompositionLayerSettingsFB> layer_structs;

	XrCompositionLayerSettingsFB projection_layer_settings = {
		XR_TYPE_COMPOSITION_LAYER_SETTINGS_FB, // type
		nullptr, // next
		0, // layerFlags
	};

	bool projection_layer_auto_filter_enabled = false;
	SupersamplingMode projection_layer_supersampling_mode = SUPERSAMPLING_MODE_DISABLED;
	SharpeningMode projection_layer_sharpening_mode = SHARPENING_MODE_DISABLED;
};

VARIANT_ENUM_CAST(OpenXRFbCompositionLayerSettingsExtension::SupersamplingMode);
VARIANT_ENUM_CAST(OpenXRFbCompositionLayerSettingsExtension::SharpeningMode);
