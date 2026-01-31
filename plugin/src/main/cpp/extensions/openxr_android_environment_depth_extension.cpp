/**************************************************************************/
/*  openxr_android_environment_depth_extension_wrapper.cpp                */
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

#include "extensions/openxr_android_environment_depth_extension.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *ANDROID_ENVIRONMENT_DEPTH_AVAILABLE_NAME = "ANDROID_ENVIRONMENT_DEPTH_AVAILABLE";
static const char *ANDROID_ENVIRONMENT_DEPTH_TEXTURE_NAME = "ANDROID_ENVIRONMENT_DEPTH_TEXTURE";
static const char *ANDROID_ENVIRONMENT_DEPTH_RESOLUTION_NAME = "ANDROID_ENVIRONMENT_DEPTH_RESOLUTION";
static const char *ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT_NAME = "ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT";
static const char *ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT_NAME = "ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT";
static const char *ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT_NAME = "ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT";
static const char *ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT_NAME = "ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT";

static const char *ANDROID_ENVIRONMENT_DEPTH_REPROJECTION_SHADER_CODE = R"(
shader_type spatial;
render_mode unshaded, world_vertex_coords, shadow_to_opacity, shadows_disabled, cull_disabled, depth_draw_always;
global uniform highp sampler2DArray ANDROID_ENVIRONMENT_DEPTH_TEXTURE;
global uniform int ANDROID_ENVIRONMENT_DEPTH_RESOLUTION;
global uniform highp vec4 ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT;
global uniform highp vec4 ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT;
global uniform highp mat4 ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT;
global uniform highp mat4 ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT;

//DEFINES
#ifdef USE_DEPTH_OFFSET_SCALE
uniform highp float depth_offset_scale = 0.0;
#ifdef USE_DEPTH_OFFSET_EXPONENT
uniform highp float depth_offset_exponent = 1.0;
#endif // USE_DEPTH_OFFSET_EXPONENT
#endif // USE_DEPTH_OFFSET_SCALE
void vertex() {
	highp vec4 tanfov = VIEW_INDEX == VIEW_MONO_LEFT ? ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT : ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT;
	highp mat4 depth_to_world = VIEW_INDEX == VIEW_MONO_LEFT ? ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT : ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT;
	highp vec2 uv = vec2((float(VERTEX_ID % ANDROID_ENVIRONMENT_DEPTH_RESOLUTION) + 0.5) / float(ANDROID_ENVIRONMENT_DEPTH_RESOLUTION), (float(VERTEX_ID / ANDROID_ENVIRONMENT_DEPTH_RESOLUTION) + 0.5) / float(ANDROID_ENVIRONMENT_DEPTH_RESOLUTION));

	// Depth texel corresponds to top left so we need to flip v.
	highp vec2 uvFlipped = vec2(uv.x, 1.0 - uv.y);

	highp float depth = texture(ANDROID_ENVIRONMENT_DEPTH_TEXTURE, vec3(uvFlipped, float(VIEW_INDEX))).x;

	// The depth camera's near plane at z=-1 is parameterized by
	// z = -1
	// x = lerp(tanL, tanR, u)
	// y = lerp(tanB, tanT, v)
	highp vec3 depth_pose = depth * vec3(mix(tanfov.xz, tanfov.yw, uv), -1.0);

	VERTEX = (depth_to_world * vec4(depth_pose, 1.0)).xyz;
	POINT_SIZE = 20.0;
}
void fragment() {
	// VERTEX is automatically transformed from world to view space; find the clip space coordinate
	highp vec4 clip_back = PROJECTION_MATRIX * vec4(VERTEX, 1.0);

	if (clip_back.w <= 0.1) {
		discard;
	}

#ifdef USE_DEPTH_OFFSET_SCALE
#ifdef USE_DEPTH_OFFSET_EXPONENT
	highp float z_adjustment = pow(abs(clip_back.w), depth_offset_exponent) * depth_offset_scale;
#else
	highp float z_adjustment = abs(clip_back.w) * depth_offset_scale;
#endif // USE_DEPTH_OFFSET_EXPONENT
	clip_back.z = clamp(clip_back.z - z_adjustment, -clip_back.w, clip_back.w);
#endif // USE_DEPTH_OFFSET_SCALE
	highp float camera_ndc_z = clip_back.z / clip_back.w;
#if CURRENT_RENDERER == RENDERER_COMPATIBILITY
	highp float camera_depth = camera_ndc_z * 0.5 + 0.5;
#else
	highp float camera_depth = camera_ndc_z;
#endif
	ALBEDO = vec3(0.0);
	DEPTH = camera_depth;
}
)";

