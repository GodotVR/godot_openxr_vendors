/**************************************************************************/
/*  openxr_meta_environment_raycast_extension.cpp                        */
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

#include "extensions/openxr_meta_environment_raycast_extension.h"

#include <godot_cpp/classes/open_xr_future_extension.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRMetaEnvironmentRaycastExtension *OpenXRMetaEnvironmentRaycastExtension::singleton = nullptr;

OpenXRMetaEnvironmentRaycastExtension *OpenXRMetaEnvironmentRaycastExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRMetaEnvironmentRaycastExtension());
	}
	return singleton;
}

OpenXRMetaEnvironmentRaycastExtension::OpenXRMetaEnvironmentRaycastExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaEnvironmentRaycastExtension singleton already exists.");
	singleton = this;

	request_extensions[XR_META_ENVIRONMENT_RAYCAST_EXTENSION_NAME] = &meta_environment_raycast_ext;
}

OpenXRMetaEnvironmentRaycastExtension::~OpenXRMetaEnvironmentRaycastExtension() {
	singleton = nullptr;
}

Dictionary OpenXRMetaEnvironmentRaycastExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

uint64_t OpenXRMetaEnvironmentRaycastExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_environment_raycast_ext) {
		system_raycast_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_raycast_properties);
	}
	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRMetaEnvironmentRaycastExtension::_on_instance_created(uint64_t p_instance) {
	if (meta_environment_raycast_ext && !initialize_meta_environment_raycast_extension((XrInstance)p_instance)) {
		UtilityFunctions::printerr("OpenXR: Failed to initialize Meta environment raycast extension function pointers.");
		meta_environment_raycast_ext = false;
	}
}

void OpenXRMetaEnvironmentRaycastExtension::_on_instance_destroyed() {
	destroy_raycaster();
	meta_environment_raycast_ext = false;
	system_raycast_properties.supportsEnvironmentRaycast = XR_FALSE;
}

void OpenXRMetaEnvironmentRaycastExtension::_on_session_created(uint64_t p_session_instance) {
	if (is_environment_raycast_supported()) {
		create_raycaster();
	}
}

void OpenXRMetaEnvironmentRaycastExtension::_on_session_destroyed() {
	destroy_raycaster();
}

bool OpenXRMetaEnvironmentRaycastExtension::is_environment_raycast_supported() const {
	OpenXRFutureExtension *future_api = OpenXRFutureExtension::get_singleton();
	return meta_environment_raycast_ext && system_raycast_properties.supportsEnvironmentRaycast && future_api && future_api->is_active();
}

bool OpenXRMetaEnvironmentRaycastExtension::is_raycaster_ready() const {
	return environment_raycaster != XR_NULL_HANDLE;
}

bool OpenXRMetaEnvironmentRaycastExtension::create_raycaster() {
	if (environment_raycaster != XR_NULL_HANDLE || raycaster_future.is_valid()) {
		return true;
	}

	if (!is_environment_raycast_supported()) {
		return false;
	}

	XrEnvironmentRaycasterCreateInfoMETA create_info = {
		XR_TYPE_ENVIRONMENT_RAYCASTER_CREATE_INFO_META, // type
		nullptr, // next
	};

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult result = xrCreateEnvironmentRaycasterAsyncMETA(SESSION, &create_info, &future);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("OpenXR: Failed to start environment raycaster creation: ", get_openxr_api()->get_error_string(result));
		emit_signal("raycaster_error", (int)result);
		return false;
	}

	raycaster_future = OpenXRFutureExtension::get_singleton()->register_future(reinterpret_cast<uint64_t>(future), callable_mp(this, &OpenXRMetaEnvironmentRaycastExtension::_on_raycaster_created));
	return true;
}

void OpenXRMetaEnvironmentRaycastExtension::_on_raycaster_created(const Ref<OpenXRFutureResult> &p_future) {
	raycaster_future.unref();

	XrEnvironmentRaycasterCreateCompletionMETA completion = {
		XR_TYPE_ENVIRONMENT_RAYCASTER_CREATE_COMPLETION_META, // type
		nullptr, // next
		XR_SUCCESS, // futureResult
		XR_NULL_HANDLE, // environmentRaycaster
	};

	XrResult result = xrCreateEnvironmentRaycasterCompleteMETA(SESSION, (XrFutureEXT)p_future->get_future(), &completion);
	if (XR_SUCCEEDED(result) && XR_SUCCEEDED(completion.futureResult) && completion.environmentRaycaster != XR_NULL_HANDLE) {
		environment_raycaster = completion.environmentRaycaster;
		emit_signal("raycaster_ready");
		return;
	}

	XrResult err = XR_FAILED(result) ? result : (XR_FAILED(completion.futureResult) ? completion.futureResult : XR_ERROR_RUNTIME_FAILURE);
	UtilityFunctions::printerr("OpenXR: Failed to complete environment raycaster creation: ", get_openxr_api()->get_error_string(err));
	emit_signal("raycaster_error", (int)err);
}

