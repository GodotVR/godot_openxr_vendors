/**************************************************************************/
/*  openxr_fb_color_space_extension_wrapper.cpp                           */
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

#include "extensions/openxr_fb_color_space_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

OpenXRFbColorSpaceExtensionWrapper *OpenXRFbColorSpaceExtensionWrapper::singleton = nullptr;

OpenXRFbColorSpaceExtensionWrapper *OpenXRFbColorSpaceExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbColorSpaceExtensionWrapper());
	}
	return singleton;
}

OpenXRFbColorSpaceExtensionWrapper::OpenXRFbColorSpaceExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbColorSpaceExtensionWrapper singleton already exists");

	request_extensions[XR_FB_COLOR_SPACE_EXTENSION_NAME] = &fb_color_space_ext;
	singleton = this;
}

OpenXRFbColorSpaceExtensionWrapper::~OpenXRFbColorSpaceExtensionWrapper() {
	cleanup();
}

Array OpenXRFbColorSpaceExtensionWrapper::get_supported_color_spaces() {
	ERR_FAIL_COND_V_MSG(!fb_color_space_ext, Array(), "XR_FB_color_space extension is not enabled");

	if (!supported_color_spaces_set) {
		populate_supported_color_spaces();
	}

	ERR_FAIL_COND_V(supported_color_spaces.size() == 0, Array());

	Array ret;
	ret.resize(supported_color_spaces.size());
	int i = 0;
	for (const XrColorSpaceFB &color_space : supported_color_spaces) {
		ret[i++] = from_openxr_color_space(color_space);
	}
	return ret;
}

OpenXRFbColorSpaceExtensionWrapper::ColorSpace OpenXRFbColorSpaceExtensionWrapper::get_native_color_space() {
	ERR_FAIL_COND_V_MSG(!fb_color_space_ext, COLOR_SPACE_RUNTIME_DEFAULT, "XR_FB_color_space extension is not enabled");
	return from_openxr_color_space(system_color_space_properties.colorSpace);
}

void OpenXRFbColorSpaceExtensionWrapper::set_color_space(ColorSpace p_color_space) {
	ERR_FAIL_COND_MSG(!fb_color_space_ext, "XR_FB_color_space extension is not enabled");
	ERR_FAIL_COND_MSG(p_color_space == COLOR_SPACE_RUNTIME_DEFAULT, "Color space must be set to an explicit option when being set at runtime");

	if (p_color_space == current_color_space) {
		return;
	}

	if (!supported_color_spaces_set) {
		populate_supported_color_spaces();
	}

	XrColorSpaceFB openxr_color_space = to_openxr_color_space(p_color_space);
	ERR_FAIL_COND_MSG(!supported_color_spaces.has(openxr_color_space), vformat("Color space is not supported: %s", p_color_space));

	XrResult result = xrSetColorSpaceFB(SESSION, openxr_color_space);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to set color space [%s]", get_openxr_api()->get_error_string(result)));

	current_color_space = p_color_space;
}

OpenXRFbColorSpaceExtensionWrapper::ColorSpace OpenXRFbColorSpaceExtensionWrapper::get_color_space() {
	ERR_FAIL_COND_V_MSG(!fb_color_space_ext, COLOR_SPACE_RUNTIME_DEFAULT, "XR_FB_color_space extension is not enabled");
	return current_color_space;
}

void OpenXRFbColorSpaceExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbColorSpaceExtensionWrapper::is_enabled);

	ClassDB::bind_method(D_METHOD("get_supported_color_spaces"), &OpenXRFbColorSpaceExtensionWrapper::get_supported_color_spaces);

	ClassDB::bind_method(D_METHOD("get_native_color_space"), &OpenXRFbColorSpaceExtensionWrapper::get_native_color_space);

	ClassDB::bind_method(D_METHOD("set_color_space", "color_space"), &OpenXRFbColorSpaceExtensionWrapper::set_color_space);
	ClassDB::bind_method(D_METHOD("get_color_space"), &OpenXRFbColorSpaceExtensionWrapper::get_color_space);

	BIND_ENUM_CONSTANT(COLOR_SPACE_RUNTIME_DEFAULT)
	BIND_ENUM_CONSTANT(COLOR_SPACE_UNMANAGED)
	BIND_ENUM_CONSTANT(COLOR_SPACE_REC2020)
	BIND_ENUM_CONSTANT(COLOR_SPACE_REC709)
	BIND_ENUM_CONSTANT(COLOR_SPACE_RIFT_CV1)
	BIND_ENUM_CONSTANT(COLOR_SPACE_RIFT_S)
	BIND_ENUM_CONSTANT(COLOR_SPACE_QUEST)
	BIND_ENUM_CONSTANT(COLOR_SPACE_P3)
	BIND_ENUM_CONSTANT(COLOR_SPACE_ADOBE_RGB)
}

void OpenXRFbColorSpaceExtensionWrapper::cleanup() {
	fb_color_space_ext = false;
	supported_color_spaces_set = false;
}