OpenXRAndroidEnvironmentDepthExtension *OpenXRAndroidEnvironmentDepthExtension::singleton = nullptr;

OpenXRAndroidEnvironmentDepthExtension *OpenXRAndroidEnvironmentDepthExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidEnvironmentDepthExtension());
	}
	return singleton;
}

OpenXRAndroidEnvironmentDepthExtension::OpenXRAndroidEnvironmentDepthExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidEnvironmentDepthExtension singleton already exists.");

	request_extensions[XR_ANDROID_DEPTH_TEXTURE_EXTENSION_NAME] = &android_environment_depth_ext;
	singleton = this;
}

OpenXRAndroidEnvironmentDepthExtension::~OpenXRAndroidEnvironmentDepthExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRAndroidEnvironmentDepthExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_environment_depth_supported"), &OpenXRAndroidEnvironmentDepthExtension::is_environment_depth_supported);

	ClassDB::bind_method(D_METHOD("start_environment_depth"), &OpenXRAndroidEnvironmentDepthExtension::start_environment_depth);
	ClassDB::bind_method(D_METHOD("stop_environment_depth"), &OpenXRAndroidEnvironmentDepthExtension::stop_environment_depth);
	ClassDB::bind_method(D_METHOD("is_environment_depth_started"), &OpenXRAndroidEnvironmentDepthExtension::is_environment_depth_started);

	ClassDB::bind_method(D_METHOD("get_supported_resolutions"), &OpenXRAndroidEnvironmentDepthExtension::get_supported_resolutions);
	ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &OpenXRAndroidEnvironmentDepthExtension::set_resolution);

	ClassDB::bind_method(D_METHOD("set_smooth", "smooth"), &OpenXRAndroidEnvironmentDepthExtension::set_smooth);

	ADD_SIGNAL(MethodInfo("openxr_android_environment_depth_started"));
	ADD_SIGNAL(MethodInfo("openxr_android_environment_depth_stopped"));

	BIND_ENUM_CONSTANT(DEPTH_CAMERA_RESOLUTION_80x80);
	BIND_ENUM_CONSTANT(DEPTH_CAMERA_RESOLUTION_160x160);
	BIND_ENUM_CONSTANT(DEPTH_CAMERA_RESOLUTION_320x320);
}

Dictionary OpenXRAndroidEnvironmentDepthExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRAndroidEnvironmentDepthExtension::_on_instance_created(uint64_t instance) {
	if (android_environment_depth_ext) {
		bool result = initialize_android_environment_depth_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize XR_ANDROID_depth_texture extension");
			android_environment_depth_ext = false;
		}
	}
}

void OpenXRAndroidEnvironmentDepthExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRAndroidEnvironmentDepthExtension::_on_session_created(uint64_t instance) {
	if (!is_environment_depth_supported()) {
		return;
	}

	uint32_t resolution_count_output = 0;
	XrResult result = xrEnumerateDepthResolutionsANDROID(SESSION, 0, &resolution_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate depth resolutions; ", get_openxr_api()->get_error_string(result));
		android_environment_depth_ext = false;
		return;
	}

	if (resolution_count_output == 0) {
		WARN_PRINT("OpenXR: found zero depth resolutions");
		android_environment_depth_ext = false;
		return;
	}

	LocalVector<XrDepthCameraResolutionANDROID> resolutions{};
	resolutions.resize(resolution_count_output);
	result = xrEnumerateDepthResolutionsANDROID(SESSION, resolution_count_output, &resolution_count_output, resolutions.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate depth resolutions; ", get_openxr_api()->get_error_string(result));
		android_environment_depth_ext = false;
		return;
	}

	if (resolutions.size() != resolution_count_output) {
		WARN_PRINT("OpenXR: enumerate depth resolutions returned a different count");
		if (resolutions.size() < resolution_count_output) {
			UtilityFunctions::printerr("OpenXR: somehow received more depth resolutions on the second query; ", get_openxr_api()->get_error_string(result));
			android_environment_depth_ext = false;
			return;
		}

		if (resolution_count_output == 0) {
			UtilityFunctions::printerr("OpenXR: second enumerate depth resolutions returned zero");
			android_environment_depth_ext = false;
			return;
		}

		resolutions.resize(resolution_count_output);
	}

	for (const XrDepthCameraResolutionANDROID supported_resolution : resolutions) {
		switch (supported_resolution) {
			case XR_DEPTH_CAMERA_RESOLUTION_80x80_ANDROID:
				supported_resolutions.insert(DEPTH_CAMERA_RESOLUTION_80x80);
				break;
			case XR_DEPTH_CAMERA_RESOLUTION_160x160_ANDROID:
				supported_resolutions.insert(DEPTH_CAMERA_RESOLUTION_160x160);
				break;
			case XR_DEPTH_CAMERA_RESOLUTION_320x320_ANDROID:
				supported_resolutions.insert(DEPTH_CAMERA_RESOLUTION_320x320);
				break;
			case XR_DEPTH_CAMERA_RESOLUTION_MAX_ENUM_ANDROID:
			default:
				UtilityFunctions::printerr("OpenXR: unexpected resolution enum value: ", supported_resolution);
				break;
		}
	}

	android_environment_depth_ext = !supported_resolutions.is_empty();
}

void OpenXRAndroidEnvironmentDepthExtension::_on_session_destroyed() {
	destroy_depth_provider();
}

