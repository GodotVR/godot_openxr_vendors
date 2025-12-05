/**************************************************************************/
/*  openxr_fb_android_surface_swapchain_create_extension_wrapper.cpp      */
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

#include "extensions/openxr_fb_android_surface_swapchain_create_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

static const char *SYNCHRONOUS_PROPERTY_NAME = "XR_FB_android_surface_swapchain_create/synchronous";
static const char *USE_TIMESTAMPS_PROPERTY_NAME = "XR_FB_android_surface_swapchain_create/use_timestamps";

OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper *OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::singleton = nullptr;

OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper *OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper());
	}
	return singleton;
}

OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper singleton already exists.");

#ifdef ANDROID_ENABLED
	request_extensions[XR_FB_ANDROID_SURFACE_SWAPCHAIN_CREATE_EXTENSION_NAME] = &fb_android_surface_swapchain_create_ext;
#endif
	singleton = this;
}

OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::~OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper() {
}

void OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::is_enabled);
}

Dictionary OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

TypedArray<Dictionary> OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::_get_viewport_composition_layer_extension_properties() {
	TypedArray<Dictionary> properties;

	{
		Dictionary synchronous;
		synchronous["name"] = SYNCHRONOUS_PROPERTY_NAME;
		synchronous["type"] = Variant::BOOL;
		synchronous["hint"] = PROPERTY_HINT_NONE;
		synchronous["hint_string"] = "";
		properties.push_back(synchronous);
	}

	{
		Dictionary use_timestamps;
		use_timestamps["name"] = USE_TIMESTAMPS_PROPERTY_NAME;
		use_timestamps["type"] = Variant::BOOL;
		use_timestamps["hint"] = PROPERTY_HINT_NONE;
		use_timestamps["hint_string"] = "";
		properties.push_back(use_timestamps);
	}

	return properties;
}

Dictionary OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::_get_viewport_composition_layer_extension_property_defaults() {
	Dictionary defaults;
	defaults[SYNCHRONOUS_PROPERTY_NAME] = false;
	defaults[USE_TIMESTAMPS_PROPERTY_NAME] = false;
	return defaults;
}

uint64_t OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::_set_android_surface_swapchain_create_info_and_get_next_pointer(const Dictionary &p_property_values, void *p_next_pointer) {
#ifdef ANDROID_ENABLED
	if (fb_android_surface_swapchain_create_ext) {
		create_info.next = p_next_pointer;
		create_info.createFlags = 0;

		if ((bool)p_property_values.get(SYNCHRONOUS_PROPERTY_NAME, false)) {
			create_info.createFlags |= XR_ANDROID_SURFACE_SWAPCHAIN_SYNCHRONOUS_BIT_FB;
		}
		if ((bool)p_property_values.get(USE_TIMESTAMPS_PROPERTY_NAME, false)) {
			create_info.createFlags |= XR_ANDROID_SURFACE_SWAPCHAIN_USE_TIMESTAMPS_BIT_FB;
		}

		if (create_info.createFlags != 0) {
			return reinterpret_cast<uint64_t>(&create_info);
		}
	}
#endif

	return reinterpret_cast<uint64_t>(p_next_pointer);
}
