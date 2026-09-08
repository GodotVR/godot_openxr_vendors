/**************************************************************************/
/*  openxr_android_geospatial_pose.h                                      */
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

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/quaternion.hpp>

using namespace godot;

class OpenXRAndroidGeospatialPose : public RefCounted {
	GDCLASS(OpenXRAndroidGeospatialPose, RefCounted);

	Quaternion orientation;
	double latitude = 0.0;
	double longitude = 0.0;
	double altitude = 0.0;

	bool orientation_valid = true;
	bool position_valid = true;

	double horizontal_accuracy = 0.0;
	double vertical_accuracy = 0.0;
	double orientation_yaw_accuracy = 0.0;

protected:
	static void _bind_methods();

public:
	void set_orientation(const Quaternion &p_orientation);
	Quaternion get_orientation() const;

	void set_latitude(double p_latitude);
	double get_latitude() const;

	void set_longitude(double p_longitude);
	double get_longitude() const;

	void set_altitude(double p_altitude);
	double get_altitude() const;

	bool is_orientation_valid() const { return orientation_valid; }
	bool is_position_valid() const { return position_valid; }

	double get_horizontal_accuracy() const { return horizontal_accuracy; }
	double get_vertical_accuracy() const { return vertical_accuracy; }
	double get_orientation_yaw_accuracy() const { return orientation_yaw_accuracy; }

	static Ref<OpenXRAndroidGeospatialPose> create(const Quaternion &p_orientation, double p_latitude, double p_longitude, double p_altitude) {
		Ref<OpenXRAndroidGeospatialPose> pose;
		pose.instantiate();
		pose->orientation = p_orientation;
		pose->latitude = p_latitude;
		pose->longitude = p_longitude;
		pose->altitude = p_altitude;
		return pose;
	}

	void set_accuracy(bool p_orientation_valid, bool p_position_valid, double p_horizontal_accuracy, double p_vertical_accuracy, double p_orientation_yaw_accuracy) {
		orientation_valid = p_orientation_valid;
		position_valid = p_position_valid;
		horizontal_accuracy = p_horizontal_accuracy;
		vertical_accuracy = p_vertical_accuracy;
		orientation_yaw_accuracy = p_orientation_yaw_accuracy;
	}
};