void OpenXRAndroidEnvironmentDepthExtension::_on_pre_render() {
#ifndef ANDROID_ENABLED
	return;
#endif

	if (!depth_provider_started || supported_resolutions.is_empty()) {
		return;
	}

	// NOTE: don't call _ensure_depth_swapchain_is_created() since there's no immediate feedback to
	// the game dev that a particular resolution or smoothness is unsupported.
	// This code should only be reached with a valid resolution and smooth (though sanity check
	// just in case).
	// See also set_resolution() and set_smooth().
	ERR_FAIL_COND(depth_camera_data.swapchain == XR_NULL_HANDLE);

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (unlikely(reprojection_material_dirty)) {
		update_reprojection_material();
	}

	rs->global_shader_parameter_set(ANDROID_ENVIRONMENT_DEPTH_AVAILABLE_NAME, false);
	rs->global_shader_parameter_set(ANDROID_ENVIRONMENT_DEPTH_TEXTURE_NAME, RID());

	XrDepthAcquireInfoANDROID acquire_info = {
		XR_TYPE_DEPTH_ACQUIRE_INFO_ANDROID, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // space
		(XrTime)get_openxr_api()->get_predicted_display_time(), // displayTime
	};

	XrDepthAcquireResultANDROID acquire_result{
		XR_TYPE_DEPTH_ACQUIRE_RESULT_ANDROID, // type
		nullptr, // next
		0, // acquiredIndex
		0, // exposureTimestamp
		{
				{
						XR_TYPE_DEPTH_VIEW_ANDROID, // type
						nullptr, // next
						{}, // fov
						{}, // pose
				},
				{
						XR_TYPE_DEPTH_VIEW_ANDROID, // type
						nullptr, // next
						{}, // fov
						{}, // pose
				} }, // views[2]
	};
	XrResult result = xrAcquireDepthSwapchainImagesANDROID(depth_camera_data.swapchain, &acquire_info, &acquire_result);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: unable to acquire depth swapchain images; ", get_openxr_api()->get_error_string(result));
		return;
	}

	const float *image_data_raw = smooth ? depth_camera_data.xr_images[acquire_result.acquiredIndex].smoothDepthImage : depth_camera_data.xr_images[acquire_result.acquiredIndex].rawDepthImage;
	if (image_data_raw == nullptr) {
		UtilityFunctions::printerr("OpenXR: unable to acquire depth swapchain images; no raw depth image");
		return;
	}

	DepthCameraData::Cache &cache = depth_camera_data.godot_texture_cache[acquire_result.acquiredIndex];
	if (unlikely(!cache.rid.is_valid())) {
		int res = 0;
		switch (resolution) {
			case DEPTH_CAMERA_RESOLUTION_80x80:
				res = 80;
				break;
			case DEPTH_CAMERA_RESOLUTION_160x160:
				res = 160;
				break;
			case DEPTH_CAMERA_RESOLUTION_320x320:
				res = 320;
				break;
			default:
				// this should never happen
				UtilityFunctions::printerr("OpenXR: invalid DepthCameraResolution; ", resolution);
				return;
		}

		cache.images[0] = Image::create_empty(res, res, false, Image::FORMAT_RF);
		cache.images[1] = Image::create_empty(res, res, false, Image::FORMAT_RF);

		TypedArray<Image> layers;
		layers.push_back(cache.images[0]);
		layers.push_back(cache.images[1]);
		cache.rid = rs->texture_2d_layered_create(layers, RenderingServer::TEXTURE_LAYERED_2D_ARRAY);
	}
	ERR_FAIL_COND(!cache.rid.is_valid());

	rs->global_shader_parameter_set(ANDROID_ENVIRONMENT_DEPTH_AVAILABLE_NAME, true);
	rs->global_shader_parameter_set(ANDROID_ENVIRONMENT_DEPTH_TEXTURE_NAME, cache.rid);

	for (int i = 0; i < 2; ++i) {
		const Ref<Image> &image = cache.images[i];
		int image_width = image->get_width();
		int image_height = image->get_height();
		bool image_mipmaps = image->has_mipmaps();
		Image::Format image_format = image->get_format();
		int image_size_bytes = image->get_data_size();

		image_data_cache.resize(image_size_bytes);
		memcpy(image_data_cache.ptrw(), image_data_raw + image_width * image_height * i, image_size_bytes);
		image->set_data(image_width, image_height, image_mipmaps, image_format, image_data_cache);
		rs->texture_2d_update(cache.rid, image, i);

		Vector4 tan_fov = Vector4(
				tan(acquire_result.views[i].fov.angleLeft),
				tan(acquire_result.views[i].fov.angleRight),
				tan(acquire_result.views[i].fov.angleDown),
				tan(acquire_result.views[i].fov.angleUp));
		rs->global_shader_parameter_set(i == 0 ? ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT_NAME : ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT_NAME, tan_fov);

		Transform3D camera_to_world;
		const XrPosef &pose = acquire_result.views[i].pose;
		camera_to_world.origin.x = pose.position.x;
		camera_to_world.origin.y = pose.position.y;
		camera_to_world.origin.z = pose.position.z;
		camera_to_world.basis = Basis{ Quaternion{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w } };
		rs->global_shader_parameter_set(i == 0 ? ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT_NAME : ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT_NAME, camera_to_world);
	}
}

uint64_t OpenXRAndroidEnvironmentDepthExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (android_environment_depth_ext) {
		system_depth_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_depth_properties);
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

bool OpenXRAndroidEnvironmentDepthExtension::start_environment_depth() {
	if (depth_provider_started) {
		return false;
	}

	if (!is_environment_depth_supported()) {
		WARN_PRINT("OpenXR: unable to start_environment_depth(); unsupported");
		return false;
	}

	if (!set_resolution(resolution)) {
		if (supported_resolutions.is_empty()) {
			WARN_PRINT("OpenXR: unable to start_environment_depth(); zero supported resolutions");
			return false;
		}

		// The default resolution is unsupported. Attempt to find a supported resolution.
		// (this resolution should be the default resolution specified in the header, since it's not
		// possible to set an unsupported resolution (see also set_resolution())).
		bool found_supported_resolution = false;
		DepthCameraResolution last_resolution = resolution;
		for (const DepthCameraResolution supported_resolution : supported_resolutions) {
			if (set_resolution(supported_resolution)) {
				WARN_PRINT(vformat("OpenXR: using supported resolution [%d] since [%d] is unsupported", supported_resolution, last_resolution));
				found_supported_resolution = true;
				break;
			}
		}

		if (!found_supported_resolution) {
			// This is an unrecoverable error.
			// This can only happen when the app doesn't have the required permissions (likely missing
			// android.permission.SCENE_UNDERSTANDING_FINE), since we have non-zero supported resolutions
			// but we cannot create a swapchain from any of them
			UtilityFunctions::printerr("OpenXR: Unable to create a swapchain from one of the supported resolutions; try granting this app android.permission.SCENE_UNDERSTANDING_FINE");
			destroy_depth_provider();
			return false;
		}
	}

	setup_global_uniforms();

	depth_provider_started = true;
	emit_signal("openxr_android_environment_depth_started");
	return true;
}

