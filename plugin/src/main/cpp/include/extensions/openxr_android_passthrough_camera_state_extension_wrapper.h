/**************************************************************************/
/*  openxr_android_passthrough_camera_state_extension_wrapper.h           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#ifndef AndroidXR_PASSTHROUGH_CAMERA_STATE_EXTENSION_H
#define AndroidXR_PASSTHROUGH_CAMERA_STATE_EXTENSION_H

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>

#include "util.h"

using namespace godot;

class OpenXRAndroidPassthroughCameraStateExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRAndroidPassthroughCameraStateExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRAndroidPassthroughCameraStateExtensionWrapper *get_singleton();

	OpenXRAndroidPassthroughCameraStateExtensionWrapper();
	virtual ~OpenXRAndroidPassthroughCameraStateExtensionWrapper() override;

	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual void _on_instance_created(uint64_t p_instance) override;

	enum PassthroughCameraState {
		PASSTHROUGH_CAMERA_STATE_DISABLED,
		PASSTHROUGH_CAMERA_STATE_INITIALIZING,
		PASSTHROUGH_CAMERA_STATE_READY,
		PASSTHROUGH_CAMERA_STATE_ERROR
	};
	PassthroughCameraState get_passthrough_camera_state();

protected:
	static void _bind_methods();

private:
	static OpenXRAndroidPassthroughCameraStateExtensionWrapper *singleton;

	bool _initialize_androidxr_passthrough_camera_state_extension();

	HashMap<String, bool *> request_extensions;
	bool available = false;

	EXT_PROTO_XRRESULT_FUNC3(xrGetPassthroughCameraStateANDROID, (XrSession), session, (const XrPassthroughCameraStateGetInfoANDROID *), getInfo, (XrPassthroughCameraStateANDROID *), cameraStateOutput);
};

VARIANT_ENUM_CAST(OpenXRAndroidPassthroughCameraStateExtensionWrapper::PassthroughCameraState);

#endif // AndroidXR_PASSTHROUGH_CAMERA_STATE_EXTENSION_H
