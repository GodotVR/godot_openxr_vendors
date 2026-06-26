/**************************************************************************/
/*  openxr_android_enumerate_system_extension_properties_extension.cpp    */
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

#include "extensions/openxr_android_enumerate_system_extension_properties_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidEnumerateSystemExtensionPropertiesExtension *OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::singleton = nullptr;

OpenXRAndroidEnumerateSystemExtensionPropertiesExtension *OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidEnumerateSystemExtensionPropertiesExtension());
	}
	return singleton;
}

OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::OpenXRAndroidEnumerateSystemExtensionPropertiesExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidEnumerateSystemExtensionPropertiesExtension singleton already exists.");

	request_extensions[XR_ANDROID_ENUMERATE_SYSTEM_EXTENSION_PROPERTIES_EXTENSION_NAME] = &android_enumerate_system_extension_properties_ext;
	singleton = this;
}

OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::~OpenXRAndroidEnumerateSystemExtensionPropertiesExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::is_enabled);
	ClassDB::bind_method(D_METHOD("get_supported_extensions"), &OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::get_supported_extensions);

	ADD_SIGNAL(MethodInfo("openxr_android_supported_extensions_changed"));
}

void OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_on_instance_created(uint64_t p_instance) {
	if (android_enumerate_system_extension_properties_ext) {
		bool result = initialize_android_enumerate_system_extension_properties_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize XR_ANDROID_enumerate_system_extension_properties extension");
			android_enumerate_system_extension_properties_ext = false;
		}
	}
}

void OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_on_session_created(uint64_t p_session) {
	_update_supported_extensions();
}

bool OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_on_event_polled(const void *p_event) {
	if (!android_enumerate_system_extension_properties_ext) {
		return false;
	}

	if (static_cast<const XrEventDataBuffer *>(p_event)->type == XR_TYPE_EVENT_DATA_SYSTEM_PROPERTIES_CHANGED_ANDROID) {
		_update_supported_extensions();
		return true;
	}

	return false;
}

bool OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::initialize_android_enumerate_system_extension_properties_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSystemExtensionPropertiesANDROID);

	return true;
}

void OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::cleanup() {
	android_enumerate_system_extension_properties_ext = false;
}

Dictionary OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

bool OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::is_enabled() const {
	return android_enumerate_system_extension_properties_ext;
}

PackedStringArray OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::get_supported_extensions() const {
	return supported_extensions;
}

void OpenXRAndroidEnumerateSystemExtensionPropertiesExtension::_update_supported_extensions() {
	if (!android_enumerate_system_extension_properties_ext) {
		return;
	}

	Ref<OpenXRAPIExtension> openxr_api = get_openxr_api();
	ERR_FAIL_NULL(openxr_api);

	XrInstance xr_instance = (XrInstance)openxr_api->get_instance();
	XrSystemId xr_system_id = (XrSystemId)openxr_api->get_system_id();
	ERR_FAIL_COND(xr_system_id == XR_NULL_SYSTEM_ID);

	uint32_t extension_count = 0;
	XrResult result = xrEnumerateSystemExtensionPropertiesANDROID(xr_instance, xr_system_id, 0, &extension_count, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrEnumerateSystemExtensionPropertiesANDROID failed to get extension count: %s", openxr_api->get_error_string(result)));
		return;
	}

	LocalVector<XrSystemExtensionPropertiesANDROID> extension_properties;
	extension_properties.resize(extension_count);
	for (XrSystemExtensionPropertiesANDROID &ext_prop : extension_properties) {
		ext_prop.type = XR_TYPE_SYSTEM_EXTENSION_PROPERTIES_ANDROID;
		ext_prop.next = nullptr;
		ext_prop.properties = {
			XR_TYPE_EXTENSION_PROPERTIES, // type
			nullptr, // next
			"", // extensionName
			0, // extensionVersion
		};
		ext_prop.isSupported = XR_FALSE;
	}

	result = xrEnumerateSystemExtensionPropertiesANDROID(xr_instance, xr_system_id, extension_count, &extension_count, extension_properties.ptr());
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrEnumerateSystemExtensionPropertiesANDROID failed: %s", openxr_api->get_error_string(result)));
		return;
	}

	PackedStringArray new_supported_extensions;
	for (const XrSystemExtensionPropertiesANDROID &ext_prop : extension_properties) {
		if (ext_prop.isSupported) {
			String ext_name = String::utf8(ext_prop.properties.extensionName);
			if (!ext_name.is_empty()) {
				new_supported_extensions.push_back(ext_name);
			}
		}
	}
	supported_extensions = new_supported_extensions;

	emit_signal("openxr_android_supported_extensions_changed");
}