void OpenXRAndroidEnvironmentDepthExtension::stop_environment_depth() {
	// Always clear depth_camera_data, even when depth_provider_started is false, since
	// set_resolution() and set_smooth() can allocate when depth_provider_started is false too.
	// This provides the user the capability of "undoing" the allocations.
	depth_camera_data.reset();

	if (!depth_provider_started) {
		return;
	}

	depth_provider_started = false;
	emit_signal("openxr_android_environment_depth_stopped");
}

bool OpenXRAndroidEnvironmentDepthExtension::is_environment_depth_started() {
	return depth_provider_started;
}

Array OpenXRAndroidEnvironmentDepthExtension::get_supported_resolutions() const {
	Array ret;
	for (const DepthCameraResolution supported_resolution : supported_resolutions) {
		ret.push_back(supported_resolution);
	}

	return ret;
}

bool OpenXRAndroidEnvironmentDepthExtension::set_resolution(DepthCameraResolution p_resolution) {
	// NOTE: This can create a new swapchain even when depth_provider_started is false.
	// This is okay, since...
	// 1: the swapchain may fail to be created (due to missing
	//    android.permission.SCENE_UNDERSTANDING_FINE), even when the resolution is supported
	// 2: because of #1, we have to attempt to create the swapchain to determine if the new resolution
	//    is completely supported.
	// The only downside is that this will allocate a swapchain, rid, and Ref<Image>s that may not be
	// used later (until stop_environment_depth() is called)
	if (!_ensure_depth_swapchain_is_created(p_resolution, smooth)) {
		UtilityFunctions::printerr(vformat("OpenXR: unable to set_resolution; the combination of %s and %s is not supported", p_resolution, smooth));
		return false;
	}

	resolution = p_resolution;
	return true;
}

bool OpenXRAndroidEnvironmentDepthExtension::set_smooth(bool p_smooth) {
	// See "NOTE:" in set_resolution()
	if (!_ensure_depth_swapchain_is_created(resolution, p_smooth)) {
		UtilityFunctions::printerr(vformat("OpenXR: unable to set_smooth; the combination of %s and %s is not supported", resolution, p_smooth));
		return false;
	}

	smooth = p_smooth;
	return true;
}

RID OpenXRAndroidEnvironmentDepthExtension::get_reprojection_mesh() {
	if (reprojection_mesh.is_null()) {
		reprojection_shader.instantiate();
		reprojection_material.instantiate();

		update_reprojection_material(true);

		reprojection_material->set_render_priority(reprojection_render_priority);

		_update_mesh();
	}

	return reprojection_mesh->get_rid();
}

void OpenXRAndroidEnvironmentDepthExtension::update_reprojection_material(bool p_creation) {
	if (reprojection_shader.is_null() || reprojection_material.is_null()) {
		return;
	}

	String shader_code = ANDROID_ENVIRONMENT_DEPTH_REPROJECTION_SHADER_CODE;
	PackedStringArray defines;

	if (reprojection_offset_scale != 0.0) {
		defines.append("#define USE_DEPTH_OFFSET_SCALE");
	}
	if (reprojection_offset_exponent != 1.0) {
		defines.append("#define USE_DEPTH_OFFSET_EXPONENT");
	}

	shader_code = shader_code.replace("//DEFINES", String("\n").join(defines));

	reprojection_shader->set_code(shader_code);

	if (p_creation) {
		reprojection_material->set_shader(reprojection_shader);
	}

	if (reprojection_offset_scale != 0.0) {
		reprojection_material->set_shader_parameter("depth_offset_scale", reprojection_offset_scale);
	}
	if (reprojection_offset_exponent != 1.0) {
		reprojection_material->set_shader_parameter("depth_offset_exponent", reprojection_offset_exponent);
	}

	reprojection_material_dirty = false;
}

