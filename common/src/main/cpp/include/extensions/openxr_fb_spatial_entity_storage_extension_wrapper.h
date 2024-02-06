/**************************************************************************/
/*  openxr_fb_spatial_entity_storage_extension_wrapper.h                  */
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
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

#include <functional>
#include <map>

using namespace godot;

typedef std::function<void(const XrEventDataSpaceSaveCompleteFB* eventData)> SaveCompleteCallback_t;
typedef std::function<void(const XrEventDataSpaceEraseCompleteFB* eventData)> EraseCompleteCallback_t;

// Wrapper for the set of Facebook XR spatial entity storage extension.
class OpenXRFbSpatialEntityStorageExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntityStorageExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_process() override;

	bool is_spatial_entity_storage_supported() {
		return fb_spatial_entity_storage_ext;
	}

	virtual bool _on_event_polled(const void *event) override;

	static OpenXRFbSpatialEntityStorageExtensionWrapper *get_singleton();

	// Creates an XRPositionalTracker that represents the persistent anchor with the specified
	// uuid and names it the passed tracker_name
	void start_tracking_persistent_anchor(const String& uuid, const String& tracker_name);

	// Releases the XRPositionalTracker that represents the persistent anchor with the specified uuid
	void stop_tracking_persistent_anchor(const String& uuid);

	// Creates an anchor with the given transform relative to play_space. When ready, this gets
	// expressed as an XRPositionalTracker, whose name will be tracker_name.
	void create_persistent_anchor(const Transform3D &transform, const String& tracker_name);

	// Deletes a persistent anchor with the specified UUID. If there is an XRPositionalTracker
	// that represents it, that will be deleted too
	void delete_persistent_anchor(const String& uuid);

	OpenXRFbSpatialEntityStorageExtensionWrapper();
	~OpenXRFbSpatialEntityStorageExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrSaveSpaceFB,
			(XrSession), session,
			(const XrSpaceSaveInfoFB *), info,
			(XrAsyncRequestIdFB *), requestId)

	EXT_PROTO_XRRESULT_FUNC3(xrEraseSpaceFB,
			(XrSession), session,
			(const XrSpaceEraseInfoFB *), info,
			(XrAsyncRequestIdFB *), requestId)

	// CORE
	EXT_PROTO_XRRESULT_FUNC4(xrLocateSpace,
		(XrSpace), space,
		(XrSpace), baseSpace,
		(XrTime), time,
		(XrSpaceLocation*), location)

	void save_spatial_anchor(const XrSpaceSaveInfoFB* info, SaveCompleteCallback_t callback);
	void erase_spatial_anchor(const XrSpaceEraseInfoFB* info, EraseCompleteCallback_t callback);
	bool initialize_fb_spatial_entity_storage_extension(const XrInstance& instance);
	void on_save_complete(const XrEventDataSpaceSaveCompleteFB* event);
	void on_erase_complete(const XrEventDataSpaceEraseCompleteFB* event);

	HashMap<String, bool *> request_extensions;
	HashMap<XrAsyncRequestIdFB, SaveCompleteCallback_t> save_complete_callbacks;
	HashMap<XrAsyncRequestIdFB, EraseCompleteCallback_t> erase_complete_callbacks;

	HashMap<String, XrSpace> anchor_spaces;
	HashMap<String, Ref<XRPositionalTracker>> trackers;

	void cleanup();

	static OpenXRFbSpatialEntityStorageExtensionWrapper *singleton;

	bool fb_spatial_entity_storage_ext = false;
};
