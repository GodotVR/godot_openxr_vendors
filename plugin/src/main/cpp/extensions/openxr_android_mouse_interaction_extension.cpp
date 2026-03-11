/**************************************************************************/
/*  openxr_android_mouse_interaction_extension.cpp                        */
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

#include "extensions/openxr_android_mouse_interaction_extension.h"

#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidMouseInteractionExtension *OpenXRAndroidMouseInteractionExtension::singleton = nullptr;

OpenXRAndroidMouseInteractionExtension *OpenXRAndroidMouseInteractionExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidMouseInteractionExtension());
	}
	return singleton;
}

OpenXRAndroidMouseInteractionExtension::OpenXRAndroidMouseInteractionExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidMouseInteractionExtension singleton already exists.");

	request_extensions[XR_ANDROID_MOUSE_INTERACTION_EXTENSION_NAME] = &android_mouse_interaction_ext;
	singleton = this;
}

OpenXRAndroidMouseInteractionExtension::~OpenXRAndroidMouseInteractionExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRAndroidMouseInteractionExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRAndroidMouseInteractionExtension::is_enabled);
}

bool OpenXRAndroidMouseInteractionExtension::is_enabled() const {
	return android_mouse_interaction_ext;
}

PackedStringArray OpenXRAndroidMouseInteractionExtension::_get_suggested_tracker_names() {
	PackedStringArray arr = {
		"/user/mouse",
	};
	return arr;
}

void OpenXRAndroidMouseInteractionExtension::_on_register_metadata(OpenXRInteractionProfileMetadata *p_interaction_profile_metadata) {
	// Mouse interaction profile.
	const String profile_path = "/interaction_profiles/android/mouse_interaction_android";
	const String user_path = "/user/mouse";

	p_interaction_profile_metadata->register_top_level_path("Android mouse", user_path, XR_ANDROID_MOUSE_INTERACTION_EXTENSION_NAME);
	p_interaction_profile_metadata->register_interaction_profile("Android mouse", profile_path, XR_ANDROID_MOUSE_INTERACTION_EXTENSION_NAME);

	p_interaction_profile_metadata->register_io_path(profile_path, "Aim pose", user_path, user_path + String("/input/aim/pose"), "", OpenXRAction::OPENXR_ACTION_POSE);

	p_interaction_profile_metadata->register_io_path(profile_path, "Select click", user_path, user_path + String("/input/select/click"), "", OpenXRAction::OPENXR_ACTION_BOOL);
	p_interaction_profile_metadata->register_io_path(profile_path, "Secondary click", user_path, user_path + String("/input/secondary_android/click"), "", OpenXRAction::OPENXR_ACTION_BOOL);
	p_interaction_profile_metadata->register_io_path(profile_path, "Tertiary click", user_path, user_path + String("/input/tertiary_android/click"), "", OpenXRAction::OPENXR_ACTION_BOOL);

	p_interaction_profile_metadata->register_io_path(profile_path, "Scroll", user_path, user_path + String("/input/scroll_android"), "", OpenXRAction::OPENXR_ACTION_VECTOR2);
}

void OpenXRAndroidMouseInteractionExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRAndroidMouseInteractionExtension::cleanup() {
	android_mouse_interaction_ext = false;
}

Dictionary OpenXRAndroidMouseInteractionExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}
