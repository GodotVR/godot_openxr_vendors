/**************************************************************************/
/*  openxr_android_scene_meshing_extension_wrapper.h                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#ifndef OPENXR_ANDROID_SCENE_MESHING_EXTENSION_H
#define OPENXR_ANDROID_SCENE_MESHING_EXTENSION_H

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>

#include "classes/openxr_android_scene_meshing.h"

#include "util.h"

using namespace godot;

class OpenXRAndroidSceneMeshingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRAndroidSceneMeshingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRAndroidSceneMeshingExtensionWrapper *get_singleton();

	OpenXRAndroidSceneMeshingExtensionWrapper();
	virtual ~OpenXRAndroidSceneMeshingExtensionWrapper() override;

	virtual Dictionary _get_requested_extensions() override;
	virtual uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_session_created(uint64_t p_instance) override;

	Array get_supported_semantic_label_sets();

	EXT_PROTO_XRRESULT_FUNC3(xrCreateSceneMeshingTrackerANDROID, (XrSession), session, (const XrSceneMeshingTrackerCreateInfoANDROID *), createInfo, (XrSceneMeshingTrackerANDROID *), tracker);
	EXT_PROTO_XRRESULT_FUNC1(xrDestroySceneMeshSnapshotANDROID, (XrSceneMeshSnapshotANDROID), snapshot);
	EXT_PROTO_XRRESULT_FUNC1(xrDestroySceneMeshingTrackerANDROID, (XrSceneMeshingTrackerANDROID), tracker);
	EXT_PROTO_XRRESULT_FUNC3(xrCreateSceneMeshSnapshotANDROID, (XrSceneMeshingTrackerANDROID), tracker, (const XrSceneMeshSnapshotCreateInfoANDROID *), createInfo, (XrSceneMeshSnapshotCreationResultANDROID *), outSnapshotCreationResult);
	EXT_PROTO_XRRESULT_FUNC4(xrGetAllSubmeshStatesANDROID, (XrSceneMeshSnapshotANDROID), snapshot, (uint32_t), submeshStateCapacityInput, (uint32_t *), submeshStateCountOutput, (XrSceneSubmeshStateANDROID *), submeshStates);
	EXT_PROTO_XRRESULT_FUNC3(xrGetSubmeshDataANDROID, (XrSceneMeshSnapshotANDROID), snapshot, (uint32_t), submeshDataCount, (XrSceneSubmeshDataANDROID *), inoutSubmeshData);

protected:
	static void _bind_methods();

private:
	static OpenXRAndroidSceneMeshingExtensionWrapper *singleton;

	bool _initialize_androidxr_scene_meshing_extension();

	HashMap<String, bool *> request_extensions;
	bool available = false;
	XrSystemSceneMeshingPropertiesANDROID scene_meshing_properties;

	EXT_PROTO_XRRESULT_FUNC5(xrEnumerateSupportedSemanticLabelSetsANDROID, (XrInstance), instance, (XrSystemId), systemId, (uint32_t), supportedSemanticLabelSetsInputCapacity, (uint32_t *), supportedSemanticLabelSetsOutputCount, (XrSceneMeshSemanticLabelSetANDROID *), supportedSemanticLabelSets);
};

#endif // OPENXR_ANDROID_SCENE_MESHING_EXTENSION_H
