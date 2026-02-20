/**************************************************************************/
/*  openxr_android_raycast_extension.h                                    */
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

#pragma once

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>

#include "classes/openxr_android_hit_result.h"
#include "util.h"

using namespace godot;

class OpenXRAndroidRaycastExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidRaycastExtension, OpenXRExtensionWrapper);

public:
	static OpenXRAndroidRaycastExtension *get_singleton();

	OpenXRAndroidRaycastExtension();
	virtual ~OpenXRAndroidRaycastExtension() override;

	virtual Dictionary _get_requested_extensions(uint64_t p_xr_version) override;
	virtual void _on_instance_created(uint64_t p_instance) override;
	virtual void _on_session_created(uint64_t p_session_instance) override;

	enum TrackableType {
		TRACKABLE_TYPE_PLANE,
		TRACKABLE_TYPE_DEPTH,
	};

	TypedArray<OpenXRAndroidHitResult> raycast(Array p_trackable_types, const Vector3 &p_origin, const Vector3 &p_trajectory, int p_max_results);
	bool is_raycast_supported() const;

protected:
	static void _bind_methods();

private:
	static OpenXRAndroidRaycastExtension *singleton;

	bool _initialize_androidxr_raycast_extension();

	HashMap<String, bool *> request_extensions;
	bool available = false;
	Vector<TrackableType> supported_trackable_types;

	EXT_PROTO_XRRESULT_FUNC5(xrEnumerateRaycastSupportedTrackableTypesANDROID, (XrInstance), instance, (XrSystemId), systemId, (uint32_t), trackableTypeCapacityInput, (uint32_t *), trackableTypeCountOutput, (XrTrackableTypeANDROID *), trackableTypes);
	EXT_PROTO_XRRESULT_FUNC3(xrRaycastANDROID, (XrSession), session, (const XrRaycastInfoANDROID *), rayInfo, (XrRaycastHitResultsANDROID *), results);
};

VARIANT_ENUM_CAST(OpenXRAndroidRaycastExtension::TrackableType);
