/**************************************************************************/
/*  openxr_ml_marker_detector_profile_settings.cpp                        */
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

#include "classes/openxr_ml_marker_detector_profile_settings.h"

using namespace godot;

void OpenXRMlMarkerDetectorProfileSettings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_fps_hint", "fps_hint"), &OpenXRMlMarkerDetectorProfileSettings::set_fps_hint);
	ClassDB::bind_method(D_METHOD("get_fps_hint"), &OpenXRMlMarkerDetectorProfileSettings::get_fps_hint);

	ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &OpenXRMlMarkerDetectorProfileSettings::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &OpenXRMlMarkerDetectorProfileSettings::get_resolution);

	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &OpenXRMlMarkerDetectorProfileSettings::set_camera);
	ClassDB::bind_method(D_METHOD("get_camera"), &OpenXRMlMarkerDetectorProfileSettings::get_camera);

	ClassDB::bind_method(D_METHOD("set_corner_refine_method", "corner_refine_method"), &OpenXRMlMarkerDetectorProfileSettings::set_corner_refine_method);
	ClassDB::bind_method(D_METHOD("get_corner_refine_method"), &OpenXRMlMarkerDetectorProfileSettings::get_corner_refine_method);

	ClassDB::bind_method(D_METHOD("set_use_edge_refinement", "use_edge_refinement"), &OpenXRMlMarkerDetectorProfileSettings::set_use_edge_refinement);
	ClassDB::bind_method(D_METHOD("get_use_edge_refinement"), &OpenXRMlMarkerDetectorProfileSettings::get_use_edge_refinement);

	ClassDB::bind_method(D_METHOD("set_full_analysis_interval", "full_analysis_interval"), &OpenXRMlMarkerDetectorProfileSettings::set_full_analysis_interval);
	ClassDB::bind_method(D_METHOD("get_full_analysis_interval"), &OpenXRMlMarkerDetectorProfileSettings::get_full_analysis_interval);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "fps_hint", PROPERTY_HINT_ENUM, "Low,Medium,High,Max"), "set_fps_hint", "get_fps_hint");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_ENUM, "Low,Medium,High"), "set_resolution", "get_resolution");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "camera", PROPERTY_HINT_ENUM, "RGB Camera,World Cameras"), "set_camera", "get_camera");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "corner_refine_method", PROPERTY_HINT_ENUM, "None,Subpixel,Contour,April Tag"), "set_corner_refine_method", "get_corner_refine_method");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_edge_refinement", PROPERTY_HINT_NONE, ""), "set_use_edge_refinement", "get_use_edge_refinement");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "full_analysis_interval", PROPERTY_HINT_ENUM, "Max,Fast,Medium,Slow"), "set_full_analysis_interval", "get_full_analysis_interval");

	BIND_ENUM_CONSTANT(FPS_LOW);
	BIND_ENUM_CONSTANT(FPS_MEDIUM);
	BIND_ENUM_CONSTANT(FPS_HIGH);
	BIND_ENUM_CONSTANT(FPS_MAX);

	BIND_ENUM_CONSTANT(RESOLUTION_LOW);
	BIND_ENUM_CONSTANT(RESOLUTION_MEDIUM);
	BIND_ENUM_CONSTANT(RESOLUTION_HIGH);

	BIND_ENUM_CONSTANT(CAMERA_RGB_CAMERA);
	BIND_ENUM_CONSTANT(CAMERA_WORLD_CAMERAS);

	BIND_ENUM_CONSTANT(CORNER_REFINE_METHOD_NONE);
	BIND_ENUM_CONSTANT(CORNER_REFINE_METHOD_SUBPIX);
	BIND_ENUM_CONSTANT(CORNER_REFINE_METHOD_CONTOUR);
	BIND_ENUM_CONSTANT(CORNER_REFINE_METHOD_APRIL_TAG);

	BIND_ENUM_CONSTANT(FULL_ANALYSIS_INTERVAL_MAX);
	BIND_ENUM_CONSTANT(FULL_ANALYSIS_INTERVAL_FAST);
	BIND_ENUM_CONSTANT(FULL_ANALYSIS_INTERVAL_MEDIUM);
	BIND_ENUM_CONSTANT(FULL_ANALYSIS_INTERVAL_SLOW);
}

void OpenXRMlMarkerDetectorProfileSettings::set_fps_hint(OpenXRMlMarkerDetectorProfileSettings::Fps p_fps_hint) {
	if (fps_hint == p_fps_hint)
		return;
	fps_hint = p_fps_hint;
	emit_changed();
}

OpenXRMlMarkerDetectorProfileSettings::Fps OpenXRMlMarkerDetectorProfileSettings::get_fps_hint() const {
	return fps_hint;
}

void OpenXRMlMarkerDetectorProfileSettings::set_resolution(OpenXRMlMarkerDetectorProfileSettings::Resolution p_resolution) {
	if (resolution == p_resolution)
		return;
	resolution = p_resolution;
	emit_changed();
}

OpenXRMlMarkerDetectorProfileSettings::Resolution OpenXRMlMarkerDetectorProfileSettings::get_resolution() const {
	return resolution;
}

void OpenXRMlMarkerDetectorProfileSettings::set_camera(OpenXRMlMarkerDetectorProfileSettings::Camera p_camera) {
	if (camera == p_camera)
		return;
	camera = p_camera;
	emit_changed();
}

OpenXRMlMarkerDetectorProfileSettings::Camera OpenXRMlMarkerDetectorProfileSettings::get_camera() const {
	return camera;
}

void OpenXRMlMarkerDetectorProfileSettings::set_corner_refine_method(OpenXRMlMarkerDetectorProfileSettings::CornerRefineMethod p_corner_refine_method) {
	if (corner_refine_method == p_corner_refine_method)
		return;
	corner_refine_method = p_corner_refine_method;
	emit_changed();
}

OpenXRMlMarkerDetectorProfileSettings::CornerRefineMethod OpenXRMlMarkerDetectorProfileSettings::get_corner_refine_method() const {
	return corner_refine_method;
}

void OpenXRMlMarkerDetectorProfileSettings::set_use_edge_refinement(bool p_use_edge_refinement) {
	if (use_edge_refinement == p_use_edge_refinement)
		return;
	use_edge_refinement = p_use_edge_refinement;
	emit_changed();
}

bool OpenXRMlMarkerDetectorProfileSettings::get_use_edge_refinement() const {
	return use_edge_refinement;
}

void OpenXRMlMarkerDetectorProfileSettings::set_full_analysis_interval(OpenXRMlMarkerDetectorProfileSettings::FullAnalysisInterval p_full_analysis_interval) {
	if (full_analysis_interval == p_full_analysis_interval)
		return;
	full_analysis_interval = p_full_analysis_interval;
	emit_changed();
}

OpenXRMlMarkerDetectorProfileSettings::FullAnalysisInterval OpenXRMlMarkerDetectorProfileSettings::get_full_analysis_interval() const {
	return full_analysis_interval;
}
