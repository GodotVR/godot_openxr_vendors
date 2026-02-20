/**************************************************************************/
/*  openxr_android_raycast_extension.cpp                                  */
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

#include "extensions/openxr_android_raycast_extension.h"
#include "extensions/openxr_android_trackables_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidRaycastExtension *OpenXRAndroidRaycastExtension::singleton = nullptr;

OpenXRAndroidRaycastExtension *OpenXRAndroidRaycastExtension::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRAndroidRaycastExtension());
	}
	return singleton;
}

OpenXRAndroidRaycastExtension::OpenXRAndroidRaycastExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidRaycastExtension singleton already exists.");

	singleton = this;
	request_extensions[XR_ANDROID_RAYCAST_EXTENSION_NAME] = &available;
}

OpenXRAndroidRaycastExtension::~OpenXRAndroidRaycastExtension() {
	singleton = nullptr;
}

Dictionary OpenXRAndroidRaycastExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRAndroidRaycastExtension::_on_instance_created(uint64_t p_instance) {
	if (!available) {
		return;
	}

	if (!_initialize_androidxr_raycast_extension()) {
		UtilityFunctions::print("Failed to initialize raycast extension");
		available = false;
		return;
	}
}

void OpenXRAndroidRaycastExtension::_on_session_created(uint64_t p_session_instance) {
	if (!available) {
		return;
	}

	XrInstance xr_instance = (XrInstance)get_openxr_api()->get_instance();
	XrSystemId xr_system_id = (XrSystemId)get_openxr_api()->get_system_id();
	uint32_t trackable_type_count_output = 0;
	XrResult result = xrEnumerateRaycastSupportedTrackableTypesANDROID(xr_instance, xr_system_id, 0, &trackable_type_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to query raycast supported trackable types; ", get_openxr_api()->get_error_string(result));
		available = false;
		return;
	}

	if (trackable_type_count_output == 0) {
		WARN_PRINT("OpenXR: query raycast supported trackables types returned zero");
		available = false;
		return;
	}

	Vector<XrTrackableTypeANDROID> supported_xrtrackable_types;
	supported_xrtrackable_types.resize(trackable_type_count_output);
	result = xrEnumerateRaycastSupportedTrackableTypesANDROID(xr_instance, xr_system_id, trackable_type_count_output, &trackable_type_count_output, supported_xrtrackable_types.ptrw());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to re-query raycast supported trackable types; ", get_openxr_api()->get_error_string(result));
		available = false;
		return;
	}

	if (supported_xrtrackable_types.size() != trackable_type_count_output) {
		WARN_PRINT("OpenXR: query raycast supported trackable types returned a different count");
		if (supported_xrtrackable_types.size() < trackable_type_count_output) {
			UtilityFunctions::printerr("OpenXR: somehow received more trackable types on the second query; ", get_openxr_api()->get_error_string(result));
			available = false;
			return;
		}

		if (trackable_type_count_output == 0) {
			UtilityFunctions::printerr("OpenXR: second query raycast supported trackables types returned zero");
			available = false;
			return;
		}

		supported_xrtrackable_types.resize(trackable_type_count_output);
	}

	int i = 0;
	for (; i < supported_xrtrackable_types.size(); ++i) {
		if (supported_xrtrackable_types[i] != XR_TRACKABLE_TYPE_NOT_VALID_ANDROID) {
			break;
		}
	}

	if (i == supported_xrtrackable_types.size()) {
		UtilityFunctions::printerr("OpenXR: query raycast supported trackables types only returned invalid trackable types");
		available = false;
		return;
	}

	for (XrTrackableTypeANDROID xr_trackable_type : supported_xrtrackable_types) {
		switch ((int)xr_trackable_type) {
			case XR_TRACKABLE_TYPE_PLANE_ANDROID:
				supported_trackable_types.push_back(TRACKABLE_TYPE_PLANE);
				break;
			case XR_TRACKABLE_TYPE_DEPTH_ANDROID:
				supported_trackable_types.push_back(TRACKABLE_TYPE_DEPTH);
				break;
			case XR_TRACKABLE_TYPE_NOT_VALID_ANDROID:
			case XR_TRACKABLE_TYPE_OBJECT_ANDROID:
			case XR_TRACKABLE_TYPE_MAX_ENUM_ANDROID:
			default:
				break;
		}
	}

	if (supported_trackable_types.is_empty()) {
		UtilityFunctions::printerr("OpenXR: found zero supported trackable types");
		available = false;
		return;
	}
}

