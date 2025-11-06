/**************************************************************************/
/*  openxr_ml_marker_detector_settings.cpp                                */
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

#include "classes/openxr_ml_marker_detector_settings.h"

using namespace godot;

void OpenXRMlMarkerDetectorSettings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_marker_type"), &OpenXRMlMarkerDetectorSettings::get_marker_type);

	ClassDB::bind_method(D_METHOD("set_marker_detector_profile", "marker_detector_profile"), &OpenXRMlMarkerDetectorSettings::set_marker_detector_profile);
	ClassDB::bind_method(D_METHOD("get_marker_detector_profile"), &OpenXRMlMarkerDetectorSettings::get_marker_detector_profile);

	ClassDB::bind_method(D_METHOD("set_marker_detector_profile_settings", "marker_detector_profile_settings"), &OpenXRMlMarkerDetectorSettings::set_marker_detector_profile_settings);
	ClassDB::bind_method(D_METHOD("get_marker_detector_profile_settings"), &OpenXRMlMarkerDetectorSettings::get_marker_detector_profile_settings);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "marker_detector_profile", PROPERTY_HINT_ENUM, "Default,Speed,Accuracy,Small Targets,Large FOV,Custom"), "set_marker_detector_profile", "get_marker_detector_profile");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "marker_detector_profile_settings", PROPERTY_HINT_RESOURCE_TYPE, "OpenXRMlMarkerDetectorProfileSettings"), "set_marker_detector_profile_settings", "get_marker_detector_profile_settings");

	BIND_ENUM_CONSTANT(MARKER_TYPE_ARUCO);
	BIND_ENUM_CONSTANT(MARKER_TYPE_APRIL_TAG);
	BIND_ENUM_CONSTANT(MARKER_TYPE_QR);
	BIND_ENUM_CONSTANT(MARKER_TYPE_EAN_13);
	BIND_ENUM_CONSTANT(MARKER_TYPE_UPC_A);
	BIND_ENUM_CONSTANT(MARKER_TYPE_CODE_128);

	BIND_ENUM_CONSTANT(MARKER_DETECTOR_PROFILE_DEFAULT);
	BIND_ENUM_CONSTANT(MARKER_DETECTOR_PROFILE_SPEED);
	BIND_ENUM_CONSTANT(MARKER_DETECTOR_PROFILE_ACCURACY);
	BIND_ENUM_CONSTANT(MARKER_DETECTOR_PROFILE_SMALL_TARGETS);
	BIND_ENUM_CONSTANT(MARKER_DETECTOR_PROFILE_LARGE_FOV);
	BIND_ENUM_CONSTANT(MARKER_DETECTOR_PROFILE_CUSTOM);
}

OpenXRMlMarkerDetectorSettings::MarkerType OpenXRMlMarkerDetectorSettings::get_marker_type() const {
	return marker_type;
}

void OpenXRMlMarkerDetectorSettings::set_marker_detector_profile(OpenXRMlMarkerDetectorSettings::MarkerDetectorProfile p_marker_detector_profile) {
	if (marker_detector_profile == p_marker_detector_profile)
		return;
	marker_detector_profile = p_marker_detector_profile;
	emit_changed();
}

OpenXRMlMarkerDetectorSettings::MarkerDetectorProfile OpenXRMlMarkerDetectorSettings::get_marker_detector_profile() const {
	return marker_detector_profile;
}

void OpenXRMlMarkerDetectorSettings::set_marker_detector_profile_settings(const Ref<OpenXRMlMarkerDetectorProfileSettings> &p_marker_detector_profile_settings) {
	if (marker_detector_profile_settings == p_marker_detector_profile_settings)
		return;
	marker_detector_profile_settings = p_marker_detector_profile_settings;
	emit_changed();
}

Ref<OpenXRMlMarkerDetectorProfileSettings> OpenXRMlMarkerDetectorSettings::get_marker_detector_profile_settings() const {
	return marker_detector_profile_settings;
}

OpenXRMlMarkerDetectorSettings::OpenXRMlMarkerDetectorSettings(OpenXRMlMarkerDetectorSettings::MarkerType p_marker_type) {
	marker_type = p_marker_type;
}
