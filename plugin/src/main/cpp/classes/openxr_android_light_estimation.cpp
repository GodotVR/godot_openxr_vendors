/**************************************************************************/
/*  openxr_android_light_estimation.cpp                                   */
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

#include "classes/openxr_android_light_estimation.h"

#include <godot_cpp/classes/directional_light3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/sky.hpp>
#include <godot_cpp/classes/world_environment.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "extensions/openxr_android_light_estimation_extension_wrapper.h"

using namespace godot;

// Copied from ARCore's hello_ar_kotlin sample.
// See: https://github.com/google-ar/arcore-android-sdk/blob/52c722e43cd8ce546eea5dc4587e70e0c7f2c006/samples/hello_ar_kotlin/app/src/main/java/com/google/ar/core/examples/kotlin/helloar/HelloArRenderer.kt#L60
static constexpr float ANDROID_LIGHT_ESTIMATION_SH_FACTORS[9] = {
	0.282095,
	-0.325735,
	0.325735,
	-0.325735,
	0.273137,
	-0.273137,
	0.078848,
	-0.273137,
	0.136569,
};

static const char *ANDROID_LIGHT_ESTIMATION_SHADER = R"(
shader_type sky;
render_mode use_debanding;

uniform vec3 coefficients[9];
uniform mat3 rotation;

// Copied from ARCore's hello_ar_kotlin sample.
// See: https://github.com/google-ar/arcore-android-sdk/blob/52c722e43cd8ce546eea5dc4587e70e0c7f2c006/samples/hello_ar_kotlin/app/src/main/assets/shaders/environmental_hdr.frag#L132
vec3 applySH(vec3 dir, vec3 sh[9]) {
	vec3 radiance = sh[0] +
					sh[1] * (dir.y) +
					sh[2] * (dir.z) +
					sh[3] * (dir.x) +
					sh[4] * (dir.y * dir.x) +
					sh[5] * (dir.y * dir.z) +
					sh[6] * (3.0 * dir.z * dir.z - 1.0) +
					sh[7] * (dir.z * dir.x) +
					sh[8] * (dir.x * dir.x - dir.y * dir.y);
	return max(radiance, 0.0);
}

vec3 linear_to_srgb(vec3 color) {
	const vec3 a = vec3(0.055f);
	return mix((vec3(1.0f) + a) * pow(color.rgb, vec3(1.0f / 2.4f)) - a, 12.92f * color.rgb, lessThan(color.rgb, vec3(0.0031308f)));
}

void sky() {
	if (AT_CUBEMAP_PASS) {
		vec3 dir = rotation * EYEDIR;
		vec3 color = applySH(-dir, coefficients);
		color = max(color, vec3(0.0));
#if CURRENT_RENDERER == RENDERER_COMPATIBILITY
		COLOR = linear_to_srgb(color);
#else
		COLOR = color;
#endif
	} else {
		// Allows visualizing the radiance map.
		COLOR = texture(RADIANCE, EYEDIR).rgb;
	}
}
)";