void OpenXRMetaEnvironmentRaycastExtension::destroy_raycaster() {
	if (environment_raycaster != XR_NULL_HANDLE) {
		xrDestroyEnvironmentRaycasterMETA(environment_raycaster);
		environment_raycaster = XR_NULL_HANDLE;
	}

	if (raycaster_future.is_valid()) {
		// Godot cancels running futures itself when the session is destroyed.
		if (raycaster_future->get_status() == OpenXRFutureResult::RESULT_RUNNING) {
			OpenXRFutureExtension::get_singleton()->cancel_future(raycaster_future->get_future());
		}
		raycaster_future.unref();
	}
}

Dictionary OpenXRMetaEnvironmentRaycastExtension::raycast(const Vector3 &p_origin, const Vector3 &p_direction, float p_max_distance) {
	Dictionary ret;
	ret["status"] = (int)XR_ENVIRONMENT_RAYCAST_HIT_STATUS_NO_HIT_META;
	ret["transform"] = Transform3D();

	if (environment_raycaster == XR_NULL_HANDLE || p_direction.is_zero_approx()) {
		return ret;
	}

	Vector3 dir = p_direction.normalized();
	XrVector3f origin = { (float)p_origin.x, (float)p_origin.y, (float)p_origin.z };
	XrVector3f direction = { (float)dir.x, (float)dir.y, (float)dir.z };

	XrEnvironmentRaycastFilterDistanceMETA distance_filter = {
		XR_TYPE_ENVIRONMENT_RAYCAST_FILTER_DISTANCE_META,
		nullptr,
		p_max_distance,
	};
	const XrEnvironmentRaycastFilterBaseHeaderMETA *filters[1] = {
		reinterpret_cast<const XrEnvironmentRaycastFilterBaseHeaderMETA *>(&distance_filter),
	};

	// Ray and hit pose are in the play space, like in every other extension of this plugin (see the manual).
	XrEnvironmentRaycastHitGetInfoMETA get_info = {
		XR_TYPE_ENVIRONMENT_RAYCAST_HIT_GET_INFO_META, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
		origin, // origin
		direction, // direction
		p_max_distance > 0.0f ? 1u : 0u, // filterCount
		p_max_distance > 0.0f ? filters : nullptr, // filters
	};

	XrEnvironmentRaycastHitMETA hit = {
		XR_TYPE_ENVIRONMENT_RAYCAST_HIT_META,
		nullptr,
		XR_ENVIRONMENT_RAYCAST_HIT_STATUS_NO_HIT_META,
		{},
	};

	XrResult result = xrPerformEnvironmentRaycastMETA(environment_raycaster, &get_info, &hit);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("OpenXR: xrPerformEnvironmentRaycastMETA failed: ", get_openxr_api()->get_error_string(result));
		return ret;
	}

	ret["status"] = (int)hit.status;
	if (hit.status == XR_ENVIRONMENT_RAYCAST_HIT_STATUS_HIT_META) {
		// The Z axis of the hit pose is the surface normal (OpenXR specification).
		ret["transform"] = OpenXRUtilities::xrPosef_to_godot_transform3d(hit.pose);
	}
	return ret;
}

bool OpenXRMetaEnvironmentRaycastExtension::initialize_meta_environment_raycast_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentRaycasterAsyncMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentRaycasterCompleteMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentRaycasterMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrPerformEnvironmentRaycastMETA);
	return true;
}

void OpenXRMetaEnvironmentRaycastExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_environment_raycast_supported"), &OpenXRMetaEnvironmentRaycastExtension::is_environment_raycast_supported);
	ClassDB::bind_method(D_METHOD("is_raycaster_ready"), &OpenXRMetaEnvironmentRaycastExtension::is_raycaster_ready);
	ClassDB::bind_method(D_METHOD("create_raycaster"), &OpenXRMetaEnvironmentRaycastExtension::create_raycaster);
	ClassDB::bind_method(D_METHOD("destroy_raycaster"), &OpenXRMetaEnvironmentRaycastExtension::destroy_raycaster);
	ClassDB::bind_method(D_METHOD("raycast", "origin", "direction", "max_distance"), &OpenXRMetaEnvironmentRaycastExtension::raycast, DEFVAL(0.0));

	ADD_SIGNAL(MethodInfo("raycaster_ready"));
	ADD_SIGNAL(MethodInfo("raycaster_error", PropertyInfo(Variant::INT, "error_code")));

	BIND_ENUM_CONSTANT(HIT_STATUS_HIT);
	BIND_ENUM_CONSTANT(HIT_STATUS_NO_HIT);
	BIND_ENUM_CONSTANT(HIT_STATUS_HIT_POINT_OCCLUDED);
	BIND_ENUM_CONSTANT(HIT_STATUS_HIT_POINT_OUTSIDE_OF_FOV);
	BIND_ENUM_CONSTANT(HIT_STATUS_RAY_OCCLUDED);
	BIND_ENUM_CONSTANT(HIT_STATUS_HIT_INVALID_ORIENTATION);
}
