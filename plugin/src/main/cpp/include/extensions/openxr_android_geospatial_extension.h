/**************************************************************************/
/*  openxr_android_geospatial_extension.h                                 */
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
#include <godot_cpp/variant/transform3d.hpp>

#include "classes/openxr_android_geospatial_pose.h"
#include "util.h"

using namespace godot;

class OpenXRAndroidGeospatialExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidGeospatialExtension, OpenXRExtensionWrapper);

public:
	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;
	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	bool _on_event_polled(const void *p_event) override;

	static OpenXRAndroidGeospatialExtension *get_singleton();

	bool is_geospatial_supported() const;

	// @todo We could remove this in Godot 4.6+ because it can be done with JavaClassWrapper
	void get_geographic_location(const Callable &p_callback);

	enum VpsAvailability {
		VPS_AVAILABILITY_UNAVAILABLE,
		VPS_AVAILABILITY_AVAILABLE,
	};

	Ref<OpenXRFutureResult> check_vps_availability(double p_latitude, double p_longitude);

	void start_geospatial();
	void stop_geospatial();

	Ref<OpenXRAndroidGeospatialPose> transform_to_geospatial_pose(const Transform3D &p_transform);
	Transform3D geospatial_pose_to_transform(const Ref<OpenXRAndroidGeospatialPose> &p_pose);

	enum GeospatialState {
		GEOSPATIAL_STATE_STOPPED,
		GEOSPATIAL_STATE_RUNNING,
		GEOSPATIAL_STATE_INITIALIZATION_FAILED,
	};

	GeospatialState get_geospatial_state() const { return geospatial_state; };

	OpenXRAndroidGeospatialExtension();
	~OpenXRAndroidGeospatialExtension();

protected:
	static void _bind_methods();

private:
	bool initialize_android_geospatial_extension(const XrInstance &p_instance);
	void cleanup();

	static OpenXRAndroidGeospatialExtension *singleton;

	HashMap<String, bool *> request_extensions;

	bool android_geospatial_ext = false;

	XrSystemGeospatialPropertiesANDROID system_geospatial_properties{
		XR_TYPE_SYSTEM_GEOSPATIAL_PROPERTIES_ANDROID, // type
		nullptr, // next
		false // supportsGeospatial
	};

	XrGeospatialTrackerANDROID geospatial_tracker = XR_NULL_HANDLE;
	GeospatialState geospatial_state = GEOSPATIAL_STATE_STOPPED;

	void _on_check_vps_availability_complete(const Ref<OpenXRFutureResult> &p_future);

	EXT_PROTO_XRRESULT_FUNC3(xrCreateGeospatialTrackerANDROID,
			(XrSession), session,
			(const XrGeospatialTrackerCreateInfoANDROID *), createInfo,
			(XrGeospatialTrackerANDROID *), geospatialTrackerOutput)

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyGeospatialTrackerANDROID,
			(XrGeospatialTrackerANDROID), geospatialTracker)

	EXT_PROTO_XRRESULT_FUNC3(xrLocateGeospatialPoseFromPoseANDROID,
			(XrGeospatialTrackerANDROID), geospatialTracker,
			(const XrGeospatialPoseFromPoseLocateInfoANDROID *), locateInfo,
			(XrGeospatialPoseResultANDROID *), geospatialPoseResult)

	EXT_PROTO_XRRESULT_FUNC3(xrLocateGeospatialPoseANDROID,
			(XrGeospatialTrackerANDROID), geospatialTracker,
			(const XrGeospatialPoseLocateInfoANDROID *), locateInfo,
			(XrSpaceLocation *), location)

	EXT_PROTO_XRRESULT_FUNC4(xrCheckVpsAvailabilityAsyncANDROID,
			(XrSession), session,
			(double), latitude,
			(double), longitude,
			(XrFutureEXT *), future)

	EXT_PROTO_XRRESULT_FUNC3(xrCheckVpsAvailabilityCompleteANDROID,
			(XrSession), session,
			(XrFutureEXT), future,
			(XrVPSAvailabilityCheckCompletionANDROID *), completion)
};

VARIANT_ENUM_CAST(OpenXRAndroidGeospatialExtension::VpsAvailability);
VARIANT_ENUM_CAST(OpenXRAndroidGeospatialExtension::GeospatialState);
