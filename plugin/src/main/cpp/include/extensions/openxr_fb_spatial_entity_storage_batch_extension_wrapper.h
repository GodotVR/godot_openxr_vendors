/**************************************************************************/
/*  openxr_fb_spatial_entity_storage_batch_extension_wrapper.h            */
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

#include "util.h"

using namespace godot;

// Wrapper for XR_FB_spatial_entity_storage_batch extension.
class OpenXRFbSpatialEntityStorageBatchExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntityStorageBatchExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;

	bool is_spatial_entity_storage_batch_supported() {
		return fb_spatial_entity_storage_batch_ext;
	}

	typedef void (*SaveSpacesCompleteCallback)(XrResult p_result, XrSpaceStorageLocationFB p_location, void *p_userdata);

	bool save_spaces(const XrSpaceListSaveInfoFB *p_info, SaveSpacesCompleteCallback p_callback, void *p_userdata);

	virtual bool _on_event_polled(const void *event) override;

	static OpenXRFbSpatialEntityStorageBatchExtensionWrapper *get_singleton();

	OpenXRFbSpatialEntityStorageBatchExtensionWrapper();
	~OpenXRFbSpatialEntityStorageBatchExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrSaveSpaceListFB,
			(XrSession), session,
			(const XrSpaceListSaveInfoFB *), info,
			(XrAsyncRequestIdFB *), requestId);

	bool initialize_fb_spatial_entity_storage_batch_extension(const XrInstance &instance);
	void on_space_list_save_complete(const XrEventDataSpaceListSaveCompleteFB *event);

	HashMap<String, bool *> request_extensions;

	struct RequestInfo {
		SaveSpacesCompleteCallback callback = nullptr;
		void *userdata = nullptr;
		XrSpaceStorageLocationFB location = XR_SPACE_STORAGE_LOCATION_INVALID_FB;

		RequestInfo() { }

		RequestInfo(SaveSpacesCompleteCallback p_callback, void *p_userdata, XrSpaceStorageLocationFB p_location) {
			callback = p_callback;
			userdata = p_userdata;
			location = p_location;
		}
	};

	HashMap<XrAsyncRequestIdFB, RequestInfo> requests;

	void cleanup();

	static OpenXRFbSpatialEntityStorageBatchExtensionWrapper *singleton;

	bool fb_spatial_entity_storage_batch_ext = false;
};