void OpenXRAndroidEnvironmentDepthExtension::set_reprojection_render_priority(int p_render_priority) {
	reprojection_render_priority = p_render_priority;

	if (reprojection_material.is_valid()) {
		reprojection_material->set_render_priority(p_render_priority);
	}
}

int OpenXRAndroidEnvironmentDepthExtension::get_reprojection_render_priority() const {
	return reprojection_render_priority;
}

void OpenXRAndroidEnvironmentDepthExtension::set_reprojection_offset_scale(float p_offset_scale) {
	reprojection_offset_scale = p_offset_scale;
	reprojection_material_dirty = true;
}

float OpenXRAndroidEnvironmentDepthExtension::get_reprojection_offset_scale() const {
	return reprojection_offset_scale;
}

void OpenXRAndroidEnvironmentDepthExtension::set_reprojection_offset_exponent(float p_offset_exponent) {
	reprojection_offset_exponent = p_offset_exponent;
	reprojection_material_dirty = true;
}

float OpenXRAndroidEnvironmentDepthExtension::get_reprojection_offset_exponent() const {
	return reprojection_offset_exponent;
}

static void create_shader_global_uniform(const String &p_name, RenderingServer::GlobalShaderParameterType p_type, Variant p_value, RenderingServer *p_rendering_server, ProjectSettings *p_project_settings, bool p_is_editor) {
	String setting_name = "shader_globals/" + p_name;
	if (!p_project_settings->has_setting(setting_name)) {
		p_rendering_server->global_shader_parameter_add(p_name, p_type, p_value);
		if (p_is_editor) {
			String type_name;
			switch (p_type) {
				case RenderingServer::GLOBAL_VAR_TYPE_BOOL: {
					type_name = "bool";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_INT: {
					type_name = "int";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY: {
					type_name = "sampler2DArray";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_MAT4: {
					type_name = "mat4";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_VEC4: {
					type_name = "vec4";
				} break;
			}

			Variant setting_value = p_value;
			if (p_type == RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY) {
				// In ProjectSettings, this uses a path as a value.
				setting_value = "";
			}

			Dictionary d;
			d["type"] = type_name;
			d["value"] = setting_value;
			p_project_settings->set(setting_name, d);
		}
	}
}

static void remove_shader_global_uniform(const String &p_name, RenderingServer *p_rendering_server, ProjectSettings *p_project_settings) {
	String setting_name = "shader_globals/" + p_name;
	if (p_project_settings->has_setting(setting_name)) {
		p_rendering_server->global_shader_parameter_remove(p_name);
		p_project_settings->clear(setting_name);
	}
}

void OpenXRAndroidEnvironmentDepthExtension::setup_global_uniforms() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	bool enabled = project_settings->get_setting_with_override("xr/openxr/extensions/androidxr/environment_depth");
	if (!enabled) {
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_AVAILABLE_NAME, rs, project_settings);
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_TEXTURE_NAME, rs, project_settings);
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_RESOLUTION_NAME, rs, project_settings);
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT_NAME, rs, project_settings);
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT_NAME, rs, project_settings);
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT_NAME, rs, project_settings);
		remove_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT_NAME, rs, project_settings);

		already_setup_global_uniforms = false;
		return;
	}

	if (already_setup_global_uniforms) {
		return;
	}

	Engine *engine = Engine::get_singleton();
	ERR_FAIL_NULL(engine);

	bool is_editor = engine->is_editor_hint();

	// Set this right away, to prevent getting in a loop of project settings changes.
	already_setup_global_uniforms = true;

	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_AVAILABLE_NAME, RenderingServer::GLOBAL_VAR_TYPE_BOOL, false, rs, project_settings, is_editor);
	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_TEXTURE_NAME, RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY, Variant(), rs, project_settings, is_editor);
	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_RESOLUTION_NAME, RenderingServer::GLOBAL_VAR_TYPE_INT, 0, rs, project_settings, is_editor);
	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_TANFOV_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_VEC4, Vector4(), rs, project_settings, is_editor);
	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_TANFOV_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_VEC4, Vector4(), rs, project_settings, is_editor);
	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Transform3D(), rs, project_settings, is_editor);
	create_shader_global_uniform(ANDROID_ENVIRONMENT_DEPTH_CAMERA_TO_WORLD_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Transform3D(), rs, project_settings, is_editor);
}

