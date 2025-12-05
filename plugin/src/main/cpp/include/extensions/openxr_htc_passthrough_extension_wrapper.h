/**************************************************************************/
/*  openxr_htc_passthrough_extension_wrapper.h                            */
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

#ifndef OPENXR_HTC_PASSTHROUGH_EXTENSION_WRAPPER_H
#define OPENXR_HTC_PASSTHROUGH_EXTENSION_WRAPPER_H

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openxr/openxr.h>

#include "util.h"

#include <map>

using namespace godot;

// Wrapper for the HTC XR passthrough extension.
class OpenXRHtcPassthroughExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRHtcPassthroughExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRHtcPassthroughExtensionWrapper *get_singleton();

	OpenXRHtcPassthroughExtensionWrapper();
	~OpenXRHtcPassthroughExtensionWrapper();

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;
	void _on_session_created(uint64_t p_session) override;
	void _on_session_destroyed() override;

	bool is_passthrough_supported() {
		return htc_passthrough_ext;
	}

	bool is_passthrough_started() {
		return htc_passthrough_ext && (passthrough_handle != XR_NULL_HANDLE) && (get_blend_mode() == XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND);
	}

	int _get_composition_layer_count() override;
	uint64_t _get_composition_layer(int p_index) override;
	int _get_composition_layer_order(int p_index) override;

protected:
	static void _bind_methods();

private:
	// Create a passthrough feature
	EXT_PROTO_XRRESULT_FUNC3(xrCreatePassthroughHTC,
			(XrSession), session,
			(const XrPassthroughCreateInfoHTC *), create_info,
			(XrPassthroughHTC *), feature_out)

	// Destroy a previously created passthrough feature
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyPassthroughHTC, (XrPassthroughHTC), feature)

	static OpenXRHtcPassthroughExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;

	bool htc_passthrough_ext = false;

	XrPassthroughHTC passthrough_handle = XR_NULL_HANDLE;

	XrCompositionLayerPassthroughHTC composition_passthrough_layer = {
		XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_HTC, // XrStructureType
		nullptr, // next
		XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT, //  XrCompositionLayerFlags
		XR_NULL_HANDLE, // XrSpace
		XR_NULL_HANDLE, // XrPassthroughHTC
		{
				// XrPassthroughColorHTC
				XR_TYPE_PASSTHROUGH_COLOR_HTC, // XrStructureType
				nullptr, // next
				1.0f // alpha (preset to opaque)
		}
	};

	bool initialize_htc_passthrough_extension(const XrInstance p_instance);
	void cleanup();

	XRInterface::EnvironmentBlendMode get_blend_mode() {
		Ref<XRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
		if (xr_interface.is_valid()) {
			return xr_interface->get_environment_blend_mode();
		}
		return XRInterface::XR_ENV_BLEND_MODE_OPAQUE;
	}
};

#endif // OPENXR_HTC_PASSTHROUGH_EXTENSION_WRAPPER_H
