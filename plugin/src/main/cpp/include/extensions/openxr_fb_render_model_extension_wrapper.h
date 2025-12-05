/**************************************************************************/
/*  openxr_fb_render_model_extension_wrapper.h                            */
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

#ifndef OPENXR_FB_RENDER_MODEL_EXTENSION_WRAPPER_H
#define OPENXR_FB_RENDER_MODEL_EXTENSION_WRAPPER_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook render model extension.
class OpenXRFbRenderModelExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbRenderModelExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	uint64_t _set_system_properties_and_get_next_pointer(void *next_pointer) override;

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	void _on_state_ready() override;

	void _on_state_stopping() override;

	static OpenXRFbRenderModelExtensionWrapper *get_singleton();

	bool is_enabled() const;
	bool is_openxr_session_active() const { return openxr_session_active; }
	PackedByteArray get_buffer(const String &p_path);

	OpenXRFbRenderModelExtensionWrapper();
	~OpenXRFbRenderModelExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC4(xrEnumerateRenderModelPathsFB,
			(XrSession), session,
			(uint32_t), pathCapacityInput,
			(uint32_t *), pathCountOutput,
			(XrRenderModelPathInfoFB *), path);

	EXT_PROTO_XRRESULT_FUNC3(xrGetRenderModelPropertiesFB,
			(XrSession), session,
			(XrPath), path,
			(XrRenderModelPropertiesFB *), properties);

	EXT_PROTO_XRRESULT_FUNC3(xrLoadRenderModelFB,
			(XrSession), session,
			(const XrRenderModelLoadInfoFB *), info,
			(XrRenderModelBufferFB *), buffer);

	EXT_PROTO_XRRESULT_FUNC3(xrStringToPath,
			(XrInstance), instance,
			(const char *), pathString,
			(XrPath *), path);

	bool initialize_fb_render_model_extension(const XrInstance instance);

	void cleanup();

	void fetch_paths();

	String _xr_path_to_string(XrPath p_path);

	XrPath _string_to_xr_path(const String &p_path);

	static OpenXRFbRenderModelExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool fb_render_model_ext = false;
	bool paths_fetched = false;
	bool openxr_session_active = false;
	XrSystemRenderModelPropertiesFB system_render_model_properties;
};

#endif // OPENXR_FB_RENDER_MODEL_EXTENSION_WRAPPER_H
