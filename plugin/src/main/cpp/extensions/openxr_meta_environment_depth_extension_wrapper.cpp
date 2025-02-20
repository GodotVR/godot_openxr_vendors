/**************************************************************************/
/*  openxr_meta_environment_depth_extension_wrapper.cpp                   */
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

#include "extensions/openxr_meta_environment_depth_extension_wrapper.h"

#ifdef ANDROID_ENABLED
#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_OPENGL_ES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <jni.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>

#include <openxr/openxr_platform.h>
#endif // ANDROID_ENABLED

#include <openxr/internal/xr_linear.h>

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *META_ENVIRONMENT_DEPTH_AVAILABLE_NAME = "META_ENVIRONMENT_DEPTH_AVAILABLE";
static const char *META_ENVIRONMENT_DEPTH_TEXTURE_NAME = "META_ENVIRONMENT_DEPTH_TEXTURE";
static const char *META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT_NAME = "META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT";
static const char *META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT_NAME = "META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT";
static const char *META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_LEFT_NAME = "META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_LEFT";
static const char *META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_RIGHT_NAME = "META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_RIGHT";
static const char *META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT_NAME = "META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT";
static const char *META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT_NAME = "META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT";
static const char *META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT_NAME = "META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT";
static const char *META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT_NAME = "META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT";

static const char *META_ENVIRONMENT_DEPTH_REPROJECTION_SHADER_CODE = R"(
shader_type spatial;
render_mode unshaded, shadow_to_opacity, shadows_disabled, cull_disabled, depth_draw_always;
global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest, repeat_disable, hint_default_black;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT;
//DEFINES
#ifdef USE_DEPTH_OFFSET_SCALE
uniform highp float depth_offset_scale = 0.0;
#ifdef USE_DEPTH_OFFSET_EXPONENT
uniform highp float depth_offset_exponent = 1.0;
#endif // USE_DEPTH_OFFSET_EXPONENT
#endif // USE_DEPTH_OFFSET_SCALE
void vertex() {
	UV = VERTEX.xy * 0.5 + 0.5;
	POSITION = vec4(VERTEX.xyz, 1.0);
}
void fragment() {
	highp mat4 camera_to_depth_proj = (VIEW_INDEX == VIEW_MONO_LEFT) ? META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT;
	highp mat4 depth_to_camera_proj = (VIEW_INDEX == VIEW_MONO_LEFT) ? META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT;
	highp vec4 clip = vec4(UV * 2.0 - 1.0, 1.0, 1.0);
	highp vec4 reprojected = camera_to_depth_proj * clip;
	reprojected /= reprojected.w;
	highp vec2 reprojected_uv = reprojected.xy * 0.5 + 0.5;
	highp float depth = 0.0;
	if (reprojected_uv.x >= 0.0 && reprojected_uv.y >= 0.0 && reprojected_uv.x <= 1.0 && reprojected_uv.y <= 1.0) {
		depth = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(reprojected_uv, float(VIEW_INDEX))).r;
	}
	if (depth == 0.0) {
		discard;
	}
	highp vec4 clip_back = vec4(reprojected.xy, depth * 2.0 - 1.0, 1.0);
#ifdef USE_DEPTH_OFFSET_SCALE
#ifdef USE_DEPTH_OFFSET_EXPONENT
	highp float z_adjustment = pow(abs(clip_back.w), depth_offset_exponent) * depth_offset_scale;
#else
	highp float z_adjustment = abs(clip_back.w) * depth_offset_scale;
#endif // USE_DEPTH_OFFSET_EXPONENT
	clip_back.z += PROJECTION_MATRIX[2][2] * z_adjustment;
#endif // USE_DEPTH_OFFSET_SCALE
	clip_back = depth_to_camera_proj * clip_back;
	highp float camera_ndc_z = clip_back.z / clip_back.w;
#if CURRENT_RENDERER == RENDERER_COMPATIBILITY
	highp float camera_depth = camera_ndc_z * 0.5 + 0.5;
