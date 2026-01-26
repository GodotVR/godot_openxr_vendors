/**************************************************************************/
/*  openxr_fb_scene_extension_wrapper.h                                   */
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
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook XR scene extension.
class OpenXRFbSceneExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSceneExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	bool is_scene_supported() {
		return fb_scene_ext;
	}

	static OpenXRFbSceneExtensionWrapper *get_singleton();

	OpenXRFbSceneExtensionWrapper();
	~OpenXRFbSceneExtensionWrapper();

	static const PackedStringArray &get_supported_semantic_labels();

	struct RoomLayout {
		XrUuidEXT floor;
		XrUuidEXT ceiling;
		Vector<XrUuidEXT> walls;
	};

	PackedStringArray get_semantic_labels(const XrSpace p_space);
	bool get_room_layout(const XrSpace p_space, RoomLayout &r_room_layout);
	Rect2 get_bounding_box_2d(const XrSpace p_space);
	AABB get_bounding_box_3d(const XrSpace p_space);
	PackedVector2Array get_boundary_2d(const XrSpace p_space);

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrGetSpaceBoundingBox2DFB,
			(XrSession), session,
			(XrSpace), space,
			(XrRect2Df *), boundingBox2DOutput)

	EXT_PROTO_XRRESULT_FUNC3(xrGetSpaceBoundingBox3DFB,
			(XrSession), session,
			(XrSpace), space,
			(XrRect3DfFB *), boundingBox3DOutput)

	EXT_PROTO_XRRESULT_FUNC3(xrGetSpaceSemanticLabelsFB,
			(XrSession), session,
			(XrSpace), space,
			(XrSemanticLabelsFB *), semanticLabelsOutput)

	EXT_PROTO_XRRESULT_FUNC3(xrGetSpaceBoundary2DFB,
			(XrSession), session,
			(XrSpace), space,
			(XrBoundary2DFB *), boundary2DOutput)

	EXT_PROTO_XRRESULT_FUNC3(xrGetSpaceRoomLayoutFB,
			(XrSession), session,
			(XrSpace), space,
			(XrRoomLayoutFB *), roomLayoutOutput)

	bool initialize_fb_scene_extension(const XrInstance instance);

	HashMap<String, bool *> request_extensions;

	void cleanup();

	static OpenXRFbSceneExtensionWrapper *singleton;

	bool fb_scene_ext = false;
};
