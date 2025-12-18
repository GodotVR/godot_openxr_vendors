/**************************************************************************/
/*  openxr_ml_marker_detector_aruco_settings.h                            */
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

#pragma once

#include "classes/openxr_ml_marker_detector_settings.h"

#include <godot_cpp/core/binder_common.hpp>

namespace godot {
class OpenXRMlMarkerDetectorArucoSettings : public OpenXRMlMarkerDetectorSettings {
	GDCLASS(OpenXRMlMarkerDetectorArucoSettings, OpenXRMlMarkerDetectorSettings);

public:
	enum ArucoDictionary {
		ARUCO_DICTIONARY_4X4_50 = 0,
		ARUCO_DICTIONARY_4X4_100 = 1,
		ARUCO_DICTIONARY_4X4_250 = 2,
		ARUCO_DICTIONARY_4X4_1000 = 3,
		ARUCO_DICTIONARY_5X5_50 = 4,
		ARUCO_DICTIONARY_5X5_100 = 5,
		ARUCO_DICTIONARY_5X5_250 = 6,
		ARUCO_DICTIONARY_5X5_1000 = 7,
		ARUCO_DICTIONARY_6X6_50 = 8,
		ARUCO_DICTIONARY_6X6_100 = 9,
		ARUCO_DICTIONARY_6X6_250 = 10,
		ARUCO_DICTIONARY_6X6_1000 = 11,
		ARUCO_DICTIONARY_7X7_50 = 12,
		ARUCO_DICTIONARY_7X7_100 = 13,
		ARUCO_DICTIONARY_7X7_250 = 14,
		ARUCO_DICTIONARY_7X7_1000 = 15,
	};

private:
	bool estimate_aruco_length = false;
	float aruco_length = 0;
	ArucoDictionary aruco_dictionary = ArucoDictionary::ARUCO_DICTIONARY_4X4_50;

protected:
	static void _bind_methods();

public:
	void set_estimate_aruco_length(bool p_estimate_aruco_length);
	bool get_estimate_aruco_length() const;

	void set_aruco_length(float p_aruco_length);
	float get_aruco_length() const;

	void set_aruco_dictionary(ArucoDictionary p_aruco_dictionary);
	ArucoDictionary get_aruco_dictionary() const;

	OpenXRMlMarkerDetectorArucoSettings();
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorArucoSettings::ArucoDictionary);
