/**************************************************************************/
/*  openxr_meta_boundary_visibility_extension_wrapper.cpp                 */
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

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
#include "extensions/openxr_meta_boundary_visibility_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRMetaBoundaryVisibilityExtensionWrapper *OpenXRMetaBoundaryVisibilityExtensionWrapper::singleton = nullptr;

OpenXRMetaBoundaryVisibilityExtensionWrapper *OpenXRMetaBoundaryVisibilityExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaBoundaryVisibilityExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaBoundaryVisibilityExtensionWrapper::OpenXRMetaBoundaryVisibilityExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaBoundaryVisibilityExtensionWrapper singleton already exists.");

	request_extensions[XR_META_BOUNDARY_VISIBILITY_EXTENSION_NAME] = &meta_boundary_visibility_ext;
	singleton = this;
}

OpenXRMetaBoundaryVisibilityExtensionWrapper::~OpenXRMetaBoundaryVisibilityExtensionWrapper() {
	cleanup();
	singleton = nullptr;
}

godot::Dictionary OpenXRMetaBoundaryVisibilityExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

uint64_t OpenXRMetaBoundaryVisibilityExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_boundary_visibility_ext) {
		boundary_visibility_properties.next = p_next_pointer;
		p_next_pointer = &boundary_visibility_properties;
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRMetaBoundaryVisibilityExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	if (meta_boundary_visibility_ext) {
		bool result = initialize_meta_boundary_visibility_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize meta_boundary_visibility extension");
			meta_boundary_visibility_ext = false;
		}
	}
}

void OpenXRMetaBoundaryVisibilityExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRMetaBoundaryVisibilityExtensionWrapper::_on_event_polled(const void *p_event) {
	if (!meta_boundary_visibility_ext) {
		return false;
	}

	if (static_cast<const XrEventDataBuffer *>(p_event)->type == XR_TYPE_EVENT_DATA_BOUNDARY_VISIBILITY_CHANGED_META) {
		XrEventDataBoundaryVisibilityChangedMETA *boundary_visibility_event = (XrEventDataBoundaryVisibilityChangedMETA *)p_event;
		current_boundary_visibility = boundary_visibility_event->boundaryVisibility;
		emit_signal("openxr_meta_boundary_visibility_changed", current_boundary_visibility == XR_BOUNDARY_VISIBILITY_NOT_SUPPRESSED_META);

		return true;
	}

	return false;
}

bool OpenXRMetaBoundaryVisibilityExtensionWrapper::is_boundary_visibility_supported() {
	return boundary_visibility_properties.supportsBoundaryVisibility;
}

void OpenXRMetaBoundaryVisibilityExtensionWrapper::set_boundary_visible(bool p_visible) {
	if (!meta_boundary_visibility_ext) {
		return;
	}

	const XrBoundaryVisibilityMETA boundary_visibility = p_visible ? XR_BOUNDARY_VISIBILITY_NOT_SUPPRESSED_META : XR_BOUNDARY_VISIBILITY_SUPPRESSED_META;
	if (boundary_visibility == current_boundary_visibility) {
		return;
	}

	Ref<XRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	if (xr_interface.is_null() || xr_interface->get_environment_blend_mode() != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND) {
		UtilityFunctions::printerr("Boundary visibility can only be suppressed when passthrough is active");
		return;
	}

	XrResult result = xrRequestBoundaryVisibilityMETA(SESSION, boundary_visibility);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to request boundary visibility update: ", get_openxr_api()->get_error_string(result));
	}
}

bool OpenXRMetaBoundaryVisibilityExtensionWrapper::is_boundary_visible() {
	return (current_boundary_visibility == XR_BOUNDARY_VISIBILITY_NOT_SUPPRESSED_META);
}

void OpenXRMetaBoundaryVisibilityExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_boundary_visibility_supported"), &OpenXRMetaBoundaryVisibilityExtensionWrapper::is_boundary_visibility_supported);

	ClassDB::bind_method(D_METHOD("set_boundary_visible", "visible"), &OpenXRMetaBoundaryVisibilityExtensionWrapper::set_boundary_visible);
	ClassDB::bind_method(D_METHOD("is_boundary_visible"), &OpenXRMetaBoundaryVisibilityExtensionWrapper::is_boundary_visible);

	ADD_SIGNAL(MethodInfo("openxr_meta_boundary_visibility_changed", PropertyInfo(Variant::BOOL, "visible")));
}

void OpenXRMetaBoundaryVisibilityExtensionWrapper::cleanup() {
	meta_boundary_visibility_ext = false;
}

bool OpenXRMetaBoundaryVisibilityExtensionWrapper::initialize_meta_boundary_visibility_extension(XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrRequestBoundaryVisibilityMETA);

	return true;
}
#endif // META_HEADERS_ENABLED
