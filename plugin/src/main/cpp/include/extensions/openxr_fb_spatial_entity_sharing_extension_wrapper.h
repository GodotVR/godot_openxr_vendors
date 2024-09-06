/**************************************************************************/
/*  openxr_fb_spatial_entity_sharing_extension_wrapper.h                  */
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

// Wrapper for XR_FB_spatial_entity_sharing extension.
class OpenXRFbSpatialEntitySharingExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntitySharingExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;

	bool is_spatial_entity_sharing_supported() {
		return fb_spatial_entity_sharing_ext;
	}

	typedef void (*ShareSpacesCompleteCallback)(XrResult p_result, void *p_userdata);

	bool share_spaces(const XrSpaceShareInfoFB *p_info, ShareSpacesCompleteCallback p_callback, void *p_userdata);

	virtual bool _on_event_polled(const void *event) override;

	static OpenXRFbSpatialEntitySharingExtensionWrapper *get_singleton();

	OpenXRFbSpatialEntitySharingExtensionWrapper();
	~OpenXRFbSpatialEntitySharingExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrShareSpacesFB,
			(XrSession), session,
			(const XrSpaceShareInfoFB *), info,
			(XrAsyncRequestIdFB *), requestId);

	bool initialize_fb_spatial_entity_sharing_extension(const XrInstance &instance);
	void on_space_share_complete(const XrEventDataSpaceShareCompleteFB *event);

	HashMap<String, bool *> request_extensions;

	struct RequestInfo {
		ShareSpacesCompleteCallback callback = nullptr;
		void *userdata = nullptr;

		RequestInfo() { }

		RequestInfo(ShareSpacesCompleteCallback p_callback, void *p_userdata) {
			callback = p_callback;
			userdata = p_userdata;
		}
	};

	HashMap<XrAsyncRequestIdFB, RequestInfo> requests;

	void cleanup();

	static OpenXRFbSpatialEntitySharingExtensionWrapper *singleton;

	bool fb_spatial_entity_sharing_ext = false;
};
