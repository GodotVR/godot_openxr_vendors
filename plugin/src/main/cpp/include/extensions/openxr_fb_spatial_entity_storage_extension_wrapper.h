/**************************************************************************/
/*  openxr_fb_spatial_entity_storage_extension_wrapper.h                    */
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

#ifndef OPENXR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_WRAPPER_H
#define OPENXR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook XR spatial entity storage extension.
class OpenXRFbSpatialEntityStorageExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntityStorageExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;
	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;
	virtual bool _on_event_polled(const void *event) override;

	bool is_spatial_entity_storage_supported() {
		return fb_spatial_entity_storage_ext;
	}

	static OpenXRFbSpatialEntityStorageExtensionWrapper *get_singleton();

	typedef void (*StorageRequestCompleteCallback)(XrResult p_result, XrSpaceStorageLocationFB p_location, void *p_userdata);

	bool save_space(const XrSpaceSaveInfoFB *p_info, StorageRequestCompleteCallback p_callback, void *p_userdata);
	bool erase_space(const XrSpaceEraseInfoFB *p_info, StorageRequestCompleteCallback p_callback, void *p_userdata);

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

	bool initialize_fb_spatial_entity_storage_extension(const XrInstance &instance);
	void on_space_save_complete(const XrEventDataSpaceSaveCompleteFB *event);
	void on_space_erase_complete(const XrEventDataSpaceEraseCompleteFB *event);

	HashMap<String, bool *> request_extensions;

	struct RequestInfo {
		StorageRequestCompleteCallback callback = nullptr;
		void *userdata = nullptr;

		RequestInfo() {}

		RequestInfo(StorageRequestCompleteCallback p_callback, void *p_userdata) {
			callback = p_callback;
			userdata = p_userdata;
		}
	};

	HashMap<XrAsyncRequestIdFB, RequestInfo> requests;

	void cleanup();

	static OpenXRFbSpatialEntityStorageExtensionWrapper *singleton;

	bool fb_spatial_entity_storage_ext = false;
};

#endif
