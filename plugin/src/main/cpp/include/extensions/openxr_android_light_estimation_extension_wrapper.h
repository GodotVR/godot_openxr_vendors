/**************************************************************************/
/*  openxr_android_light_estimation_extension_wrapper.h                   */
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

#include <androidxr/androidxr.h>
#include <openxr/openxr.h>

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

class OpenXRAndroidLightEstimationExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRAndroidLightEstimationExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;

	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;

	static OpenXRAndroidLightEstimationExtensionWrapper *get_singleton();

	bool is_light_estimation_supported() {
		return android_light_estimation_ext && system_light_estimation_properties.supportsLightEstimation;
	}

	bool start_light_estimation();
	void stop_light_estimation();
	bool is_light_estimation_started();

	void _on_process() override;

	enum LightEstimateType {
		LIGHT_ESTIMATE_TYPE_DIRECTIONAL_LIGHT = 1,
		LIGHT_ESTIMATE_TYPE_AMBIENT = 2,
		LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_AMBIENT = 4,
		LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_TOTAL = 8,
		LIGHT_ESTIMATE_TYPE_CUBEMAP = 16,
		LIGHT_ESTIMATE_TYPE_ALL = 0x7fffffff,
	};

	void set_light_estimate_types(BitField<LightEstimateType> p_estimate_types);
	BitField<LightEstimateType> get_light_estimate_types() const;

	bool is_estimate_valid() const;
	int64_t get_last_updated_time() const;

	bool is_directional_light_valid() const;
	Color get_directional_light_intensity() const;
	Vector3 get_directional_light_direction() const;

	bool is_ambient_light_valid() const;
	Color get_ambient_light_intensity() const;
	Color get_ambient_light_color_correction() const;

	bool is_spherical_harmonics_ambient_valid() const;
	PackedVector3Array get_spherical_harmonics_ambient_coefficients() const;

	bool is_spherical_harmonics_total_valid() const;
	PackedVector3Array get_spherical_harmonics_total_coefficients() const;

	OpenXRAndroidLightEstimationExtensionWrapper();
	~OpenXRAndroidLightEstimationExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrCreateLightEstimatorANDROID,
			(XrSession), session,
			(XrLightEstimatorCreateInfoANDROID *), createinfo,
			(XrLightEstimatorANDROID *), outHandle)

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyLightEstimatorANDROID,
			(XrLightEstimatorANDROID), estimator)

	EXT_PROTO_XRRESULT_FUNC3(xrGetLightEstimateANDROID,
			(XrLightEstimatorANDROID), estimator,
			(const XrLightEstimateGetInfoANDROID *), input,
			(XrLightEstimateANDROID *), output)

	bool initialize_android_light_estimation_extension(const XrInstance &instance);
	void cleanup();

	void clear_light_info();

	static OpenXRAndroidLightEstimationExtensionWrapper *singleton;

	HashMap<String, bool *> request_extensions;
	bool android_light_estimation_ext = false;

	BitField<LightEstimateType> estimate_types = 0;
	XrLightEstimatorANDROID light_estimator = XR_NULL_HANDLE;

	XrSystemLightEstimationPropertiesANDROID system_light_estimation_properties = {
		XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_ANDROID, // type
		nullptr, // next
		XR_FALSE, // supportsLightEstimation
	};

	XrLightEstimateANDROID estimate_info = {
		XR_TYPE_LIGHT_ESTIMATE_ANDROID, // type
		nullptr, // next
		XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID, // state
		0, // lastUpdatedTime
	};

	XrDirectionalLightANDROID directional_light_info = {
		XR_TYPE_DIRECTIONAL_LIGHT_ANDROID, // type
		nullptr, // next
		XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID, // state
		XrVector3f{ 0.0, 0.0, 0.0 }, // intensity
		XrVector3f{ 0.0, 0.0, 0.0 }, // direction
	};

	XrAmbientLightANDROID ambient_light_info = {
		XR_TYPE_AMBIENT_LIGHT_ANDROID, // type
		nullptr, // next
		XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID, // state
		XrVector3f{ 0.0, 0.0, 0.0 }, // intensity
		XrVector3f{ 0.0, 0.0, 0.0 }, // colorCorrection
	};

	XrSphericalHarmonicsANDROID spherical_harmonics_ambient_info = {
		XR_TYPE_SPHERICAL_HARMONICS_ANDROID, // type
		nullptr, // next
		XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID, // state
		XR_SPHERICAL_HARMONICS_KIND_AMBIENT_ANDROID, // kind
		{}, // coefficients
	};

	XrSphericalHarmonicsANDROID spherical_harmonics_total_info = {
		XR_TYPE_SPHERICAL_HARMONICS_ANDROID, // type
		nullptr, // next
		XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID, // state
		XR_SPHERICAL_HARMONICS_KIND_TOTAL_ANDROID, // kind
		{}, // coefficients
	};
};

VARIANT_BITFIELD_CAST(OpenXRAndroidLightEstimationExtensionWrapper::LightEstimateType);
