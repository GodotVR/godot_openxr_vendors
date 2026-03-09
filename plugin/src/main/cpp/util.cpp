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
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/projection.hpp>

using namespace godot;

namespace {
bool hexchar_to_uint8(uint8_t &out, char32_t hexchar) {
	if (U'0' <= hexchar && hexchar <= U'9') {
		out = hexchar - U'0';
		return true;
	}

	if (U'a' <= hexchar && hexchar <= U'f') {
		out = 10 + (hexchar - U'a');
		return true;
	}

	return false;
}

bool hexchars_to_uint8(uint8_t &ret, char32_t hexchar0, char32_t hexchar1) {
	// ab
	// ||
	// |hexchar1
	// hexchar0
	uint8_t d0;
	uint8_t d1;
	if (!hexchar_to_uint8(d0, hexchar0) || !hexchar_to_uint8(d1, hexchar1)) {
		return false;
	}

	ret = (d0 * 16) + d1;
	return true;
}
} //namespace

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

Vector3 OpenXRUtilities::XrVector3f_to_godot_vector3(const XrVector3f &vector) {
	Vector3 out;
	out.x = vector.x;
	out.y = vector.y;
	out.z = vector.z;
	return out;
}

XrUuid OpenXRUtilities::string_name_to_uuid(const StringName &p_uuid_str) {
	// expecting p_uuid_str of the form: "ffffffff-ffff-ffff-ffff-ffffffffffff"
	// note that we expect the uuid to be all lowercase (so A, B, C, D, E, and F are invalid)

	// expecting a string generated from uuid_to_string_name(), which has 36 characters
	if (p_uuid_str.length() != 36) {
		return XrUuid{};
	}

	// expecting a string generated from uuid_to_string_name(), which has 5 chunks
	PackedStringArray strs = p_uuid_str.split("-");
	if (strs.size() != 5 || strs[0].length() != 8 || strs[1].length() != 4 || strs[2].length() != 4 || strs[3].length() != 4 || strs[4].length() != 12) {
		return XrUuid{};
	}

	XrUuid ret;
	uint8_t *data = &ret.data[0];
	const char32_t *chunk0{ strs[0].ptr() };
	const char32_t *chunk1{ strs[1].ptr() };
	const char32_t *chunk2{ strs[2].ptr() };
	const char32_t *chunk3{ strs[3].ptr() };
	const char32_t *chunk4{ strs[4].ptr() };
	if (!hexchars_to_uint8(data[0], chunk0[0], chunk0[1]) ||
			!hexchars_to_uint8(data[1], chunk0[2], chunk0[3]) ||
			!hexchars_to_uint8(data[2], chunk0[4], chunk0[5]) ||
			!hexchars_to_uint8(data[3], chunk0[6], chunk0[7]) ||
			!hexchars_to_uint8(data[4], chunk1[0], chunk1[1]) ||
			!hexchars_to_uint8(data[5], chunk1[2], chunk1[3]) ||
			!hexchars_to_uint8(data[6], chunk2[0], chunk2[1]) ||
			!hexchars_to_uint8(data[7], chunk2[2], chunk2[3]) ||
			!hexchars_to_uint8(data[8], chunk3[0], chunk3[1]) ||
			!hexchars_to_uint8(data[9], chunk3[2], chunk3[3]) ||
			!hexchars_to_uint8(data[10], chunk4[0], chunk4[1]) ||
			!hexchars_to_uint8(data[11], chunk4[2], chunk4[3]) ||
			!hexchars_to_uint8(data[12], chunk4[4], chunk4[5]) ||
			!hexchars_to_uint8(data[13], chunk4[6], chunk4[7]) ||
			!hexchars_to_uint8(data[14], chunk4[8], chunk4[9]) ||
			!hexchars_to_uint8(data[15], chunk4[10], chunk4[11])) {
		return XrUuid{};
	}

	return ret;
}
