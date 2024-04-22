/**************************************************************************/
/*  openxr_fb_spatial_entity_user.cpp                                     */
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

#include "classes/openxr_fb_spatial_entity_user.h"

#include "extensions/openxr_fb_spatial_entity_user_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialEntityUser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_user_id"), &OpenXRFbSpatialEntityUser::get_user_id);

	ClassDB::bind_static_method("OpenXRFbSpatialEntityUser", D_METHOD("create_user", "user_id"), &OpenXRFbSpatialEntityUser::create_user);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "user_id", PROPERTY_HINT_NONE, ""), "", "get_user_id");
}

String OpenXRFbSpatialEntityUser::_to_string() const {
	return String("[OpenXRFbSpatialEntityUser ") + itos(user_id) + String("]");
}

uint64_t OpenXRFbSpatialEntityUser::get_user_id() const {
	return user_id;
}

Ref<OpenXRFbSpatialEntityUser> OpenXRFbSpatialEntityUser::create_user(uint64_t p_user_id) {
	return Ref<OpenXRFbSpatialEntityUser>(memnew(OpenXRFbSpatialEntityUser(p_user_id)));
}

OpenXRFbSpatialEntityUser::OpenXRFbSpatialEntityUser(uint64_t p_user_id) {
	user_id = p_user_id;

	OpenXRFbSpatialEntityUserExtensionWrapper *spatial_entity_user_extension = OpenXRFbSpatialEntityUserExtensionWrapper::get_singleton();
	if (spatial_entity_user_extension) {
		user = spatial_entity_user_extension->create_user(p_user_id);
	}
}
