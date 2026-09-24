/**************************************************************************/
/*  openxr_meta_environment_raycast_extension.h                          */
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
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "util.h"

using namespace godot;

class OpenXRMetaEnvironmentRaycastExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRMetaEnvironmentRaycastExtension, OpenXRExtensionWrapper);

public:
	enum HitStatus {
		HIT_STATUS_HIT = XR_ENVIRONMENT_RAYCAST_HIT_STATUS_HIT_META,
		HIT_STATUS_NO_HIT = XR_ENVIRONMENT_RAYCAST_HIT_STATUS_NO_HIT_META,
		HIT_STATUS_HIT_POINT_OCCLUDED = XR_ENVIRONMENT_RAYCAST_HIT_STATUS_HIT_POINT_OCCLUDED_META,
		HIT_STATUS_HIT_POINT_OUTSIDE_OF_FOV = XR_ENVIRONMENT_RAYCAST_HIT_STATUS_HIT_POINT_OUTSIDE_OF_FOV_META,
		HIT_STATUS_RAY_OCCLUDED = XR_ENVIRONMENT_RAYCAST_HIT_STATUS_RAY_OCCLUDED_META,
		HIT_STATUS_HIT_INVALID_ORIENTATION = XR_ENVIRONMENT_RAYCAST_HIT_STATUS_HIT_INVALID_ORIENTATION_META,
	};

	static OpenXRMetaEnvironmentRaycastExtension *get_singleton();

	OpenXRMetaEnvironmentRaycastExtension();
	virtual ~OpenXRMetaEnvironmentRaycastExtension() override;

	// OpenXRExtensionWrapper interface
	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_instance_destroyed() override;
	virtual void _on_session_created(uint64_t p_session_instance) override;
	virtual void _on_session_destroyed() override;

	// API methods for GDScript
	bool is_environment_raycast_supported() const;
	bool is_raycaster_ready() const;
	bool create_raycaster();
	void destroy_raycaster();

	Dictionary raycast(const Vector3 &p_origin, const Vector3 &p_direction, float p_max_distance);

protected:
	static void _bind_methods();

private:
	static OpenXRMetaEnvironmentRaycastExtension *singleton;

	HashMap<String, bool *> request_extensions;
	bool meta_environment_raycast_ext = false;

	XrSystemEnvironmentRaycastPropertiesMETA system_raycast_properties = {
		XR_TYPE_SYSTEM_ENVIRONMENT_RAYCAST_PROPERTIES_META,
		nullptr,
		XR_FALSE,
	};

	XrEnvironmentRaycasterMETA environment_raycaster = XR_NULL_HANDLE;
	Ref<OpenXRFutureResult> raycaster_future;

	// Function prototypes for XR_META_environment_raycast
	EXT_PROTO_XRRESULT_FUNC3(xrCreateEnvironmentRaycasterAsyncMETA, (XrSession), session, (const XrEnvironmentRaycasterCreateInfoMETA *), info, (XrFutureEXT *), future);
	EXT_PROTO_XRRESULT_FUNC3(xrCreateEnvironmentRaycasterCompleteMETA, (XrSession), session, (XrFutureEXT), future, (XrEnvironmentRaycasterCreateCompletionMETA *), completion);
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyEnvironmentRaycasterMETA, (XrEnvironmentRaycasterMETA), environmentRaycaster);
	EXT_PROTO_XRRESULT_FUNC3(xrPerformEnvironmentRaycastMETA, (XrEnvironmentRaycasterMETA), environmentRaycaster, (const XrEnvironmentRaycastHitGetInfoMETA *), info, (XrEnvironmentRaycastHitMETA *), hitPoint);

	bool initialize_meta_environment_raycast_extension(const XrInstance &p_instance);
	void _on_raycaster_created(const Ref<OpenXRFutureResult> &p_future);
};

VARIANT_ENUM_CAST(OpenXRMetaEnvironmentRaycastExtension::HitStatus);
