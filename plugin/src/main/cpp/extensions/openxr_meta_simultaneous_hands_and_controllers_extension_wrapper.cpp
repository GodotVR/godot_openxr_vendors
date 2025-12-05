/**************************************************************************/
/*  openxr_meta_simultaneous_hands_and_controllers_extension_wrapper.cpp  */
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

#include "extensions/openxr_meta_simultaneous_hands_and_controllers_extension_wrapper.h"
#include "openxr/openxr.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper *OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::singleton = nullptr;

OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper *OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper singleton already exists.");

	request_extensions[XR_META_SIMULTANEOUS_HANDS_AND_CONTROLLERS_EXTENSION_NAME] = &meta_simultaneous_hands_and_controllers_ext;
	singleton = this;
}

OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::~OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

uint64_t OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_simultaneous_hands_and_controllers_ext) {
		simultaneous_hands_and_controllers_properties.next = p_next_pointer;
		p_next_pointer = &simultaneous_hands_and_controllers_properties;
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (meta_simultaneous_hands_and_controllers_ext) {
		bool result = initialize_meta_simultaneous_hands_and_controllers_extension((XrInstance)p_instance);
		if (!result) {
			ERR_PRINT("Failed to initialize meta_simultaneous_hands_and_controllers extension");
			meta_simultaneous_hands_and_controllers_ext = false;
		}
	}
}

void OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::is_simultaneous_hands_and_controllers_supported() {
	return simultaneous_hands_and_controllers_properties.supportsSimultaneousHandsAndControllers;
}

void OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::resume_simultaneous_hands_and_controllers_tracking() {
	ERR_FAIL_COND_MSG(!meta_simultaneous_hands_and_controllers_ext, "XR_META_simultaneous_hands_and_controllers extension is not enabled");

	XrResult result = xrResumeSimultaneousHandsAndControllersTrackingMETA(SESSION, &simultaneous_hands_and_controllers_tracking_resume_info);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to resume simultaneous hands and controllers tracking [%s]", get_openxr_api()->get_error_string(result)));
}

void OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::pause_simultaneous_hands_and_controllers_tracking() {
	ERR_FAIL_COND_MSG(!meta_simultaneous_hands_and_controllers_ext, "XR_META_simultaneous_hands_and_controllers extension is not enabled");

	XrResult result = xrPauseSimultaneousHandsAndControllersTrackingMETA(SESSION, &simultaneous_hands_and_controllers_tracking_pause_info);
	ERR_FAIL_COND_MSG(XR_FAILED(result), vformat("Failed to pause simultaneous hands and controllers tracking [%s]", get_openxr_api()->get_error_string(result)));
}

void OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_simultaneous_hands_and_controllers_supported"), &OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::is_simultaneous_hands_and_controllers_supported);

	ClassDB::bind_method(D_METHOD("resume_simultaneous_hands_and_controllers_tracking"), &OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::resume_simultaneous_hands_and_controllers_tracking);
	ClassDB::bind_method(D_METHOD("pause_simultaneous_hands_and_controllers_tracking"), &OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::pause_simultaneous_hands_and_controllers_tracking);
}

void OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::cleanup() {
	meta_simultaneous_hands_and_controllers_ext = false;
}

bool OpenXRMetaSimultaneousHandsAndControllersExtensionWrapper::initialize_meta_simultaneous_hands_and_controllers_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrResumeSimultaneousHandsAndControllersTrackingMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrPauseSimultaneousHandsAndControllersTrackingMETA);

	return true;
}
