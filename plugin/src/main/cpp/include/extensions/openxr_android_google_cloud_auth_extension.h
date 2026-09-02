/**************************************************************************/
/*  openxr_android_google_cloud_auth_extension.h                          */
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

#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/classes/open_xr_future_result.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

class OpenXRAndroidGoogleCloudAuthExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidGoogleCloudAuthExtension, OpenXRExtensionWrapper);

public:
	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;

	static OpenXRAndroidGoogleCloudAuthExtension *get_singleton();

	bool is_enabled() const;

	Ref<OpenXRFutureResult> set_google_cloud_auth_api_key(const String &p_api_key);
	Ref<OpenXRFutureResult> set_google_cloud_auth_token(const String &p_auth_token);
	Ref<OpenXRFutureResult> set_google_cloud_auth_keyless();

	bool is_authenticated() const;

	OpenXRAndroidGoogleCloudAuthExtension();
	~OpenXRAndroidGoogleCloudAuthExtension();

protected:
	static void _bind_methods();

private:
	bool initialize_android_google_cloud_auth_extension(const XrInstance &p_instance);
	void cleanup();

	static OpenXRAndroidGoogleCloudAuthExtension *singleton;

	HashMap<String, bool *> request_extensions;

	bool android_google_cloud_auth_ext = false;
	bool authenticated = false;

	void _on_auth_complete(const Ref<OpenXRFutureResult> &p_future);

	EXT_PROTO_XRRESULT_FUNC3(xrSetGoogleCloudAuthAsyncANDROID,
			(XrSession), session,
			(const XrGoogleCloudAuthInfoBaseHeaderANDROID *), authInfo,
			(XrFutureEXT *), future)

	EXT_PROTO_XRRESULT_FUNC3(xrSetGoogleCloudAuthCompleteANDROID,
			(XrSession), session,
			(XrFutureEXT), future,
			(XrFutureCompletionEXT *), completion)
};
