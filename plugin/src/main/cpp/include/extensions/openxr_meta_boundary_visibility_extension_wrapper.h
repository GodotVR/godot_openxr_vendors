/**************************************************************************/
/*  openxr_meta_boundary_visibility_extension_wrapper.h                   */
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
#ifndef OPENXR_META_BOUNDARY_VISIBILITY_EXTENSION_WRAPPER_H
#define OPENXR_META_BOUNDARY_VISIBILITY_EXTENSION_WRAPPER_H

#include <meta_openxr_preview/meta_boundary_visibility.h>
#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <map>

#include "util.h"

using namespace godot;

class OpenXRMetaBoundaryVisibilityExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMetaBoundaryVisibilityExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRMetaBoundaryVisibilityExtensionWrapper *get_singleton();

	OpenXRMetaBoundaryVisibilityExtensionWrapper();
	~OpenXRMetaBoundaryVisibilityExtensionWrapper();

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;
	bool _on_event_polled(const void *event) override;

	bool is_boundary_visibility_supported();

	void set_boundary_visible(bool p_visible);
	bool is_boundary_visible();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC2(xrRequestBoundaryVisibilityMETA,
			(XrSession), session,
			(XrBoundaryVisibilityMETA), boundaryVisibility)

	bool initialize_meta_boundary_visibility_extension(XrInstance p_instance);

	void cleanup();

	static OpenXRMetaBoundaryVisibilityExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool meta_boundary_visibility_ext = false;

	XrSystemBoundaryVisibilityPropertiesMETA boundary_visibility_properties = {
		XR_TYPE_SYSTEM_BOUNDARY_VISIBILITY_PROPERTIES_META, // type
		nullptr, // next
		false // supportsBoundaryVisibility
	};

	XrBoundaryVisibilityMETA current_boundary_visibility = XR_BOUNDARY_VISIBILITY_NOT_SUPPRESSED_META;
};

#endif // OPENXR_META_BOUNDARY_VISIBILITY_EXTENSION_WRAPPER_H
#endif // META_HEADERS_ENABLED
