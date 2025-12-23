/**************************************************************************/
/*  openxr_fb_color_space_extension_wrapper.h                             */
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
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the meta color space extension.
class OpenXRFbColorSpaceExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbColorSpaceExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	enum ColorSpace {
		COLOR_SPACE_RUNTIME_DEFAULT,
		COLOR_SPACE_UNMANAGED,
		COLOR_SPACE_REC2020,
		COLOR_SPACE_REC709,
		COLOR_SPACE_RIFT_CV1,
		COLOR_SPACE_RIFT_S,
		COLOR_SPACE_QUEST,
		COLOR_SPACE_P3,
		COLOR_SPACE_ADOBE_RGB,
	};

	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;
	void _on_session_destroyed() override;
	void _on_state_ready() override;

	static OpenXRFbColorSpaceExtensionWrapper *get_singleton();

	bool is_enabled() { return fb_color_space_ext; }

	OpenXRFbColorSpaceExtensionWrapper();
	~OpenXRFbColorSpaceExtensionWrapper();

	Array get_supported_color_spaces();

	ColorSpace get_native_color_space();

	void set_color_space(ColorSpace p_color_space);
	ColorSpace get_color_space();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC4(xrEnumerateColorSpacesFB,
			(XrSession), session,
			(uint32_t), colorSpaceCapacityInput,
			(uint32_t *), colorSpaceCountOutput,
			(XrColorSpaceFB *), colorSpaces);

	EXT_PROTO_XRRESULT_FUNC2(xrSetColorSpaceFB,
			(XrSession), session,
			(const XrColorSpaceFB), colorSpace);

	bool initialize_fb_color_space_extension(const XrInstance instance);

	void cleanup();

	void populate_supported_color_spaces();

	ColorSpace from_openxr_color_space(XrColorSpaceFB p_color_space);
	XrColorSpaceFB to_openxr_color_space(ColorSpace p_color_space);

	static OpenXRFbColorSpaceExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;

	bool fb_color_space_ext = false;
	bool supported_color_spaces_set = false;

	XrSystemColorSpacePropertiesFB system_color_space_properties = {
		XR_TYPE_SYSTEM_COLOR_SPACE_PROPERTIES_FB, // type
		nullptr, // next
	};

	Vector<XrColorSpaceFB> supported_color_spaces;
	ColorSpace current_color_space = COLOR_SPACE_RUNTIME_DEFAULT;
};

VARIANT_ENUM_CAST(OpenXRFbColorSpaceExtensionWrapper::ColorSpace)
