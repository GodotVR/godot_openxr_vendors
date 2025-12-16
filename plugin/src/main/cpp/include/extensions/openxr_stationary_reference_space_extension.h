/**************************************************************************/
/*  openxr_stationary_reference_space_extension.h                         */
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

// @todo GH Issue 304: Remove check for meta headers when feature becomes part of OpenXR spec.
#ifdef META_HEADERS_ENABLED
#pragma once

#include <meta_openxr_preview/extx2_stationary_reference_space.h>
#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <map>

#include "util.h"

using namespace godot;

class OpenXRStationaryReferenceSpaceExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRStationaryReferenceSpaceExtension, OpenXRExtensionWrapper);

public:
	static OpenXRStationaryReferenceSpaceExtension *get_singleton();

	OpenXRStationaryReferenceSpaceExtension();
	~OpenXRStationaryReferenceSpaceExtension();

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;
	void _on_session_created(uint64_t p_instance) override;

	void set_stationary_reference_space_enabled(bool p_enable);

	StringName get_stationary_reference_space_uuid();

	bool is_stationary_reference_space_available();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateReferenceSpace,
			(XrSession), session,
			(const XrReferenceSpaceCreateInfo *), createInfo,
			(XrSpace *), space)

	EXT_PROTO_XRRESULT_FUNC1(xrDestroySpace,
			(XrSpace), space)

	EXT_PROTO_XRRESULT_FUNC3(xrGetStationaryReferenceSpaceIdEXTX2,
			(XrSession), session,
			(const XrStationaryReferenceSpaceIdGetInfoEXTX2 *), getInfo,
			(XrStationaryReferenceSpaceIdResultEXTX2 *), result)

	bool initialize_stationary_reference_space_extension(XrInstance p_instance);

	void cleanup();

	static OpenXRStationaryReferenceSpaceExtension *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool stationary_reference_space_ext = false;

	XrSpace stationary_space = XR_NULL_HANDLE;
};

#endif // META_HEADERS_ENABLED
