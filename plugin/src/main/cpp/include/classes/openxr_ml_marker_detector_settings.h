/**************************************************************************/
/*  openxr_ml_marker_detector_settings.h                                  */
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

#ifndef OPENXR_ML_MARKER_DETECTOR_SETTINGS_H
#define OPENXR_ML_MARKER_DETECTOR_SETTINGS_H

#include "classes/openxr_ml_marker_detector_profile_settings.h"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/binder_common.hpp>

namespace godot {
class OpenXRMlMarkerDetectorSettings : public Resource {
	GDCLASS(OpenXRMlMarkerDetectorSettings, Resource);

public:
	enum MarkerType {
		MARKER_TYPE_ARUCO = 0,
		MARKER_TYPE_APRIL_TAG = 1,
		MARKER_TYPE_QR = 2,
		MARKER_TYPE_EAN_13 = 3,
		MARKER_TYPE_UPC_A = 4,
		MARKER_TYPE_CODE_128 = 5,
	};

	enum MarkerDetectorProfile {
		MARKER_DETECTOR_PROFILE_DEFAULT = 0,
		MARKER_DETECTOR_PROFILE_SPEED = 1,
		MARKER_DETECTOR_PROFILE_ACCURACY = 2,
		MARKER_DETECTOR_PROFILE_SMALL_TARGETS = 3,
		MARKER_DETECTOR_PROFILE_LARGE_FOV = 4,
		MARKER_DETECTOR_PROFILE_CUSTOM = 5,
	};

private:
	MarkerType marker_type;
	MarkerDetectorProfile marker_detector_profile = MarkerDetectorProfile::MARKER_DETECTOR_PROFILE_DEFAULT;
	Ref<OpenXRMlMarkerDetectorProfileSettings> marker_detector_profile_settings;

protected:
	static void _bind_methods();

public:
	MarkerType get_marker_type() const;

	void set_marker_detector_profile(MarkerDetectorProfile p_marker_detector_profile);
	MarkerDetectorProfile get_marker_detector_profile() const;

	void set_marker_detector_profile_settings(const Ref<OpenXRMlMarkerDetectorProfileSettings> &p_marker_detector_profile_settings);
	Ref<OpenXRMlMarkerDetectorProfileSettings> get_marker_detector_profile_settings() const;

	OpenXRMlMarkerDetectorSettings() = default;
	OpenXRMlMarkerDetectorSettings(MarkerType marker_type);
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorSettings::MarkerType);
VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorSettings::MarkerDetectorProfile);

#endif
