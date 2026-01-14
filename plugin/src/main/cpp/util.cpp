/**************************************************************************/
/*  util.cpp                                                              */
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

#include "util.h"

#include <openxr/internal/xr_linear.h>
#include <openxr/openxr.h>
#include <stdio.h>
#include <godot_cpp/variant/projection.hpp>

using namespace godot;

StringName OpenXRUtilities::uuid_to_string_name(const XrUuid &p_uuid) {
	const uint8_t *data = p_uuid.data;
	char uuid_str[37];

	sprintf(uuid_str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			data[0], data[1], data[2], data[3],
			data[4], data[5],
			data[6], data[7],
			data[8], data[9],
			data[10], data[11], data[12], data[13], data[14], data[15]);

	return StringName(uuid_str);
}

void OpenXRUtilities::xrMatrix4x4f_to_godot_projection(XrMatrix4x4f *m, godot::Projection &p) {
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			p.columns[j][i] = m->m[j * 4 + i];
		}
	}
}

Transform3D OpenXRUtilities::xrPosef_to_godot_transform3d(const XrPosef &pose) {
	Transform3D out;
	out.origin.x = pose.position.x;
	out.origin.y = pose.position.y;
	out.origin.z = pose.position.z;
	out.basis = Basis{ Quaternion{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w } };
	return out;
}
