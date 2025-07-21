/**************************************************************************/
/*  openxr_ml_marker_detector_aruco_settings.cpp                          */
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

#include "classes/openxr_ml_marker_detector_aruco_settings.h"

using namespace godot;

void OpenXRMlMarkerDetectorArucoSettings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_estimate_aruco_length", "estimate_aruco_length"), &OpenXRMlMarkerDetectorArucoSettings::set_estimate_aruco_length);
	ClassDB::bind_method(D_METHOD("get_estimate_aruco_length"), &OpenXRMlMarkerDetectorArucoSettings::get_estimate_aruco_length);

	ClassDB::bind_method(D_METHOD("set_aruco_length", "aruco_length"), &OpenXRMlMarkerDetectorArucoSettings::set_aruco_length);
	ClassDB::bind_method(D_METHOD("get_aruco_length"), &OpenXRMlMarkerDetectorArucoSettings::get_aruco_length);

	ClassDB::bind_method(D_METHOD("set_aruco_dictionary", "aruco_dictionary"), &OpenXRMlMarkerDetectorArucoSettings::set_aruco_dictionary);
	ClassDB::bind_method(D_METHOD("get_aruco_dictionary"), &OpenXRMlMarkerDetectorArucoSettings::get_aruco_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "estimate_aruco_length", PROPERTY_HINT_NONE, ""), "set_estimate_aruco_length", "get_estimate_aruco_length");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "aruco_length", PROPERTY_HINT_NONE, ""), "set_aruco_length", "get_aruco_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "aruco_dictionary", PROPERTY_HINT_ENUM, "4X4_50,4X4_100,4X4_250,4X4_1000,5X5_50,5X5_100,5X5_250,5X5_1000,6X6_50,6X6_100,6X6_250,6X6_1000,7X7_50,7X7_100,7X7_250,7X7_1000"), "set_aruco_dictionary", "get_aruco_dictionary");

	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_4X4_50);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_4X4_100);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_4X4_250);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_4X4_1000);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_5X5_50);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_5X5_100);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_5X5_250);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_5X5_1000);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_6X6_50);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_6X6_100);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_6X6_250);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_6X6_1000);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_7X7_50);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_7X7_100);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_7X7_250);
	BIND_ENUM_CONSTANT(ARUCO_DICTIONARY_7X7_1000);
}

void OpenXRMlMarkerDetectorArucoSettings::set_estimate_aruco_length(bool p_estimate_aruco_length) {
	if (estimate_aruco_length == p_estimate_aruco_length)
		return;
	estimate_aruco_length = p_estimate_aruco_length;
	emit_changed();
}

bool OpenXRMlMarkerDetectorArucoSettings::get_estimate_aruco_length() const {
	return estimate_aruco_length;
}

void OpenXRMlMarkerDetectorArucoSettings::set_aruco_length(float p_aruco_length) {
	if (aruco_length == p_aruco_length)
		return;
	aruco_length = p_aruco_length;
	emit_changed();
}

float OpenXRMlMarkerDetectorArucoSettings::get_aruco_length() const {
	return aruco_length;
}

void OpenXRMlMarkerDetectorArucoSettings::set_aruco_dictionary(OpenXRMlMarkerDetectorArucoSettings::ArucoDictionary p_aruco_dictionary) {
	if (aruco_dictionary == p_aruco_dictionary)
		return;
	aruco_dictionary = p_aruco_dictionary;
	emit_changed();
}

OpenXRMlMarkerDetectorArucoSettings::ArucoDictionary OpenXRMlMarkerDetectorArucoSettings::get_aruco_dictionary() const {
	return aruco_dictionary;
}

OpenXRMlMarkerDetectorArucoSettings::OpenXRMlMarkerDetectorArucoSettings() :
		OpenXRMlMarkerDetectorSettings(OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_ARUCO) {
}