bool OpenXRAndroidEnvironmentDepthExtension::initialize_android_environment_depth_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateDepthResolutionsANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateDepthSwapchainANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyDepthSwapchainANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateDepthSwapchainImagesANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrAcquireDepthSwapchainImagesANDROID);

	return true;
}

void OpenXRAndroidEnvironmentDepthExtension::cleanup() {
	android_environment_depth_ext = false;
	depth_camera_data.reset();
}

bool OpenXRAndroidEnvironmentDepthExtension::_ensure_depth_swapchain_is_created(DepthCameraResolution p_resolution, bool p_smooth) {
	if (!supported_resolutions.has(p_resolution)) {
		UtilityFunctions::printerr("OpenXR: resolution is not supported");
		return false;
	}

	if (p_resolution == resolution && p_smooth == smooth && depth_camera_data.swapchain != XR_NULL_HANDLE) {
		return true;
	}

	int res = 0;
	XrDepthCameraResolutionANDROID xr_resolution;
	switch (p_resolution) {
		case DEPTH_CAMERA_RESOLUTION_80x80:
			res = 80;
			xr_resolution = XR_DEPTH_CAMERA_RESOLUTION_80x80_ANDROID;
			break;
		case DEPTH_CAMERA_RESOLUTION_160x160:
			res = 160;
			xr_resolution = XR_DEPTH_CAMERA_RESOLUTION_160x160_ANDROID;
			break;
		case DEPTH_CAMERA_RESOLUTION_320x320:
			res = 320;
			xr_resolution = XR_DEPTH_CAMERA_RESOLUTION_320x320_ANDROID;
			break;
		default:
			UtilityFunctions::printerr("OpenXR: invalid DepthCameraResolution; ", p_resolution);
			return false;
	}

	XrDepthSwapchainCreateInfoANDROID create_info = {
		XR_TYPE_DEPTH_SWAPCHAIN_CREATE_INFO_ANDROID, // type
		nullptr, // next
		xr_resolution, // resolution
		p_smooth ? (XR_DEPTH_SWAPCHAIN_CREATE_SMOOTH_DEPTH_IMAGE_BIT_ANDROID | XR_DEPTH_SWAPCHAIN_CREATE_SMOOTH_CONFIDENCE_IMAGE_BIT_ANDROID) : (XR_DEPTH_SWAPCHAIN_CREATE_RAW_DEPTH_IMAGE_BIT_ANDROID | XR_DEPTH_SWAPCHAIN_CREATE_RAW_CONFIDENCE_IMAGE_BIT_ANDROID), // createFlags
	};

	XrDepthSwapchainANDROID swapchain = XR_NULL_HANDLE;
	XrResult result = xrCreateDepthSwapchainANDROID(SESSION, &create_info, &swapchain);
	if (result != XR_SUCCESS || XR_NULL_HANDLE == swapchain) {
		UtilityFunctions::printerr("OpenXR: Failed to create depth swapchain; ", get_openxr_api()->get_error_string(result));
		return false;
	}

	DepthCameraData data;

	// got the swapchain, enumerate the depth images (this only needs to be done once per swapchain,
	// according to the example here:
	// https://developer.android.com/develop/xr/openxr/extensions/XR_ANDROID_depth_texture#example-code)
	uint32_t depth_image_count_output = 0;
	result = xrEnumerateDepthSwapchainImagesANDROID(swapchain, 0, &depth_image_count_output, nullptr);
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate depth swapchain images; ", get_openxr_api()->get_error_string(result));
		_free_swapchain(swapchain);
		return false;
	}

	if (depth_image_count_output == 0) {
		WARN_PRINT("OpenXR: found zero depth swapchain images");
		_free_swapchain(swapchain);
		return false;
	}

	data.xr_images.resize(depth_image_count_output);
	for (XrDepthSwapchainImageANDROID &image : data.xr_images) {
		image = XrDepthSwapchainImageANDROID{
			XR_TYPE_DEPTH_SWAPCHAIN_IMAGE_ANDROID, // type
			nullptr, // next
			nullptr, // rawDepthImage
			nullptr, // rawDepthConfidenceImage
			nullptr, // smoothDepthImage
			nullptr, // smoothDepthConfidenceImage
		};
	}

	result = xrEnumerateDepthSwapchainImagesANDROID(swapchain, depth_image_count_output, &depth_image_count_output, data.xr_images.ptr());
	if (result != XR_SUCCESS) {
		UtilityFunctions::printerr("OpenXR: Failed to enumerate depth swapchain images; ", get_openxr_api()->get_error_string(result));
		_free_swapchain(swapchain);
		return false;
	}

	if (data.xr_images.size() != depth_image_count_output) {
		WARN_PRINT("OpenXR: enumerate depth swapchain images returned a different count");
		if (data.xr_images.size() < depth_image_count_output) {
			UtilityFunctions::printerr("OpenXR: somehow received more depth swapchain images on the second query; ", get_openxr_api()->get_error_string(result));
			_free_swapchain(swapchain);
			return false;
		}

		if (depth_image_count_output == 0) {
			UtilityFunctions::printerr("OpenXR: second enumerate depth swapchain images returned zero");
			_free_swapchain(swapchain);
			return false;
		}

		data.xr_images.resize(depth_image_count_output);
	}

	data.swapchain = swapchain;
	data.godot_texture_cache.resize(depth_image_count_output);
	depth_camera_data.reset();
	depth_camera_data = data;

	return true;
}

