/**************************************************************************/
/*  openxr_fb_spatial_entity_batch.cpp                                    */
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

#include "classes/openxr_fb_spatial_entity_batch.h"

#include <godot_cpp/templates/local_vector.hpp>

#include "extensions/openxr_fb_spatial_entity_storage_batch_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_sharing_extension_wrapper.h"

#include "classes/openxr_fb_spatial_entity_user.h"

using namespace godot;

void OpenXRFbSpatialEntityBatch::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_entities"), &OpenXRFbSpatialEntityBatch::get_entities);

	ClassDB::bind_method(D_METHOD("save_to_storage", "location"), &OpenXRFbSpatialEntityBatch::save_to_storage);
	ClassDB::bind_method(D_METHOD("share_with_users", "users"), &OpenXRFbSpatialEntityBatch::share_with_users);

	ClassDB::bind_static_method("OpenXRFbSpatialEntityBatch", D_METHOD("create_batch", "entities"), &OpenXRFbSpatialEntityBatch::create_batch);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "entities", PROPERTY_HINT_ARRAY_TYPE, "OpenXRFbSpatialEntity"), "", "get_entities");

	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_batch_saved", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "location")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_entity_batch_shared", PropertyInfo(Variant::Type::BOOL, "succeeded")));
}

String OpenXRFbSpatialEntityBatch::_to_string() const {
	PackedStringArray uuids;
	uuids.resize(entities.size());
	for (int i = 0; i < entities.size(); i++) {
		Ref<OpenXRFbSpatialEntity> entity = entities[i];
		uuids[i] = entity.is_valid() ? entity->get_uuid() : "null";
	}
	return String("[OpenXRFbSpatialEntityBatch ") + String(", ").join(uuids) + String("]");
}

TypedArray<OpenXRFbSpatialEntity> OpenXRFbSpatialEntityBatch::get_entities() const {
	return entities;
}

void OpenXRFbSpatialEntityBatch::save_to_storage(OpenXRFbSpatialEntity::StorageLocation p_location) {
	XrSpaceListSaveInfoFB save_info = {
		XR_TYPE_SPACE_LIST_SAVE_INFO_FB, // type
		nullptr, // next
		(uint32_t)spaces.size(), // spaceCount
		const_cast<XrSpace *>(spaces.ptr()), // spaces
		OpenXRFbSpatialEntity::to_openxr_storage_location(p_location), // location
	};

	Ref<OpenXRFbSpatialEntityBatch> *userdata = memnew(Ref<OpenXRFbSpatialEntityBatch>(this));
	OpenXRFbSpatialEntityStorageBatchExtensionWrapper::get_singleton()->save_spaces(&save_info, OpenXRFbSpatialEntityBatch::_on_save_to_storage, userdata);
}

void OpenXRFbSpatialEntityBatch::_on_save_to_storage(XrResult p_result, XrSpaceStorageLocationFB p_location, void *p_userdata) {
	Ref<OpenXRFbSpatialEntityBatch> *userdata = (Ref<OpenXRFbSpatialEntityBatch> *)p_userdata;
	(*userdata)->emit_signal("openxr_fb_spatial_entity_batch_saved", XR_SUCCEEDED(p_result), OpenXRFbSpatialEntity::from_openxr_storage_location(p_location));
	memdelete(userdata);
}

void OpenXRFbSpatialEntityBatch::share_with_users(const TypedArray<OpenXRFbSpatialEntityUser> &p_users) {
	LocalVector<XrSpaceUserFB> users;
	users.resize(p_users.size());
	for (int i = 0; i < p_users.size(); i++) {
		Ref<OpenXRFbSpatialEntityUser> user = p_users[i];
		users[i] = user->get_user_handle();
	}

	XrSpaceShareInfoFB info = {
		XR_TYPE_SPACE_SHARE_INFO_FB, // type
		nullptr, // next
		(uint32_t)spaces.size(), // spaceCount
		const_cast<XrSpace *>(spaces.ptr()), // spaces
		(uint32_t)users.size(), // userCount
		users.ptr(), // users
	};

	Ref<OpenXRFbSpatialEntityBatch> *userdata = memnew(Ref<OpenXRFbSpatialEntityBatch>(this));
	OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton()->share_spaces(&info, OpenXRFbSpatialEntityBatch::_on_share_with_users, userdata);
}

void OpenXRFbSpatialEntityBatch::_on_share_with_users(XrResult p_result, void *p_userdata) {
	Ref<OpenXRFbSpatialEntityBatch> *userdata = (Ref<OpenXRFbSpatialEntityBatch> *)p_userdata;
	(*userdata)->emit_signal("openxr_fb_spatial_entity_batch_shared", XR_SUCCEEDED(p_result));
	memdelete(userdata);
}


Ref<OpenXRFbSpatialEntityBatch> OpenXRFbSpatialEntityBatch::create_batch(const TypedArray<OpenXRFbSpatialEntity> &p_entities) {
	return Ref<OpenXRFbSpatialEntityBatch>(memnew(OpenXRFbSpatialEntityBatch(p_entities)));
}

OpenXRFbSpatialEntityBatch::OpenXRFbSpatialEntityBatch(const TypedArray<OpenXRFbSpatialEntity> &p_entities) {
	entities = p_entities;

	for (int i = 0; i < entities.size(); i++) {
		Ref<OpenXRFbSpatialEntity> entity = entities[i];
		if (entity.is_valid()) {
			XrSpace space = entity->get_space();
			if (space != XR_NULL_HANDLE) {
				spaces.push_back(space);
			}
		}
	}
}