#else
	highp float camera_depth = camera_ndc_z;
#endif
	ALBEDO = vec3(0.0, 0.0, 0.0);
	DEPTH = 1.0 - camera_depth;
}
)";

OpenXRMetaEnvironmentDepthExtensionWrapper *OpenXRMetaEnvironmentDepthExtensionWrapper::singleton = nullptr;

OpenXRMetaEnvironmentDepthExtensionWrapper *OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaEnvironmentDepthExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaEnvironmentDepthExtensionWrapper::OpenXRMetaEnvironmentDepthExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaEnvironmentDepthExtensionWrapper singleton already exists.");

	request_extensions[XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME] = &meta_environment_depth_ext;
	singleton = this;

#ifndef ANDROID_ENABLED
	graphics_api = GRAPHICS_API_UNSUPPORTED;
#endif // ANDROID_ENABLED
}

OpenXRMetaEnvironmentDepthExtensionWrapper::~OpenXRMetaEnvironmentDepthExtensionWrapper() {
	cleanup();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_environment_depth_supported"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_supported);
	ClassDB::bind_method(D_METHOD("is_hand_removal_supported"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_hand_removal_supported);

	ClassDB::bind_method(D_METHOD("start_environment_depth"), &OpenXRMetaEnvironmentDepthExtensionWrapper::start_environment_depth);
	ClassDB::bind_method(D_METHOD("stop_environment_depth"), &OpenXRMetaEnvironmentDepthExtensionWrapper::stop_environment_depth);
	ClassDB::bind_method(D_METHOD("is_environment_depth_started"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_started);

	ClassDB::bind_method(D_METHOD("set_hand_removal_enabled", "enable"), &OpenXRMetaEnvironmentDepthExtensionWrapper::set_hand_removal_enabled);
	ClassDB::bind_method(D_METHOD("get_hand_removal_enabled"), &OpenXRMetaEnvironmentDepthExtensionWrapper::get_hand_removal_enabled);

	ADD_SIGNAL(MethodInfo("openxr_meta_environment_depth_started"));
	ADD_SIGNAL(MethodInfo("openxr_meta_environment_depth_stopped"));
}

Dictionary OpenXRMetaEnvironmentDepthExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (meta_environment_depth_ext) {
		bool result = initialize_meta_environment_depth_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize XR_META_environment_depth extension");
			meta_environment_depth_ext = false;
		}
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_session_destroyed() {
	destroy_depth_provider();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_pre_render() {
#ifdef ANDROID_ENABLED
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (unlikely(graphics_api == GRAPHICS_API_UNKNOWN)) {
		check_graphics_api();
	}

	if (unlikely(reprojection_material_dirty)) {
		update_reprojection_material();
	}

	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, false);
	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, RID());

	if (depth_provider == XR_NULL_HANDLE || !depth_provider_started) {
		return;
	}

	Ref<OpenXRAPIExtension> openxr_api = get_openxr_api();
	ERR_FAIL_COND(openxr_api.is_null());

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL(xr_server);

	Ref<XRInterface> openxr_interface = xr_server->find_interface("OpenXR");
	ERR_FAIL_COND(openxr_interface.is_null());

	SceneTree *scene_tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	ERR_FAIL_NULL(scene_tree);

	XrEnvironmentDepthImageAcquireInfoMETA acquire_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META, // type
		nullptr, // next
		(XrSpace)openxr_api->get_play_space(),
		openxr_api->get_predicted_display_time(),
	};

	XrEnvironmentDepthImageMETA depth_image = {
		XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META, // type
		nullptr, // next
		0, // swapchainIndex
		0.0, // nearZ
		0.0, // farZ
		{
				// views
				{
						XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META, // type
						nullptr, // next
				},
				{
						XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META, // type
						nullptr, // next
				},
		}
	};

	XrResult result = xrAcquireEnvironmentDepthImageMETA(depth_provider, &acquire_info, &depth_image);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to acquire environment depth image: ", openxr_api->get_error_string(result));
		return;
	}

	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, true);
	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, depth_swapchain_textures[depth_image.swapchainIndex]);

	Transform3D world_origin = xr_server->get_world_origin();
	Vector2 viewport_size = openxr_interface->get_render_target_size();
	float aspect = viewport_size.width / viewport_size.height;

	Camera3D *camera = scene_tree->get_root()->get_camera_3d();
	float z_near = camera->get_near();
	float z_far = camera->get_far();

	for (int i = 0; i < 2; i++) {
		XrPosef local_from_depth_eye = depth_image.views[i].pose;
		XrPosef depth_eye_from_local;
		XrPosef_Invert(&depth_eye_from_local, &local_from_depth_eye);

		XrMatrix4x4f view_mat;
		XrMatrix4x4f_CreateFromRigidTransform(&view_mat, &depth_eye_from_local);

		XrMatrix4x4f projection_mat;
		XrMatrix4x4f_CreateProjectionFov(
				&projection_mat,
				GRAPHICS_OPENGL,
				depth_image.views[i].fov,
				depth_image.nearZ,
				std::isfinite(depth_image.farZ) ? depth_image.farZ : 0);

		// Copy into Godot projections.
		Projection godot_view_mat;
		OpenXRUtilities::xrMatrix4x4f_to_godot_projection(&view_mat, godot_view_mat);
		Projection godot_projection_mat;
		OpenXRUtilities::xrMatrix4x4f_to_godot_projection(&projection_mat, godot_projection_mat);

		Projection depth_proj_view = godot_projection_mat * godot_view_mat;
		rs->global_shader_parameter_set(i == 0 ? META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT_NAME : META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT_NAME, depth_proj_view);
		rs->global_shader_parameter_set(i == 0 ? META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_LEFT_NAME : META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_RIGHT_NAME, depth_proj_view.inverse());

		Projection camera_proj_view = openxr_interface->get_projection_for_view(i, aspect, z_near, z_far) * openxr_interface->get_transform_for_view(i, world_origin).affine_inverse();

		if (graphics_api == GRAPHICS_API_VULKAN) {
			Projection correction;
			correction.set_depth_correction(true);
			camera_proj_view = correction * camera_proj_view;
		}

		rs->global_shader_parameter_set(i == 0 ? META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT_NAME : META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT_NAME, depth_proj_view * camera_proj_view.inverse());
		rs->global_shader_parameter_set(i == 0 ? META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT_NAME : META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT_NAME, camera_proj_view * depth_proj_view.inverse());
	}