void OpenXRAndroidLightEstimation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_directional_light", "directional_light"), &OpenXRAndroidLightEstimation::set_directional_light);
	ClassDB::bind_method(D_METHOD("get_directional_light"), &OpenXRAndroidLightEstimation::get_directional_light);

	ClassDB::bind_method(D_METHOD("set_directional_light_mode", "mode"), &OpenXRAndroidLightEstimation::set_directional_light_mode);
	ClassDB::bind_method(D_METHOD("get_directional_light_mode"), &OpenXRAndroidLightEstimation::get_directional_light_mode);

	ClassDB::bind_method(D_METHOD("set_world_environment", "world_environment"), &OpenXRAndroidLightEstimation::set_world_environment);
	ClassDB::bind_method(D_METHOD("get_world_environment"), &OpenXRAndroidLightEstimation::get_world_environment);

	ClassDB::bind_method(D_METHOD("set_ambient_light_mode", "mode"), &OpenXRAndroidLightEstimation::set_ambient_light_mode);
	ClassDB::bind_method(D_METHOD("get_ambient_light_mode"), &OpenXRAndroidLightEstimation::get_ambient_light_mode);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "directional_light", PROPERTY_HINT_NODE_TYPE, "DirectionalLight3D"), "set_directional_light", "get_directional_light");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "directional_light_mode", PROPERTY_HINT_ENUM, "Disabled,Direction Only,Direction + Intensity,Direction + Color + Intensity"), "set_directional_light_mode", "get_directional_light_mode");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_environment", PROPERTY_HINT_NODE_TYPE, "WorldEnvironment"), "set_world_environment", "get_world_environment");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ambient_light_mode", PROPERTY_HINT_ENUM, "Disabled,Color,Spherical Harmonics"), "set_ambient_light_mode", "get_ambient_light_mode");

	BIND_ENUM_CONSTANT(DIRECTIONAL_LIGHT_MODE_DISABLED);
	BIND_ENUM_CONSTANT(DIRECTIONAL_LIGHT_MODE_DIRECTION_ONLY);
	BIND_ENUM_CONSTANT(DIRECTIONAL_LIGHT_MODE_DIRECTION_INTENSITY);
	BIND_ENUM_CONSTANT(DIRECTIONAL_LIGHT_MODE_DIRECTION_COLOR_INTENSITY);

	BIND_ENUM_CONSTANT(AMBIENT_LIGHT_MODE_DISABLED);
	BIND_ENUM_CONSTANT(AMBIENT_LIGHT_MODE_COLOR);
	BIND_ENUM_CONSTANT(AMBIENT_LIGHT_MODE_SPHERICAL_HARMONICS);
	//BIND_ENUM_CONSTANT(AMBIENT_LIGHT_MODE_CUBEMAP);
}

void OpenXRAndroidLightEstimation::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			sky.instantiate();

			XRServer *xr_server = XRServer::get_singleton();
			if (xr_server) {
				Ref<OpenXRInterface> openxr_interface = xr_server->find_interface("OpenXR");
				if (openxr_interface.is_valid()) {
					openxr_interface->connect("session_begun", callable_mp(this, &OpenXRAndroidLightEstimation::start_or_stop));
				}
			}
		} break;

		case NOTIFICATION_PREDELETE: {
			XRServer *xr_server = XRServer::get_singleton();
			if (xr_server) {
				Ref<OpenXRInterface> openxr_interface = xr_server->find_interface("OpenXR");
				if (openxr_interface.is_valid()) {
					openxr_interface->disconnect("session_begun", callable_mp(this, &OpenXRAndroidLightEstimation::start_or_stop));
				}
			}
		} break;

		case NOTIFICATION_READY:
		case NOTIFICATION_VISIBILITY_CHANGED:
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_EXIT_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				start_or_stop();
			}
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			update_light_estimate();
		} break;
	}
}

void OpenXRAndroidLightEstimation::set_directional_light(DirectionalLight3D *p_directional_light) {
	if (p_directional_light) {
		directional_light_id = p_directional_light->get_instance_id();
		if (is_processing_internal()) {
			configure_light_estimate_types();
		}
	} else {
		directional_light_id = ObjectID();
	}
}

DirectionalLight3D *OpenXRAndroidLightEstimation::get_directional_light() const {
	return Object::cast_to<DirectionalLight3D>(ObjectDB::get_instance(directional_light_id));
}

void OpenXRAndroidLightEstimation::set_directional_light_mode(DirectionalLightMode p_directional_light_mode) {
	directional_light_mode = p_directional_light_mode;
	if (directional_light_mode != DIRECTIONAL_LIGHT_MODE_DISABLED && is_processing_internal()) {
		configure_light_estimate_types();
	}
}

