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

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <openxr/openxr.h>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

#include <optional>
#include <map>

using namespace godot;

struct XrSceneObjectInternal {
	XrUuidEXT uuid;
	XrSpace space;
	std::optional<String> label;

	// Vertices and lines on a plane, probably use this for a floor / ceiling as they are irregularly shaped
	// We store a std::vector instead of XrBoundary2DFB to own the vertex memory
	std::optional<Vector<XrVector2f>> boundary2D;
	// A rectangle containing the whole thing, perfect for desks / tables / play surfaces
	std::optional<XrRect2Df> boundingBox2D;
	// 3D box for the whole thing, better for obstacles and other objects not used as a surface
	std::optional<XrRect3DfFB> boundingBox3D;
};

// Wrapper for the set of Facebook XR scene extension.
class OpenXRFbSceneExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSceneExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	bool is_scene_supported() {
		return fb_scene_ext;
	}

	static OpenXRFbSceneExtensionWrapper *get_singleton();

	OpenXRFbSceneExtensionWrapper();
	~OpenXRFbSceneExtensionWrapper();

	std::optional<String> get_semantic_labels(const XrSpace& space);
	void get_shapes(const XrSpace& space, XrSceneObjectInternal& object);

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
