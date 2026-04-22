/**************************************************************************/
/*  openxr_session_helper_extension.h                                     */
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

#ifndef OPENXR_SESSION_HELPER_EXTENSION_H
#define OPENXR_SESSION_HELPER_EXTENSION_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include "util.h"

using namespace godot;

class OpenXRSessionHelperExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRSessionHelperExtension, OpenXRExtensionWrapper);

public:
	static OpenXRSessionHelperExtension *get_singleton();

	OpenXRSessionHelperExtension();
	virtual ~OpenXRSessionHelperExtension() override;

	virtual void _on_instance_created(uint64_t instance) override;
	virtual void _on_state_ready() override;

protected:
	static void _bind_methods();

private:
	// Vars for initializing extension
	static OpenXRSessionHelperExtension *singleton;

	bool _should_request_stop_xrs_client = false;

	String _get_openxr_runtime_name(XrInstance instance);

	EXT_PROTO_XRRESULT_FUNC2(xrGetInstanceProperties, (XrInstance), instance, (XrInstanceProperties *), instanceProperties);
};

#endif // OPENXR_SESSION_HELPER_EXTENSION_H
