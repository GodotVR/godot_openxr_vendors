/**************************************************************************/
/*  openxr_fb_spatial_entity_user.h                                       */
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

#include <openxr/openxr.h>

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {
class OpenXRFbSpatialEntityUser : public RefCounted {
	GDCLASS(OpenXRFbSpatialEntityUser, RefCounted);

	uint64_t user_id = 0;
	XrSpaceUserFB user = XR_NULL_HANDLE;

protected:
	static void _bind_methods();

	String _to_string() const;

public:
	_FORCE_INLINE_ XrSpaceUserFB get_user_handle() { return user; }

	uint64_t get_user_id() const;

	static Ref<OpenXRFbSpatialEntityUser> create_user(uint64_t p_user_id);

	OpenXRFbSpatialEntityUser() = default;
	OpenXRFbSpatialEntityUser(uint64_t p_user_id);
};
} // namespace godot
