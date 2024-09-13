/**************************************************************************/
/*  openxr_fb_spatial_entity_batch.h                                      */
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

#ifndef OPENXR_FB_SPATIAL_ENTITY_BATCH_H
#define OPENXR_FB_SPATIAL_ENTITY_BATCH_H

#include <openxr/openxr.h>

#include "openxr_fb_spatial_entity.h"

namespace godot {
class OpenXRFbSpatialEntityUser;

class OpenXRFbSpatialEntityBatch : public RefCounted {
	GDCLASS(OpenXRFbSpatialEntityBatch, RefCounted);

	Vector<XrSpace> spaces;
	TypedArray<OpenXRFbSpatialEntity> entities;

protected:
	static void _bind_methods();

	static void _on_save_to_storage(XrResult p_result, XrSpaceStorageLocationFB p_location, void *p_userdata);
	static void _on_share_with_users(XrResult p_result, void *p_userdata);

	String _to_string() const;

public:
	TypedArray<OpenXRFbSpatialEntity> get_entities() const;

	void save_to_storage(OpenXRFbSpatialEntity::StorageLocation p_location);
	void share_with_users(const TypedArray<OpenXRFbSpatialEntityUser> &p_users);

	static Ref<OpenXRFbSpatialEntityBatch> create_batch(const TypedArray<OpenXRFbSpatialEntity> &p_entities);

	OpenXRFbSpatialEntityBatch() = default;
	OpenXRFbSpatialEntityBatch(const TypedArray<OpenXRFbSpatialEntity> &p_entities);
};
} // namespace godot

#endif
