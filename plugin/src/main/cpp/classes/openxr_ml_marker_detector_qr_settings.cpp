/**************************************************************************/
/*  openxr_ml_marker_detector_qr_settings.cpp                             */
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

#include "classes/openxr_ml_marker_detector_qr_settings.h"

using namespace godot;

void OpenXRMlMarkerDetectorQrSettings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_estimate_qr_length", "estimate_qr_length"), &OpenXRMlMarkerDetectorQrSettings::set_estimate_qr_length);
	ClassDB::bind_method(D_METHOD("get_estimate_qr_length"), &OpenXRMlMarkerDetectorQrSettings::get_estimate_qr_length);

	ClassDB::bind_method(D_METHOD("set_qr_length", "qr_length"), &OpenXRMlMarkerDetectorQrSettings::set_qr_length);
	ClassDB::bind_method(D_METHOD("get_qr_length"), &OpenXRMlMarkerDetectorQrSettings::get_qr_length);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "estimate_qr_length", PROPERTY_HINT_NONE, ""), "set_estimate_qr_length", "get_estimate_qr_length");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "qr_length", PROPERTY_HINT_NONE, ""), "set_qr_length", "get_qr_length");
}

void OpenXRMlMarkerDetectorQrSettings::set_estimate_qr_length(bool p_estimate_qr_length) {
	if (estimate_qr_length == p_estimate_qr_length)
		return;
	estimate_qr_length = p_estimate_qr_length;
	emit_changed();
}

bool OpenXRMlMarkerDetectorQrSettings::get_estimate_qr_length() const {
	return estimate_qr_length;
}

void OpenXRMlMarkerDetectorQrSettings::set_qr_length(float p_qr_length) {
	if (qr_length == p_qr_length)
		return;
	qr_length = p_qr_length;
	emit_changed();
}

float OpenXRMlMarkerDetectorQrSettings::get_qr_length() const {
	return qr_length;
}

OpenXRMlMarkerDetectorQrSettings::OpenXRMlMarkerDetectorQrSettings() :
		OpenXRMlMarkerDetectorSettings(OpenXRMlMarkerDetectorSettings::MarkerType::MARKER_TYPE_QR) {
}