OpenXRAndroidLightEstimation::DirectionalLightMode OpenXRAndroidLightEstimation::get_directional_light_mode() const {
	return directional_light_mode;
}

void OpenXRAndroidLightEstimation::set_world_environment(WorldEnvironment *p_world_environment) {
	reset_sky();
	if (p_world_environment) {
		world_environment_id = p_world_environment->get_instance_id();
		if (is_processing_internal()) {
			configure_light_estimate_types();
		}
	} else {
		world_environment_id = ObjectID();
	}
}

WorldEnvironment *OpenXRAndroidLightEstimation::get_world_environment() const {
	return Object::cast_to<WorldEnvironment>(ObjectDB::get_instance(world_environment_id));
}

void OpenXRAndroidLightEstimation::set_ambient_light_mode(AmbientLightMode p_ambient_light_mode) {
	ambient_light_mode = p_ambient_light_mode;
	if (ambient_light_mode != AMBIENT_LIGHT_MODE_DISABLED && is_processing_internal()) {
		configure_light_estimate_types();
	}
}

OpenXRAndroidLightEstimation::AmbientLightMode OpenXRAndroidLightEstimation::get_ambient_light_mode() const {
	return ambient_light_mode;
}

void OpenXRAndroidLightEstimation::start_or_stop() {
	OpenXRAndroidLightEstimationExtensionWrapper *light_estimation_extension = OpenXRAndroidLightEstimationExtensionWrapper::get_singleton();
	ERR_FAIL_NULL(light_estimation_extension);

	Ref<OpenXRAPIExtension> openxr_api = light_estimation_extension->get_openxr_api();
	if (openxr_api.is_null()) {
		return;
	}

	if (openxr_api->is_running() && is_inside_tree() && is_visible()) {
		bool light_estimation_started = light_estimation_extension->is_light_estimation_started();
		if (!light_estimation_started) {
			light_estimation_started = light_estimation_extension->start_light_estimation();
		}
		set_process_internal(light_estimation_started);
		if (light_estimation_started) {
			configure_light_estimate_types();
		}
	} else {
		if (light_estimation_extension->is_light_estimation_started()) {
			light_estimation_extension->stop_light_estimation();
		}
		set_process_internal(false);
	}
}

void OpenXRAndroidLightEstimation::configure_light_estimate_types() {
	OpenXRAndroidLightEstimationExtensionWrapper *light_estimation_extension = OpenXRAndroidLightEstimationExtensionWrapper::get_singleton();
	ERR_FAIL_NULL(light_estimation_extension);

	BitField<OpenXRAndroidLightEstimationExtensionWrapper::LightEstimateType> estimate_types = light_estimation_extension->get_light_estimate_types();

	if (get_directional_light() && directional_light_mode != DIRECTIONAL_LIGHT_MODE_DISABLED) {
		estimate_types.set_flag(OpenXRAndroidLightEstimationExtensionWrapper::LIGHT_ESTIMATE_TYPE_DIRECTIONAL_LIGHT);
	}

	if (get_world_environment() && ambient_light_mode != AMBIENT_LIGHT_MODE_DISABLED) {
		if (ambient_light_mode == AMBIENT_LIGHT_MODE_COLOR) {
			estimate_types.set_flag(OpenXRAndroidLightEstimationExtensionWrapper::LIGHT_ESTIMATE_TYPE_AMBIENT);
		} else if (ambient_light_mode == AMBIENT_LIGHT_MODE_SPHERICAL_HARMONICS) {
			estimate_types.set_flag(OpenXRAndroidLightEstimationExtensionWrapper::LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_TOTAL);
		}
	}

	light_estimation_extension->set_light_estimate_types(estimate_types);
}

