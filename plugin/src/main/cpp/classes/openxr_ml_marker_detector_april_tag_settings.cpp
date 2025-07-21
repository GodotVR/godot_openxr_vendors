/**************************************************************************/
/*  openxr_ml_marker_detector_april_tag_settings.cpp                      */
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

#include "classes/openxr_ml_marker_detector_april_tag_settings.h"

using namespace godot;

void OpenXRMlMarkerDetectorAprilTagSettings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_estimate_april_tag_length", "estimate_april_tag_length"), &OpenXRMlMarkerDetectorAprilTagSettings::set_estimate_april_tag_length);
	ClassDB::bind_method(D_METHOD("get_estimate_april_tag_length"), &OpenXRMlMarkerDetectorAprilTagSettings::get_estimate_april_tag_length);

	ClassDB::bind_method(D_METHOD("set_april_tag_length", "april_tag_length"), &OpenXRMlMarkerDetectorAprilTagSettings::set_april_tag_length);
	ClassDB::bind_method(D_METHOD("get_april_tag_length"), &OpenXRMlMarkerDetectorAprilTagSettings::get_april_tag_length);

	ClassDB::bind_method(D_METHOD("set_april_tag_dictionary", "april_tag_dictionary"), &OpenXRMlMarkerDetectorAprilTagSettings::set_april_tag_dictionary);
	ClassDB::bind_method(D_METHOD("get_april_tag_dictionary"), &OpenXRMlMarkerDetectorAprilTagSettings::get_april_tag_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "estimate_april_tag_length", PROPERTY_HINT_NONE, ""), "set_estimate_april_tag_length", "get_estimate_april_tag_length");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "april_tag_length", PROPERTY_HINT_NONE, ""), "set_april_tag_length", "get_april_tag_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "april_tag_dictionary", PROPERTY_HINT_ENUM, "16H5,25H9,36H10,36H11"), "set_april_tag_dictionary", "get_april_tag_dictionary");

	BIND_ENUM_CONSTANT(APRIL_TAG_DICT_16H5);
	BIND_ENUM_CONSTANT(APRIL_TAG_DICT_25H9);
	BIND_ENUM_CONSTANT(APRIL_TAG_DICT_36H10);
	BIND_ENUM_CONSTANT(APRIL_TAG_DICT_36H11);
}

void OpenXRMlMarkerDetectorAprilTagSettings::set_estimate_april_tag_length(bool p_estimate_april_tag_length) {
	if (estimate_april_tag_length == p_estimate_april_tag_length)
		return;
	estimate_april_tag_length = p_estimate_april_tag_length;
	emit_changed();
}

bool OpenXRMlMarkerDetectorAprilTagSettings::get_estimate_april_tag_length() const {
	return estimate_april_tag_length;
}

void OpenXRMlMarkerDetectorAprilTagSettings::set_april_tag_length(float p_april_tag_length) {
	if (april_tag_length == p_april_tag_length)
		return;
	april_tag_length = p_april_tag_length;
	emit_changed();
}

float OpenXRMlMarkerDetectorAprilTagSettings::get_april_tag_length() const {
	return april_tag_length;
}

void OpenXRMlMarkerDetectorAprilTagSettings::set_april_tag_dictionary(OpenXRMlMarkerDetectorAprilTagSettings::AprilTagDictionary p_april_tag_dictionary) {
	if (april_tag_dictionary == p_april_tag_dictionary)
		return;
	april_tag_dictionary = p_april_tag_dictionary;
	emit_changed();
}

OpenXRMlMarkerDetectorAprilTagSettings::AprilTagDictionary OpenXRMlMarkerDetectorAprilTagSettings::get_april_tag_dictionary() const {
	return april_tag_dictionary;
}

OpenXRMlMarkerDetectorAprilTagSettings::OpenXRMlMarkerDetectorAprilTagSettings() :
		OpenXRMlMarkerDetectorSettings(OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_APRIL_TAG) {
}