void OpenXRFbColorSpaceExtensionWrapper::populate_supported_color_spaces() {
	if (supported_color_spaces_set) {
		return;
	}

	uint32_t num_color_spaces = 0;
	XrResult result = xrEnumerateColorSpacesFB(SESSION, 0, &num_color_spaces, nullptr);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to get supported color space count [%s]", get_openxr_api()->get_error_string(result)));

	supported_color_spaces.resize(num_color_spaces);
	result = xrEnumerateColorSpacesFB(SESSION, num_color_spaces, &num_color_spaces, supported_color_spaces.ptrw());
	if (XR_FAILED(result)) {
		supported_color_spaces.clear();
		ERR_FAIL_MSG(vformat("Failed to get supported color spaces [%s]", get_openxr_api()->get_error_string(result)));
	}

	supported_color_spaces_set = true;
}

OpenXRFbColorSpaceExtensionWrapper::ColorSpace OpenXRFbColorSpaceExtensionWrapper::from_openxr_color_space(XrColorSpaceFB p_color_space) {
	switch (p_color_space) {
		case XR_COLOR_SPACE_UNMANAGED_FB: {
			return COLOR_SPACE_UNMANAGED;
		} break;
		case XR_COLOR_SPACE_REC2020_FB: {
			return COLOR_SPACE_REC2020;
		} break;
		case XR_COLOR_SPACE_REC709_FB: {
			return COLOR_SPACE_REC709;
		} break;
		case XR_COLOR_SPACE_RIFT_CV1_FB: {
			return COLOR_SPACE_RIFT_CV1;
		} break;
		case XR_COLOR_SPACE_RIFT_S_FB: {
			return COLOR_SPACE_RIFT_S;
		} break;
		case XR_COLOR_SPACE_QUEST_FB: {
			return COLOR_SPACE_QUEST;
		} break;
		case XR_COLOR_SPACE_P3_FB: {
			return COLOR_SPACE_P3;
		} break;
		case XR_COLOR_SPACE_ADOBE_RGB_FB: {
			return COLOR_SPACE_ADOBE_RGB;
		} break;
		case XR_COLOR_SPACE_MAX_ENUM_FB:
		default: {
			ERR_FAIL_V_MSG(COLOR_SPACE_RUNTIME_DEFAULT, vformat("Unknown OpenXR color space: %s", p_color_space));
		}
	}
}

XrColorSpaceFB OpenXRFbColorSpaceExtensionWrapper::to_openxr_color_space(ColorSpace p_color_space) {
	switch (p_color_space) {
		case COLOR_SPACE_UNMANAGED: {
			return XR_COLOR_SPACE_UNMANAGED_FB;
		} break;
		case COLOR_SPACE_REC2020: {
			return XR_COLOR_SPACE_REC2020_FB;
		} break;
		case COLOR_SPACE_REC709: {
			return XR_COLOR_SPACE_REC709_FB;
		} break;
		case COLOR_SPACE_RIFT_CV1: {
			return XR_COLOR_SPACE_RIFT_CV1_FB;
		} break;
		case COLOR_SPACE_RIFT_S: {
			return XR_COLOR_SPACE_RIFT_S_FB;
		} break;
		case COLOR_SPACE_QUEST: {
			return XR_COLOR_SPACE_QUEST_FB;
		} break;
		case COLOR_SPACE_P3: {
			return XR_COLOR_SPACE_P3_FB;
		} break;
		case COLOR_SPACE_ADOBE_RGB: {
			return XR_COLOR_SPACE_ADOBE_RGB_FB;
		} break;
		case COLOR_SPACE_RUNTIME_DEFAULT:
		default: {
			ERR_FAIL_V_MSG(XR_COLOR_SPACE_MAX_ENUM_FB, vformat("Unknown color space: %s", p_color_space));
		}
	}
}

uint64_t OpenXRFbColorSpaceExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (fb_color_space_ext) {
		system_color_space_properties.next = p_next_pointer;
		p_next_pointer = &system_color_space_properties;
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

godot::Dictionary OpenXRFbColorSpaceExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbColorSpaceExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_color_space_ext) {
		bool result = initialize_fb_color_space_extension((XrInstance)instance);
		if (!result) {
			ERR_PRINT("Failed to initialize XR_FB_color_space extension");
			fb_color_space_ext = false;
		}
	}
}

void OpenXRFbColorSpaceExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbColorSpaceExtensionWrapper::_on_session_destroyed() {
	current_color_space = COLOR_SPACE_RUNTIME_DEFAULT;
}

void OpenXRFbColorSpaceExtensionWrapper::_on_state_ready() {
	if (!fb_color_space_ext) {
		return;
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	ColorSpace starting_color_space = ColorSpace((int)project_settings->get_setting_with_override("xr/openxr/extensions/meta/color_space/starting_color_space"));
	if (starting_color_space == COLOR_SPACE_RUNTIME_DEFAULT) {
		WARN_PRINT("Recommended color space project setting is REC709");
	} else {
		set_color_space(starting_color_space);
	}
}

bool OpenXRFbColorSpaceExtensionWrapper::initialize_fb_color_space_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateColorSpacesFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetColorSpaceFB);

	return true;
}
