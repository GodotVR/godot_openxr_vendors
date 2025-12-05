/**************************************************************************/
/*  openxr_meta_colocation_discovery.cpp                                 */
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

#include "extensions/openxr_meta_colocation_discovery_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRMetaColocationDiscoveryExtensionWrapper *OpenXRMetaColocationDiscoveryExtensionWrapper::singleton = nullptr;

OpenXRMetaColocationDiscoveryExtensionWrapper *OpenXRMetaColocationDiscoveryExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaColocationDiscoveryExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaColocationDiscoveryExtensionWrapper::OpenXRMetaColocationDiscoveryExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaColocationDiscoveryExtensionWrapper singleton already exists.");

	request_extensions[XR_META_COLOCATION_DISCOVERY_EXTENSION_NAME] = &meta_colocation_discovery_ext;
	singleton = this;
}

OpenXRMetaColocationDiscoveryExtensionWrapper::~OpenXRMetaColocationDiscoveryExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRMetaColocationDiscoveryExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (!meta_colocation_discovery_ext) {
		return;
	}

	if (!initialize_extension((XrInstance)instance)) {
		meta_colocation_discovery_ext = false;
	}
}

uint64_t OpenXRMetaColocationDiscoveryExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_colocation_discovery_ext) {
		colocation_discovery_properties.next = p_next_pointer;
		p_next_pointer = &colocation_discovery_properties;
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::initialize_extension(const XrInstance instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrStartColocationAdvertisementMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopColocationAdvertisementMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStartColocationDiscoveryMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopColocationDiscoveryMETA);

	return true;
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::_on_event_polled(const void *event) {
	if (!meta_colocation_discovery_ext) {
		return false;
	}

	const XrEventDataBuffer *event_buffer = (const XrEventDataBuffer *)event;

	switch (event_buffer->type) {
		case XR_TYPE_EVENT_DATA_START_COLOCATION_ADVERTISEMENT_COMPLETE_META: {
			on_start_advertisement_complete((const XrEventDataStartColocationAdvertisementCompleteMETA *)event);
			return true;
		}
		case XR_TYPE_EVENT_DATA_STOP_COLOCATION_ADVERTISEMENT_COMPLETE_META: {
			on_stop_advertisement_complete((const XrEventDataStopColocationAdvertisementCompleteMETA *)event);
			return true;
		}
		case XR_TYPE_EVENT_DATA_COLOCATION_ADVERTISEMENT_COMPLETE_META: {
			on_advertisement_complete((const XrEventDataColocationAdvertisementCompleteMETA *)event);
			return true;
		}
		case XR_TYPE_EVENT_DATA_START_COLOCATION_DISCOVERY_COMPLETE_META: {
			on_start_discovery_complete((const XrEventDataStartColocationDiscoveryCompleteMETA *)event);
			return true;
		}
		case XR_TYPE_EVENT_DATA_COLOCATION_DISCOVERY_RESULT_META: {
			on_discovery_result((const XrEventDataColocationDiscoveryResultMETA *)event);
			return true;
		}
		case XR_TYPE_EVENT_DATA_COLOCATION_DISCOVERY_COMPLETE_META: {
			on_discovery_complete((const XrEventDataColocationDiscoveryCompleteMETA *)event);
			return true;
		}
		case XR_TYPE_EVENT_DATA_STOP_COLOCATION_DISCOVERY_COMPLETE_META: {
			on_stop_discovery_complete((const XrEventDataStopColocationDiscoveryCompleteMETA *)event);
			return true;
		}
		default:
			return false;
	}
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::is_colocation_discovery_supported() const {
	return meta_colocation_discovery_ext && colocation_discovery_properties.supportsColocationDiscovery;
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::start_advertisement(const PackedByteArray &p_buffer) {
	ERR_FAIL_COND_V_MSG(!meta_colocation_discovery_ext, false, "XR_META_colocation_discovery extension is not enabled");
	ERR_FAIL_COND_V_MSG(p_buffer.size() > XR_MAX_COLOCATION_DISCOVERY_BUFFER_SIZE_META, false,
			vformat("Buffer size %d exceeds maximum size of %d bytes", p_buffer.size(), XR_MAX_COLOCATION_DISCOVERY_BUFFER_SIZE_META));

	if (is_advertising) {
		UtilityFunctions::push_warning("Already advertising, stop current advertisement first");
		return false;
	}

	XrColocationAdvertisementStartInfoMETA start_info = {
		XR_TYPE_COLOCATION_ADVERTISEMENT_START_INFO_META, // type
		nullptr, // next
		(uint32_t)p_buffer.size(), // bufferSize
		(uint8_t *)p_buffer.ptr() // buffer
	};

	XrAsyncRequestIdFB request_id;
	XrResult result = xrStartColocationAdvertisementMETA(SESSION, &start_info, &request_id);

	if (XR_SUCCEEDED(result)) {
		return true;
	} else {
		UtilityFunctions::push_error(vformat("Failed to start colocation advertisement: %d", result));
		return false;
	}
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::stop_advertisement() {
	ERR_FAIL_COND_V_MSG(!meta_colocation_discovery_ext, false, "XR_META_colocation_discovery extension is not enabled");

	if (!is_advertising) {
		UtilityFunctions::push_warning("Not currently advertising");
		return false;
	}

	XrColocationAdvertisementStopInfoMETA stop_info = {
		XR_TYPE_COLOCATION_ADVERTISEMENT_STOP_INFO_META, // type
		nullptr // next
	};

	XrAsyncRequestIdFB request_id;
	XrResult result = xrStopColocationAdvertisementMETA(SESSION, &stop_info, &request_id);

	if (XR_SUCCEEDED(result)) {
		return true;
	} else {
		UtilityFunctions::push_error(vformat("Failed to stop colocation advertisement: %d", result));
		return false;
	}
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::start_discovery() {
	ERR_FAIL_COND_V_MSG(!meta_colocation_discovery_ext, false, "XR_META_colocation_discovery extension is not enabled");

	if (is_discovering) {
		UtilityFunctions::push_warning("Already discovering, stop current discovery first");
		return false;
	}

	XrColocationDiscoveryStartInfoMETA start_info = {
		XR_TYPE_COLOCATION_DISCOVERY_START_INFO_META, // type
		nullptr // next
	};

	XrAsyncRequestIdFB request_id;
	XrResult result = xrStartColocationDiscoveryMETA(SESSION, &start_info, &request_id);

	if (XR_SUCCEEDED(result)) {
		return true;
	} else {
		UtilityFunctions::push_error(vformat("Failed to start colocation discovery: %d", result));
		return false;
	}
}

bool OpenXRMetaColocationDiscoveryExtensionWrapper::stop_discovery() {
	ERR_FAIL_COND_V_MSG(!meta_colocation_discovery_ext, false, "XR_META_colocation_discovery extension is not enabled");

	if (!is_discovering) {
		UtilityFunctions::push_warning("Not currently discovering");
		return false;
	}

	XrColocationDiscoveryStopInfoMETA stop_info = {
		XR_TYPE_COLOCATION_DISCOVERY_STOP_INFO_META, // type
		nullptr // next
	};

	XrAsyncRequestIdFB request_id;
	XrResult result = xrStopColocationDiscoveryMETA(SESSION, &stop_info, &request_id);

	if (XR_SUCCEEDED(result)) {
		return true;
	} else {
		UtilityFunctions::push_error(vformat("Failed to stop colocation discovery: %d", result));
		return false;
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_start_advertisement_complete(const XrEventDataStartColocationAdvertisementCompleteMETA *event) {
	if (XR_SUCCEEDED(event->result)) {
		is_advertising = true;
		String uuid_str = OpenXRUtilities::uuid_to_string_name(event->advertisementUuid);
		emit_signal("openxr_meta_colocation_discovery_advertisement_started", uuid_str);
	} else {
		UtilityFunctions::push_error(vformat("Failed to start advertisement: %d", event->result));
		emit_signal("openxr_meta_colocation_discovery_advertisement_failed", (int)event->result);
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_stop_advertisement_complete(const XrEventDataStopColocationAdvertisementCompleteMETA *event) {
	if (XR_SUCCEEDED(event->result)) {
		is_advertising = false;
		emit_signal("openxr_meta_colocation_discovery_advertisement_stopped");
	} else {
		UtilityFunctions::push_error(vformat("Failed to stop advertisement: %d", event->result));
		emit_signal("openxr_meta_colocation_discovery_advertisement_failed", (int)event->result);
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_advertisement_complete(const XrEventDataColocationAdvertisementCompleteMETA *event) {
	is_advertising = false;
	if (XR_SUCCEEDED(event->result)) {
		emit_signal("openxr_meta_colocation_discovery_advertisement_complete");
	} else {
		UtilityFunctions::push_error(vformat("Advertisement ended with error: %d", event->result));
		emit_signal("openxr_meta_colocation_discovery_advertisement_failed", (int)event->result);
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_start_discovery_complete(const XrEventDataStartColocationDiscoveryCompleteMETA *event) {
	if (XR_SUCCEEDED(event->result)) {
		is_discovering = true;
		emit_signal("openxr_meta_colocation_discovery_discovery_started");
	} else {
		UtilityFunctions::push_error(vformat("Failed to start discovery: %d", event->result));
		emit_signal("openxr_meta_colocation_discovery_discovery_failed", (int)event->result);
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_discovery_result(const XrEventDataColocationDiscoveryResultMETA *event) {
	String uuid_str = OpenXRUtilities::uuid_to_string_name(event->advertisementUuid);

	PackedByteArray buffer;
	buffer.resize(event->bufferSize);
	memcpy(buffer.ptrw(), event->buffer, event->bufferSize);

	emit_signal("openxr_meta_colocation_discovery_discovery_result", uuid_str, buffer);
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_discovery_complete(const XrEventDataColocationDiscoveryCompleteMETA *event) {
	is_discovering = false;
	if (XR_SUCCEEDED(event->result)) {
		emit_signal("openxr_meta_colocation_discovery_discovery_complete");
	} else {
		UtilityFunctions::push_error(vformat("Discovery ended with error: %d", event->result));
		emit_signal("openxr_meta_colocation_discovery_discovery_failed", (int)event->result);
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::on_stop_discovery_complete(const XrEventDataStopColocationDiscoveryCompleteMETA *event) {
	if (XR_SUCCEEDED(event->result)) {
		is_discovering = false;
		emit_signal("openxr_meta_colocation_discovery_discovery_stopped");
	} else {
		UtilityFunctions::push_error(vformat("Failed to stop discovery: %d", event->result));
		emit_signal("openxr_meta_colocation_discovery_discovery_failed", (int)event->result);
	}
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_colocation_discovery_supported"), &OpenXRMetaColocationDiscoveryExtensionWrapper::is_colocation_discovery_supported);
	ClassDB::bind_method(D_METHOD("start_advertisement", "buffer"), &OpenXRMetaColocationDiscoveryExtensionWrapper::start_advertisement);
	ClassDB::bind_method(D_METHOD("stop_advertisement"), &OpenXRMetaColocationDiscoveryExtensionWrapper::stop_advertisement);
	ClassDB::bind_method(D_METHOD("start_discovery"), &OpenXRMetaColocationDiscoveryExtensionWrapper::start_discovery);
	ClassDB::bind_method(D_METHOD("stop_discovery"), &OpenXRMetaColocationDiscoveryExtensionWrapper::stop_discovery);

	// Signals
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_advertisement_started", PropertyInfo(Variant::STRING, "advertisement_uuid")));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_advertisement_stopped"));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_advertisement_complete"));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_advertisement_failed", PropertyInfo(Variant::INT, "error_code")));

	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_discovery_started"));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_discovery_result",
			PropertyInfo(Variant::STRING, "advertisement_uuid"),
			PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data")));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_discovery_complete"));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_discovery_stopped"));
	ADD_SIGNAL(MethodInfo("openxr_meta_colocation_discovery_discovery_failed", PropertyInfo(Variant::INT, "error_code")));
}

void OpenXRMetaColocationDiscoveryExtensionWrapper::cleanup() {
	meta_colocation_discovery_ext = false;
	is_advertising = false;
	is_discovering = false;
}
