/**************************************************************************/
/*  openxr_fb_scene_capture_extension_wrapper.h                           */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Original contributed implementation:                                   */
/*   Copyright (c) 2022-2023 MattaKis Consulting Kft. (Migeran)           */
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

#ifndef OPENXR_FB_SCENE_CAPTURE_EXTENSION_WRAPPER_H
#define OPENXR_FB_SCENE_CAPTURE_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

#include <map>

using namespace godot;

// Wrapper for the set of Facebook XR scene capture extension.
class OpenXRFbSceneCaptureExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSceneCaptureExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	bool is_scene_capture_supported() {
		return fb_scene_capture_ext;
	}

	typedef void (*SceneCaptureCompleteCallback)(XrResult p_result, void *p_userdata);

	bool request_scene_capture(String p_request, SceneCaptureCompleteCallback p_callback, void *p_userdata);
	bool is_scene_capture_enabled();

	// Supports legacy exposed API.
	bool _request_scene_capture_bind();

	virtual bool _on_event_polled(const void *event) override;

	static OpenXRFbSceneCaptureExtensionWrapper *get_singleton();

	OpenXRFbSceneCaptureExtensionWrapper();
	~OpenXRFbSceneCaptureExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrRequestSceneCaptureFB,
			(XrSession), session,
			(const XrSceneCaptureRequestInfoFB *), request,
			(XrAsyncRequestIdFB *), requestId)

	bool initialize_fb_scene_capture_extension(const XrInstance instance);
	void on_scene_capture_complete(const XrEventDataSceneCaptureCompleteFB *p_event);

	std::map<godot::String, bool *> request_extensions;

	struct RequestInfo {
		SceneCaptureCompleteCallback callback = nullptr;
		void *userdata = nullptr;

		RequestInfo(SceneCaptureCompleteCallback p_callback, void *p_userdata) {
			callback = p_callback;
			userdata = p_userdata;
		}

		RequestInfo() { }
	};

	HashMap<XrAsyncRequestIdFB, RequestInfo> requests;

	void cleanup();

	static OpenXRFbSceneCaptureExtensionWrapper *singleton;

	bool fb_scene_capture_ext = false;

	bool scene_capture_enabled = false;
};

#endif // OPENXR_FB_SCENE_CAPTURE_EXTENSION_WRAPPER_H