void OpenXRAndroidLightEstimation::update_light_estimate() {
	OpenXRAndroidLightEstimationExtensionWrapper *light_estimation_extension = OpenXRAndroidLightEstimationExtensionWrapper::get_singleton();
	ERR_FAIL_NULL(light_estimation_extension);

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL(xr_server);

	if (!light_estimation_extension->is_estimate_valid()) {
		return;
	}

	int64_t estimate_time = light_estimation_extension->get_last_updated_time();
	if (estimate_time <= last_update_time) {
		return;
	}
	last_update_time = estimate_time;

	DirectionalLight3D *direction_light = get_directional_light();
	if (direction_light && directional_light_mode != DIRECTIONAL_LIGHT_MODE_DISABLED && light_estimation_extension->is_directional_light_valid()) {
		Basis light_basis = Basis::looking_at(-light_estimation_extension->get_directional_light_direction()) * xr_server->get_world_origin().basis;
		direction_light->set_global_basis(light_basis);

		if (directional_light_mode >= DIRECTIONAL_LIGHT_MODE_DIRECTION_INTENSITY) {
			Color intensity = light_estimation_extension->get_directional_light_intensity();

			if (directional_light_mode == DIRECTIONAL_LIGHT_MODE_DIRECTION_COLOR_INTENSITY) {
				// The color is premultiplied with intensity.
				direction_light->set_color(intensity.linear_to_srgb());
				direction_light->set_param(Light3D::PARAM_ENERGY, 1.0);
			} else {
				float luminance = (0.2126 * intensity.r) + (0.7152 * intensity.g) + (0.0722 * intensity.b);
				direction_light->set_param(Light3D::PARAM_ENERGY, luminance);
			}
		}
	}

	WorldEnvironment *world_environment = get_world_environment();
	Ref<Environment> env;
	if (world_environment) {
		env = world_environment->get_environment();
	}

	if (env.is_valid() && ambient_light_mode != AMBIENT_LIGHT_MODE_DISABLED) {
		if (ambient_light_mode == AMBIENT_LIGHT_MODE_COLOR && light_estimation_extension->is_ambient_light_valid()) {
			// The color is premultiplied with intensity.
			Color intensity = light_estimation_extension->get_ambient_light_intensity();
			env->set_ambient_light_color(intensity.linear_to_srgb());
			env->set_ambient_light_energy(1.0);
			env->set_ambient_source(Environment::AMBIENT_SOURCE_COLOR);
		} else if (ambient_light_mode == AMBIENT_LIGHT_MODE_SPHERICAL_HARMONICS && light_estimation_extension->is_spherical_harmonics_total_valid()) {
			PackedVector3Array coefficients = light_estimation_extension->get_spherical_harmonics_total_coefficients();
			Vector3 *coefficients_ptr = coefficients.ptrw();
			for (int i = 0; i < 9; i++) {
				coefficients_ptr[i] = coefficients_ptr[i] * ANDROID_LIGHT_ESTIMATION_SH_FACTORS[i];
			}

			if (sky_shader.is_null()) {
				sky_shader.instantiate();
				sky_shader->set_code(ANDROID_LIGHT_ESTIMATION_SHADER);
			}
			if (sky_material.is_null()) {
				sky_material.instantiate();
				sky_material->set_shader(sky_shader);
				sky->set_material(sky_material);
			}

			sky_material->set_shader_parameter("coefficients", coefficients);
			sky_material->set_shader_parameter("rotation", xr_server->get_world_origin().basis);
			if (env->get_sky() != sky) {
				if (old_sky.is_null()) {
					old_sky = env->get_sky();
				}
				env->set_sky(sky);
			}
			env->set_ambient_source(Environment::AMBIENT_SOURCE_SKY);
		}
	}
}

void OpenXRAndroidLightEstimation::reset_sky() {
	WorldEnvironment *old_world_environment = get_world_environment();
	if (old_world_environment) {
		Ref<Environment> env = old_world_environment->get_environment();
		if (env.is_valid() && env->get_sky() == sky) {
			env->set_sky(old_sky);
		}
	}
	old_sky.unref();
}
