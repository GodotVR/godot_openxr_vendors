/**************************************************************************/
/*  openxr_ml_marker_detector_profile_settings.h                          */
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

#ifndef OPENXR_ML_MARKER_DETECTOR_PROFILE_SETTINGS_H
#define OPENXR_ML_MARKER_DETECTOR_PROFILE_SETTINGS_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/binder_common.hpp>

namespace godot {
class OpenXRMlMarkerDetectorProfileSettings : public Resource {
	GDCLASS(OpenXRMlMarkerDetectorProfileSettings, Resource);

public:
	enum Fps {
		FPS_LOW = 0,
		FPS_MEDIUM = 1,
		FPS_HIGH = 2,
		FPS_MAX = 3,
	};
	enum Resolution {
		RESOLUTION_LOW = 0,
		RESOLUTION_MEDIUM = 1,
		RESOLUTION_HIGH = 2,
	};
	enum Camera {
		CAMERA_RGB_CAMERA = 0,
		CAMERA_WORLD_CAMERAS = 1,
	};
	enum CornerRefineMethod {
		CORNER_REFINE_METHOD_NONE = 0,
		CORNER_REFINE_METHOD_SUBPIX = 1,
		CORNER_REFINE_METHOD_CONTOUR = 2,
		CORNER_REFINE_METHOD_APRIL_TAG = 3,
	};
	enum FullAnalysisInterval {
		FULL_ANALYSIS_INTERVAL_MAX = 0,
		FULL_ANALYSIS_INTERVAL_FAST = 1,
		FULL_ANALYSIS_INTERVAL_MEDIUM = 2,
		FULL_ANALYSIS_INTERVAL_SLOW = 3,
	};

private:
	Fps fps_hint = Fps::FPS_LOW;
	Resolution resolution = Resolution::RESOLUTION_LOW;
	Camera camera = Camera::CAMERA_RGB_CAMERA;
	CornerRefineMethod corner_refine_method = CornerRefineMethod::CORNER_REFINE_METHOD_NONE;
	bool use_edge_refinement = false;
	FullAnalysisInterval full_analysis_interval = FullAnalysisInterval::FULL_ANALYSIS_INTERVAL_MAX;

protected:
	static void _bind_methods();

public:
	void set_fps_hint(Fps p_fps_hint);
	Fps get_fps_hint() const;

	void set_resolution(Resolution p_resolution);
	Resolution get_resolution() const;

	void set_camera(Camera p_camera);
	Camera get_camera() const;

	void set_corner_refine_method(CornerRefineMethod p_corner_refine_method);
	CornerRefineMethod get_corner_refine_method() const;

	void set_use_edge_refinement(bool p_use_edge_refinement);
	bool get_use_edge_refinement() const;

	void set_full_analysis_interval(FullAnalysisInterval p_full_analysis_interval);
	FullAnalysisInterval get_full_analysis_interval() const;
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorProfileSettings::Fps);
VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorProfileSettings::Resolution);
VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorProfileSettings::Camera);
VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorProfileSettings::CornerRefineMethod);
VARIANT_ENUM_CAST(OpenXRMlMarkerDetectorProfileSettings::FullAnalysisInterval);

#endif