void OpenXRAndroidEnvironmentDepthExtension::_free_swapchain(XrDepthSwapchainANDROID p_swapchain) {
	if (p_swapchain != XR_NULL_HANDLE) {
		OpenXRAndroidEnvironmentDepthExtension::get_singleton()->xrDestroyDepthSwapchainANDROID(p_swapchain);
	}
};

void OpenXRAndroidEnvironmentDepthExtension::destroy_depth_provider() {
	stop_environment_depth();
	supported_resolutions.clear();
}

void OpenXRAndroidEnvironmentDepthExtension::DepthCameraData::reset() {
	OpenXRAndroidEnvironmentDepthExtension::_free_swapchain(swapchain);
	swapchain = XR_NULL_HANDLE;

	RenderingServer *rs = RenderingServer::get_singleton();
	for (Cache &cache : godot_texture_cache) {
		if (rs != nullptr && cache.rid.is_valid()) {
			rs->free_rid(cache.rid);
		}
		cache.rid = RID();
		cache.images[0] = Ref<Image>();
		cache.images[1] = Ref<Image>();
	}

	godot_texture_cache.reset();
}

void OpenXRAndroidEnvironmentDepthExtension::_update_mesh() {
	if (reprojection_material.is_null()) {
		return;
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	// Create a quad mesh consisting of many points
	int res;
	switch (resolution) {
		case DEPTH_CAMERA_RESOLUTION_80x80:
			res = 80;
			break;
		case DEPTH_CAMERA_RESOLUTION_160x160:
			res = 160;
			break;
		case DEPTH_CAMERA_RESOLUTION_320x320:
			res = 320;
			break;
		default:
			// this "should never happen", but print anyway
			UtilityFunctions::printerr("OpenXR: Invalid resolution");
			return;
	}

	// the shader will reposition these vertices
	PackedVector3Array vertices;
	vertices.resize(res * res);

	Array arr;
	arr.resize(RenderingServer::ARRAY_MAX);
	arr[RenderingServer::ARRAY_VERTEX] = vertices;

	reprojection_mesh.instantiate();
	reprojection_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_POINTS, arr);
	reprojection_mesh->surface_set_material(0, reprojection_material);
	reprojection_mesh->set_custom_aabb(AABB(Vector3(-1000, -1000, -1000), Vector3(2000, 2000, 2000)));
	rs->global_shader_parameter_set(ANDROID_ENVIRONMENT_DEPTH_RESOLUTION_NAME, res);
}