#endif // ANDROID_ENABLED
}

uint64_t OpenXRMetaEnvironmentDepthExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_environment_depth_ext) {
		system_depth_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_depth_properties);
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::start_environment_depth() {
	if (depth_provider == XR_NULL_HANDLE) {
		if (!create_depth_provider()) {
			return;
		}
	}

	XrResult result = xrStartEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to start environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	setup_global_uniforms();

	depth_provider_started = true;
	emit_signal("openxr_meta_environment_depth_started");
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::stop_environment_depth() {
	ERR_FAIL_NULL(depth_provider);

	XrResult result = xrStopEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to stop environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	depth_provider_started = false;
	emit_signal("openxr_meta_environment_depth_stopped");
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_started() {
	return depth_provider_started;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_hand_removal_enabled(bool p_enable) {
	if (depth_provider == XR_NULL_HANDLE) {
		if (!create_depth_provider()) {
			return;
		}
	}

	XrEnvironmentDepthHandRemovalSetInfoMETA info = {
		XR_TYPE_ENVIRONMENT_DEPTH_HAND_REMOVAL_SET_INFO_META, // type
		nullptr, // next
		p_enable, // enabled
	};

	XrResult result = xrSetEnvironmentDepthHandRemovalMETA(depth_provider, &info);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to set hand removal enabled: ", get_openxr_api()->get_error_string(result));
		return;
	}

	hand_removal_enabled = p_enable;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::get_hand_removal_enabled() const {
	return hand_removal_enabled;
}

RID OpenXRMetaEnvironmentDepthExtensionWrapper::get_reprojection_mesh() {
	if (reprojection_mesh.is_null()) {
		reprojection_shader.instantiate();
		reprojection_material.instantiate();

		update_reprojection_material(true);

		reprojection_material->set_render_priority(reprojection_render_priority);

		PackedVector3Array vertices;
		vertices.resize(3);
		vertices[0] = Vector3(-1.0f, -1.0f, 1.0f);
		vertices[1] = Vector3(3.0f, -1.0f, 1.0f);
		vertices[2] = Vector3(-1.0f, 3.0f, 1.0f);

		Array arr;
		arr.resize(RenderingServer::ARRAY_MAX);
		arr[RenderingServer::ARRAY_VERTEX] = vertices;

		reprojection_mesh.instantiate();
		reprojection_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);
		reprojection_mesh->surface_set_material(0, reprojection_material);
		reprojection_mesh->set_custom_aabb(AABB(Vector3(-1000, -1000, -1000), Vector3(2000, 2000, 2000)));
	}

	return reprojection_mesh->get_rid();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::update_reprojection_material(bool p_creation) {
	if (reprojection_shader.is_null() || reprojection_material.is_null()) {
		return;
	}

	String shader_code = META_ENVIRONMENT_DEPTH_REPROJECTION_SHADER_CODE;
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

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_reprojection_render_priority(int p_render_priority) {
	reprojection_render_priority = p_render_priority;

	if (reprojection_material.is_valid()) {
		reprojection_material->set_render_priority(p_render_priority);
	}
}

int OpenXRMetaEnvironmentDepthExtensionWrapper::get_reprojection_render_priority() const {
	return reprojection_render_priority;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_reprojection_offset_scale(float p_offset_scale) {
	reprojection_offset_scale = p_offset_scale;
	reprojection_material_dirty = true;
}

float OpenXRMetaEnvironmentDepthExtensionWrapper::get_reprojection_offset_scale() const {
	return reprojection_offset_scale;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_reprojection_offset_exponent(float p_offset_exponent) {
	reprojection_offset_exponent = p_offset_exponent;
	reprojection_material_dirty = true;
}

float OpenXRMetaEnvironmentDepthExtensionWrapper::get_reprojection_offset_exponent() const {
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
				case RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY: {
					type_name = "sampler2DArray";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_MAT4: {
					type_name = "mat4";
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

void OpenXRMetaEnvironmentDepthExtensionWrapper::setup_global_uniforms() {
	if (already_setup_global_uniforms) {
		return;
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	Engine *engine = Engine::get_singleton();
	ERR_FAIL_NULL(engine);

	bool is_editor = engine->is_editor_hint();

	// Set this right away, to prevent getting in a loop of project settings changes.
	already_setup_global_uniforms = true;

	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, RenderingServer::GLOBAL_VAR_TYPE_BOOL, false, rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY, Variant(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::initialize_meta_environment_depth_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStartEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateEnvironmentDepthSwapchainImagesMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetEnvironmentDepthSwapchainStateMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrAcquireEnvironmentDepthImageMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetEnvironmentDepthHandRemovalMETA);

	return true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::cleanup() {
	meta_environment_depth_ext = false;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::create_depth_provider() {
#ifdef ANDROID_ENABLED
	if (!meta_environment_depth_ext) {
		UtilityFunctions::printerr("Environment depth not supported");
		return false;
	}
	if (graphics_api == GRAPHICS_API_UNKNOWN) {
		check_graphics_api();
	}
	if (graphics_api == GRAPHICS_API_UNSUPPORTED) {
		UtilityFunctions::printerr("Environment depth is not supported with the current graphics API");
		return false;
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rs, false);

	XrResult result;

	XrEnvironmentDepthProviderCreateInfoMETA provider_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	result = xrCreateEnvironmentDepthProviderMETA(SESSION, &provider_create_info, &depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create environment depth provider: ", get_openxr_api()->get_error_string(result));
		return false;
	}

	XrEnvironmentDepthSwapchainCreateInfoMETA swapchain_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	result = xrCreateEnvironmentDepthSwapchainMETA(depth_provider, &swapchain_create_info, &depth_swapchain);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create environment depth swapchain: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	XrEnvironmentDepthSwapchainStateMETA swapchain_state = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_STATE_META, // type
		nullptr, // next
		0, // width
		0, // height
	};

	result = xrGetEnvironmentDepthSwapchainStateMETA(depth_swapchain, &swapchain_state);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to get environment depth swapchain state: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	uint32_t swapchain_length = 0;

	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to get environment depth swapchain image count: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	if (graphics_api == GRAPHICS_API_OPENGL) {
		LocalVector<XrSwapchainImageOpenGLESKHR> swapchain_images;

		swapchain_images.resize(swapchain_length);
		for (auto &image : swapchain_images) {
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
			image.next = nullptr;
			image.image = 0;
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader *)swapchain_images.ptr());
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to get environment depth swapchain images: ", get_openxr_api()->get_error_string(result));
			destroy_depth_provider();
			return false;
		}

		depth_swapchain_textures.reserve(swapchain_length);

		for (const auto &image : swapchain_images) {
			RID texture = rs->texture_create_from_native_handle(
					RenderingServer::TextureType::TEXTURE_TYPE_LAYERED,
					Image::Format::FORMAT_RH, // GL_DEPTH_COMPONENT16
					image.image,
					swapchain_state.width,
					swapchain_state.height,
					1,
					2,
					RenderingServer::TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY);

			depth_swapchain_textures.push_back(texture);
		}
	} else if (graphics_api == GRAPHICS_API_VULKAN) {
		LocalVector<XrSwapchainImageVulkanKHR> swapchain_images;

		swapchain_images.resize(swapchain_length);
		for (auto &image : swapchain_images) {
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
			image.next = nullptr;
			image.image = 0;
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader *)swapchain_images.ptr());
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to get environment depth swapchain images: ", get_openxr_api()->get_error_string(result));
			destroy_depth_provider();
			return false;
		}

		depth_swapchain_textures.reserve(swapchain_length);

		RenderingDevice *rendering_device = RenderingServer::get_singleton()->get_rendering_device();

		for (const auto &image : swapchain_images) {
			RID rd_texture = rendering_device->texture_create_from_extension(
					RenderingDevice::TEXTURE_TYPE_2D_ARRAY,
					RenderingDevice::DATA_FORMAT_D16_UNORM,
					RenderingDevice::TEXTURE_SAMPLES_1,
					RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					reinterpret_cast<uint64_t>(image.image),
					swapchain_state.width,
					swapchain_state.height,
					1,
					2);

			RID texture = rs->texture_rd_create(rd_texture, RenderingServer::TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY);

			depth_swapchain_textures.push_back(texture);
		}
	}

	return true;
#else
	return false;
#endif // ANDROID_ENABLED
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::check_graphics_api() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	String rendering_driver = RenderingServer::get_singleton()->get_current_rendering_driver_name();
	if (rendering_driver.contains("opengl")) {
		graphics_api = GRAPHICS_API_OPENGL;
	} else if (rendering_driver == "vulkan") {
		graphics_api = GRAPHICS_API_VULKAN;
	} else {
		graphics_api = GRAPHICS_API_UNSUPPORTED;
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::destroy_depth_provider() {
	if (depth_provider_started) {
		stop_environment_depth();
	}

	if (depth_swapchain != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthSwapchainMETA(depth_swapchain);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to destroy environment depth swapchain: ", get_openxr_api()->get_error_string(result));
		}
		depth_swapchain = XR_NULL_HANDLE;
	}

	depth_swapchain_textures.clear();

	if (depth_provider != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthProviderMETA(depth_provider);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to destroy environment depth provider: ", get_openxr_api()->get_error_string(result));
		}
		depth_provider = XR_NULL_HANDLE;
		hand_removal_enabled = false;
	}
}