TypedArray<OpenXRAndroidHitResult> OpenXRAndroidRaycastExtension::raycast(Array p_trackable_types, const Vector3 &p_origin, const Vector3 &p_trajectory, int p_max_results) {
	TypedArray<OpenXRAndroidHitResult> ret;
	if (!available) {
		return ret;
	}

	Vector<XrTrackableTrackerANDROID> xrtrackers;
	HashSet<TrackableType> seen_trackable_types;
	for (int i = 0; i < p_trackable_types.size(); ++i) {
		if (p_trackable_types[i].get_type() != Variant::Type::INT) {
			WARN_PRINT("OpenXR: trackable type must be an int");
			continue;
		}

		TrackableType requested_trackable_type_value = (TrackableType)((int)p_trackable_types[i]);
		if (!supported_trackable_types.has(requested_trackable_type_value)) {
			WARN_PRINT(vformat("OpenXR: trackable type %d is not supported", requested_trackable_type_value));
			continue;
		}

		// ensure only one of each trackable
		if (seen_trackable_types.end() == seen_trackable_types.find(requested_trackable_type_value)) {
			XrTrackableTypeANDROID xrtrackable = XR_TRACKABLE_TYPE_NOT_VALID_ANDROID;
			switch (requested_trackable_type_value) {
				case TRACKABLE_TYPE_PLANE:
					xrtrackable = XR_TRACKABLE_TYPE_PLANE_ANDROID;
					break;
				case TRACKABLE_TYPE_DEPTH:
					xrtrackable = XR_TRACKABLE_TYPE_DEPTH_ANDROID;
					break;
				default:
					// should never get here; the for loop above should have caught this already
					WARN_PRINT(vformat("OpenXR: trackable type %d is not supported", requested_trackable_type_value));
					continue;
			}
			xrtrackers.push_back(OpenXRAndroidTrackablesExtension::get_singleton()->get_or_create_xrtrackable_tracker(xrtrackable));
			seen_trackable_types.insert(requested_trackable_type_value);
		}
	}

	XrVector3f origin{ p_origin.x, p_origin.y, p_origin.z };

	XrVector3f trajectory{ p_trajectory.x, p_trajectory.y, p_trajectory.z };

	XrRaycastInfoANDROID xr_raycast_info = {
		XR_TYPE_RAYCAST_INFO_ANDROID, // type
		nullptr, // next
		(uint32_t)p_max_results, // maxResults
		(uint32_t)xrtrackers.size(), // trackerCount
		xrtrackers.ptr(), // trackers
		origin, // origin
		trajectory, // trajectory
		(XrSpace)get_openxr_api()->get_play_space(), // space
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
	};

	Vector<XrRaycastHitResultANDROID> results;
	results.resize(xr_raycast_info.maxResults);
	XrRaycastHitResultsANDROID hit_results = {
		XR_TYPE_RAYCAST_HIT_RESULTS_ANDROID, // type
		nullptr, // next
		(uint32_t)results.size(), // resultsCapacityInput
		0, // resultsCountOutput
		results.ptrw(), // results
	};
	XrResult result = xrRaycastANDROID(SESSION, &xr_raycast_info, &hit_results);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to raycast; ", get_openxr_api()->get_error_string(result));
		return ret;
	}

	if (hit_results.resultsCountOutput == 0) {
		return ret;
	}

	for (int i = 0; i < hit_results.resultsCountOutput; ++i) {
		Ref<OpenXRAndroidHitResult> hit_result{};
		hit_result.instantiate();
		if (hit_result.is_valid() && hit_result->set_hit_result(hit_results.results[i])) {
			ret.push_back(hit_result);
		}
	}

	return ret;
}

bool OpenXRAndroidRaycastExtension::is_raycast_supported() const {
	return available;
}

void OpenXRAndroidRaycastExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("raycast", "trackable_types", "origin", "trajectory", "max_results"), &OpenXRAndroidRaycastExtension::raycast);
	ClassDB::bind_method(D_METHOD("is_raycast_supported"), &OpenXRAndroidRaycastExtension::is_raycast_supported);
	BIND_ENUM_CONSTANT(TRACKABLE_TYPE_PLANE);
	BIND_ENUM_CONSTANT(TRACKABLE_TYPE_DEPTH);
}

bool OpenXRAndroidRaycastExtension::_initialize_androidxr_raycast_extension() {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateRaycastSupportedTrackableTypesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrRaycastANDROID);
	return true;
}
