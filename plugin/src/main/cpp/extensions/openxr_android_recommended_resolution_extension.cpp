/**************************************************************************/
/*  openxr_android_recommended_resolution_extension.cpp                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "extensions/openxr_android_recommended_resolution_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidRecommendedResolutionExtension *OpenXRAndroidRecommendedResolutionExtension::singleton = nullptr;

OpenXRAndroidRecommendedResolutionExtension *OpenXRAndroidRecommendedResolutionExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidRecommendedResolutionExtension());
	}
	return singleton;
}

OpenXRAndroidRecommendedResolutionExtension::OpenXRAndroidRecommendedResolutionExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidRecommendedResolutionExtension singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_RECOMMENDED_RESOLUTION_EXTENSION_NAME] = &available;
}

OpenXRAndroidRecommendedResolutionExtension::~OpenXRAndroidRecommendedResolutionExtension() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidRecommendedResolutionExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

bool OpenXRAndroidRecommendedResolutionExtension::_on_event_polled(const void *p_event) {
	if (static_cast<const XrEventDataBuffer *>(p_event)->type == XR_TYPE_EVENT_DATA_RECOMMENDED_RESOLUTION_CHANGED_ANDROID) {
		get_openxr_api()->update_main_swapchain_size();
		return true;
	}
	return false;
}

void OpenXRAndroidRecommendedResolutionExtension::_bind_methods() {
}
