/**************************************************************************/
/*  openxr_meta_headset_id_extension_wrapper.cpp                          */
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

#include "extensions/openxr_meta_headset_id_extension_wrapper.h"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "openxr/openxr.h"
#include "util.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

OpenXRMetaHeadsetIDExtensionWrapper *OpenXRMetaHeadsetIDExtensionWrapper::singleton = nullptr;

OpenXRMetaHeadsetIDExtensionWrapper *OpenXRMetaHeadsetIDExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaHeadsetIDExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaHeadsetIDExtensionWrapper::OpenXRMetaHeadsetIDExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaHeadsetIDExtensionWrapper singleton already exists.");

	request_extensions[XR_META_HEADSET_ID_EXTENSION_NAME] = &meta_headset_id_ext;
	singleton = this;
}

OpenXRMetaHeadsetIDExtensionWrapper::~OpenXRMetaHeadsetIDExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRMetaHeadsetIDExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

uint64_t OpenXRMetaHeadsetIDExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_headset_id_ext) {
		headset_id_properties.next = p_next_pointer;
		p_next_pointer = &headset_id_properties;
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRMetaHeadsetIDExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

String OpenXRMetaHeadsetIDExtensionWrapper::get_headset_id() {
	ERR_FAIL_COND_V_MSG(!meta_headset_id_ext, "", "XR_META_headset_id extension is not enabled");

	if (headset_id_string.is_empty()) {
		headset_id_string = OpenXRUtilities::uuid_to_string_name(headset_id_properties.id);
	}

	return headset_id_string;
}

void OpenXRMetaHeadsetIDExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_headset_id"), &OpenXRMetaHeadsetIDExtensionWrapper::get_headset_id);
}

void OpenXRMetaHeadsetIDExtensionWrapper::cleanup() {
	meta_headset_id_ext = false;
}
