/**************************************************************************/
/*  openxr_fb_passthrough_extension_wrapper.cpp                           */
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

#include "extensions/openxr_fb_passthrough_extension_wrapper.h"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbPassthroughExtensionWrapper *OpenXRFbPassthroughExtensionWrapper::singleton = nullptr;

OpenXRFbPassthroughExtensionWrapper *OpenXRFbPassthroughExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbPassthroughExtensionWrapper());
	}
	return singleton;
}

OpenXRFbPassthroughExtensionWrapper::OpenXRFbPassthroughExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbPassthroughExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_PASSTHROUGH_EXTENSION_NAME] = &fb_passthrough_ext;
	request_extensions[XR_FB_TRIANGLE_MESH_EXTENSION_NAME] = &fb_triangle_mesh_ext;
	request_extensions[XR_META_PASSTHROUGH_PREFERENCES_EXTENSION_NAME] = &meta_passthrough_preferences_ext;
	request_extensions[XR_META_PASSTHROUGH_COLOR_LUT_EXTENSION_NAME] = &meta_passthrough_color_lut_ext;

	singleton = this;
}

OpenXRFbPassthroughExtensionWrapper::~OpenXRFbPassthroughExtensionWrapper() {
	cleanup();
}

void OpenXRFbPassthroughExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_passthrough_supported"), &OpenXRFbPassthroughExtensionWrapper::is_passthrough_supported);
	ClassDB::bind_method(D_METHOD("is_passthrough_started"), &OpenXRFbPassthroughExtensionWrapper::is_passthrough_started);

	ClassDB::bind_method(D_METHOD("set_texture_opacity_factor", "value"), &OpenXRFbPassthroughExtensionWrapper::set_texture_opacity_factor);
	ClassDB::bind_method(D_METHOD("get_texture_opacity_factor"), &OpenXRFbPassthroughExtensionWrapper::get_texture_opacity_factor);

	ClassDB::bind_method(D_METHOD("get_current_layer_purpose"), &OpenXRFbPassthroughExtensionWrapper::get_current_layer_purpose);

	ClassDB::bind_method(D_METHOD("set_edge_color", "color"), &OpenXRFbPassthroughExtensionWrapper::set_edge_color);
	ClassDB::bind_method(D_METHOD("get_edge_color"), &OpenXRFbPassthroughExtensionWrapper::get_edge_color);

	ClassDB::bind_method(D_METHOD("set_passthrough_filter", "filter"), &OpenXRFbPassthroughExtensionWrapper::set_passthrough_filter);
	ClassDB::bind_method(D_METHOD("get_current_passthrough_filter"), &OpenXRFbPassthroughExtensionWrapper::get_current_passthrough_filter);
	ClassDB::bind_method(D_METHOD("set_color_map", "gradient"), &OpenXRFbPassthroughExtensionWrapper::set_color_map);
	ClassDB::bind_method(D_METHOD("set_mono_map", "curve"), &OpenXRFbPassthroughExtensionWrapper::set_mono_map);
	ClassDB::bind_method(D_METHOD("set_brightness_contrast_saturation", "brightness", "contrast", "saturation"), &OpenXRFbPassthroughExtensionWrapper::set_brightness_contrast_saturation);

	ClassDB::bind_method(D_METHOD("has_passthrough_capability"), &OpenXRFbPassthroughExtensionWrapper::has_passthrough_capability);
	ClassDB::bind_method(D_METHOD("has_color_passthrough_capability"), &OpenXRFbPassthroughExtensionWrapper::has_color_passthrough_capability);
	ClassDB::bind_method(D_METHOD("has_layer_depth_passthrough_capability"), &OpenXRFbPassthroughExtensionWrapper::has_layer_depth_passthrough_capability);

	ClassDB::bind_method(D_METHOD("is_passthrough_preferred"), &OpenXRFbPassthroughExtensionWrapper::is_passthrough_preferred);

	ClassDB::bind_method(D_METHOD("set_color_lut", "weight", "color_lut"), &OpenXRFbPassthroughExtensionWrapper::set_color_lut);
	ClassDB::bind_method(D_METHOD("set_interpolated_color_lut", "weight", "source_color_lut", "target_color_lut"), &OpenXRFbPassthroughExtensionWrapper::set_interpolated_color_lut);
	ClassDB::bind_method(D_METHOD("get_max_color_lut_resolution"), &OpenXRFbPassthroughExtensionWrapper::get_max_color_lut_resolution);

	ADD_SIGNAL(MethodInfo("openxr_fb_projected_passthrough_layer_created"));
	ADD_SIGNAL(MethodInfo("openxr_fb_passthrough_stopped"));
	ADD_SIGNAL(MethodInfo("openxr_fb_passthrough_state_changed", PropertyInfo(Variant::INT, "event_type")));

	BIND_ENUM_CONSTANT(LAYER_PURPOSE_NONE);
	BIND_ENUM_CONSTANT(LAYER_PURPOSE_RECONSTRUCTION);
	BIND_ENUM_CONSTANT(LAYER_PURPOSE_PROJECTED);

	BIND_ENUM_CONSTANT(PASSTHROUGH_FILTER_DISABLED);
	BIND_ENUM_CONSTANT(PASSTHROUGH_FILTER_COLOR_MAP);
	BIND_ENUM_CONSTANT(PASSTHROUGH_FILTER_MONO_MAP);
	BIND_ENUM_CONSTANT(PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION);
	BIND_ENUM_CONSTANT(PASSTHROUGH_FILTER_COLOR_MAP_LUT);
	BIND_ENUM_CONSTANT(PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT);

	BIND_ENUM_CONSTANT(PASSTHROUGH_ERROR_NON_RECOVERABLE);
	BIND_ENUM_CONSTANT(PASSTHROUGH_ERROR_RECOVERABLE);
	BIND_ENUM_CONSTANT(PASSTHROUGH_ERROR_RESTORED);
}

godot::Dictionary OpenXRFbPassthroughExtensionWrapper::_get_requested_extensions(uint64_t p_xr_version) {
	godot::Dictionary result;
	for (auto ext : request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbPassthroughExtensionWrapper::cleanup() {
	fb_passthrough_ext = false;
	fb_triangle_mesh_ext = false;
	meta_passthrough_preferences_ext = false;
	meta_passthrough_color_lut_ext = false;
}

uint64_t OpenXRFbPassthroughExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (fb_passthrough_ext) {
		system_passthrough_properties.next = p_next_pointer;
		p_next_pointer = &system_passthrough_properties;
	}
	if (meta_passthrough_color_lut_ext) {
		system_passthrough_color_lut_properties.next = p_next_pointer;
		p_next_pointer = &system_passthrough_color_lut_properties;
	}
	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRFbPassthroughExtensionWrapper::_on_instance_created(uint64_t p_instance) {
	XrInstance instance = (XrInstance)p_instance;
	if (fb_passthrough_ext) {
		bool result = initialize_fb_passthrough_extension(instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_passthrough extension");
			fb_passthrough_ext = false;
		}
	}

	if (fb_triangle_mesh_ext) {
		bool result = initialize_fb_triangle_mesh_extension(instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_triangle_mesh extension");
			fb_triangle_mesh_ext = false;
		}
	}

	if (meta_passthrough_preferences_ext) {
		bool result = initialize_meta_passthrough_preferences_extension(instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize meta_passthrough_preferences extension");
			meta_passthrough_preferences_ext = false;
		}
	}

	if (meta_passthrough_color_lut_ext) {
		bool result = initialize_meta_passthrough_color_lut_extension(instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize meta_passthrough_color_lut extension");
			meta_passthrough_color_lut_ext = false;
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_session_created(uint64_t p_session) {
	XrSession session = (XrSession)p_session;
	if (fb_passthrough_ext) {
		// If another extension is already emulating alpha blend mode, then we don't attempt to.
		if (get_openxr_api()->is_environment_blend_mode_alpha_supported() == OpenXRAPIExtension::OPENXR_ALPHA_BLEND_MODE_SUPPORT_EMULATING) {
			// Act as if the extension is not enabled.
			fb_passthrough_ext = false;
			return;
		}

		// Create the passthrough feature and start it.
		XrPassthroughCreateInfoFB passthrough_create_info = {
			XR_TYPE_PASSTHROUGH_CREATE_INFO_FB, // type
			nullptr, // next
			0, // flags
		};

		// Since OpenXR isn't yet involved in rendering yet, it's safe to write to `render_state`.
		XrResult result = xrCreatePassthroughFB(session, &passthrough_create_info, &render_state.passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create passthrough");
			render_state.passthrough_handle = XR_NULL_HANDLE;
			return;
		}

		get_openxr_api()->register_composition_layer_provider(this);
		get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(true);
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_state_ready() {
	if (!fb_passthrough_ext) {
		return;
	}

	XRInterface::EnvironmentBlendMode blend_mode = get_blend_mode();
	if (blend_mode == XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND) {
		start_passthrough_layer(LAYER_PURPOSE_RECONSTRUCTION);
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_process() {
	if (!fb_passthrough_ext) {
		return;
	}

	XRInterface::EnvironmentBlendMode blend_mode = get_blend_mode();

	// Reconstruction layer will always take priority
	if (blend_mode == XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && current_passthrough_layer != LAYER_PURPOSE_RECONSTRUCTION) {
		start_passthrough_layer(LAYER_PURPOSE_RECONSTRUCTION);
	} else if (blend_mode != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && geometry_instances.get_rid_count() > 0 && current_passthrough_layer != LAYER_PURPOSE_PROJECTED) {
		start_passthrough_layer(LAYER_PURPOSE_PROJECTED);
	} else if (blend_mode != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && geometry_instances.get_rid_count() == 0 && current_passthrough_layer != LAYER_PURPOSE_NONE) {
		stop_passthrough();
	}
}

bool OpenXRFbPassthroughExtensionWrapper::_on_event_polled(const void *p_event) {
	if (!fb_passthrough_ext) {
		return false;
	}

	if (static_cast<const XrEventDataBuffer *>(p_event)->type == XR_TYPE_EVENT_DATA_PASSTHROUGH_STATE_CHANGED_FB) {
		XrEventDataPassthroughStateChangedFB *passthrough_event = (XrEventDataPassthroughStateChangedFB *)p_event;
		switch (passthrough_event->flags) {
			case XR_PASSTHROUGH_STATE_CHANGED_REINIT_REQUIRED_BIT_FB: {
				stop_passthrough();
			} break;
			case XR_PASSTHROUGH_STATE_CHANGED_NON_RECOVERABLE_ERROR_BIT_FB: {
				emit_signal("openxr_fb_passthrough_state_changed", PASSTHROUGH_ERROR_NON_RECOVERABLE);
			} break;
			case XR_PASSTHROUGH_STATE_CHANGED_RECOVERABLE_ERROR_BIT_FB: {
				emit_signal("openxr_fb_passthrough_state_changed", PASSTHROUGH_ERROR_RECOVERABLE);
			} break;
			case XR_PASSTHROUGH_STATE_CHANGED_RESTORED_ERROR_BIT_FB: {
				emit_signal("openxr_fb_passthrough_state_changed", PASSTHROUGH_ERROR_RESTORED);
			} break;
		}
		return true;
	}

	return false;
}

void OpenXRFbPassthroughExtensionWrapper::_on_session_destroyed() {
	if (fb_passthrough_ext) {
		// Since OpenXR is no longer involved in rendering, it's safe to write to `render_state`.
		_stop_passthrough_rt();

		XrResult result;
		if (render_state.passthrough_handle != XR_NULL_HANDLE) {
			result = xrDestroyPassthroughFB(render_state.passthrough_handle);
			if (XR_FAILED(result)) {
				UtilityFunctions::print("Unable to destroy passthrough feature");
			}
			render_state.passthrough_handle = XR_NULL_HANDLE;

			get_openxr_api()->unregister_composition_layer_provider(this);
			get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(false);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbPassthroughExtensionWrapper::start_passthrough() {
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_start_passthrough_rt));
}

void OpenXRFbPassthroughExtensionWrapper::_start_passthrough_rt() {
	if (render_state.passthrough_handle == XR_NULL_HANDLE) {
		UtilityFunctions::print("Cannot start passthrough before passthrough handle is created");
		return;
	}

	XrResult result = xrPassthroughStartFB(render_state.passthrough_handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to start passthrough, error code: ", result);
		_stop_passthrough_rt();
		return;
	}

	render_state.passthrough_started = true;

	// Use `call_deferred()` to set the public value on the main thread.
	callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_passthrough_started).bind(true).call_deferred();
}

void OpenXRFbPassthroughExtensionWrapper::start_passthrough_layer(LayerPurpose p_layer_purpose) {
	current_passthrough_layer = p_layer_purpose;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_start_passthrough_layer_rt).bind(p_layer_purpose));
}

void OpenXRFbPassthroughExtensionWrapper::_start_passthrough_layer_rt(LayerPurpose p_layer_purpose) {
	XrPassthroughLayerPurposeFB xr_layer_purpose;
	switch (p_layer_purpose) {
		case LAYER_PURPOSE_RECONSTRUCTION:
			xr_layer_purpose = XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
			break;
		case LAYER_PURPOSE_PROJECTED:
			xr_layer_purpose = XR_PASSTHROUGH_LAYER_PURPOSE_PROJECTED_FB;
			break;
		case LAYER_PURPOSE_NONE:
		case LAYER_PURPOSE_MAX:
			UtilityFunctions::print("Corresponding XrPassthroughLayerPurposeFB not found for LayerPurpose: ", p_layer_purpose);
			// Use `call_deferred()` to set the public value on the main thread.
			callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_current_passthrough_layer).bind(LAYER_PURPOSE_NONE).call_deferred();
			return;
	}

	// If passthrough hasn't started, start it.
	if (!render_state.passthrough_started) {
		_start_passthrough_rt();
		if (!render_state.passthrough_started) {
			// Use `call_deferred()` to set the public value on the main thread.
			callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_current_passthrough_layer).bind(LAYER_PURPOSE_NONE).call_deferred();
			return;
		}
	}

	// If a different layer is active, pause it.
	if (render_state.current_passthrough_layer >= 0 && render_state.current_passthrough_layer != p_layer_purpose) {
		XrResult result = xrPassthroughLayerPauseFB(render_state.passthrough_layer[render_state.current_passthrough_layer]);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to pause current passthrough layer, error code: ", result);
			// Use `call_deferred()` to set the public value on the main thread.
			callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_current_passthrough_layer).bind(LAYER_PURPOSE_NONE).call_deferred();
			return;
		}
	}

	// If layer does not exist, create it.
	if (render_state.passthrough_layer[p_layer_purpose] == XR_NULL_HANDLE) {
		XrPassthroughLayerCreateInfoFB passthrough_layer_config = {
			XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB, // type
			nullptr, // next
			render_state.passthrough_handle, // passthrough
			XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB, // flags
			xr_layer_purpose, // purpose
		};

		XrResult result = xrCreatePassthroughLayerFB(SESSION, &passthrough_layer_config, &render_state.passthrough_layer[p_layer_purpose]);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create passthrough layer ", p_layer_purpose, ", error code: ", result);
			stop_passthrough();
			return;
		}

		if (p_layer_purpose == LAYER_PURPOSE_PROJECTED) {
			// Use `call_deferred()` so the signal will be emitted on the main thread.
			callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_emit_signal).bind("openxr_fb_projected_passthrough_layer_created").call_deferred();
		}
	} else { // Else resume already created layer.
		XrResult result = xrPassthroughLayerResumeFB(render_state.passthrough_layer[p_layer_purpose]);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to resume passthrough layer ", p_layer_purpose, ", error code: ", result);
			// Use `call_deferred()` so the signal will be emitted on the main thread.
			callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_current_passthrough_layer).bind(LAYER_PURPOSE_NONE).call_deferred();
			return;
		}
	}

	render_state.current_passthrough_layer = p_layer_purpose;

	// Apply passthrough style to layer
	XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
	}
}

int OpenXRFbPassthroughExtensionWrapper::_get_composition_layer_count() {
	return render_state.passthrough_started ? 1 : 0;
}

uint64_t OpenXRFbPassthroughExtensionWrapper::_get_composition_layer(int p_index) {
	if (p_index == 0) {
		render_state.composition_passthrough_layer.layerHandle = render_state.passthrough_layer[render_state.current_passthrough_layer];
		return reinterpret_cast<uint64_t>(&render_state.composition_passthrough_layer);
	} else {
		return 0;
	}
}

int OpenXRFbPassthroughExtensionWrapper::_get_composition_layer_order(int p_index) {
	// Ensure the passthrough layer will be behind the projection layer.
	return -100;
}

void OpenXRFbPassthroughExtensionWrapper::stop_passthrough() {
	current_passthrough_layer = LAYER_PURPOSE_NONE;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_stop_passthrough_rt));
}

void OpenXRFbPassthroughExtensionWrapper::_stop_passthrough_rt() {
	if (!fb_passthrough_ext) {
		return;
	}

	XrResult result;
	for (int i = 0; i < LAYER_PURPOSE_MAX; i++) {
		if (render_state.passthrough_layer[i] != XR_NULL_HANDLE) {
			result = xrDestroyPassthroughLayerFB(render_state.passthrough_layer[i]);
			if (XR_FAILED(result)) {
				UtilityFunctions::print("Unable to destroy passthrough layer, error code: ", result);
			}
			render_state.passthrough_layer[i] = XR_NULL_HANDLE;
		}
	}

	if (render_state.passthrough_handle != XR_NULL_HANDLE) {
		result = xrPassthroughPauseFB(render_state.passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Unable to stop passthrough feature, error code: ", result);
			return;
		}
	}

	render_state.current_passthrough_layer = LAYER_PURPOSE_NONE;
	render_state.passthrough_started = false;

	// Use `call_deferred()` so signal is emitted and public values are updated on the main thread.
	callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_emit_signal).bind("openxr_fb_passthrough_stopped").call_deferred();
	callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_passthrough_started).bind(false).call_deferred();
}

RID OpenXRFbPassthroughExtensionWrapper::geometry_instance_create(const Array &p_array_mesh, const Transform3D &p_transform) {
	if (current_passthrough_layer != LAYER_PURPOSE_PROJECTED) {
		start_passthrough_layer(LAYER_PURPOSE_PROJECTED);
	}

	RID ret = geometry_instances.make_rid();
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_geometry_instance_initialize_rt).bind(ret, p_array_mesh, p_transform));
	return ret;
}

void OpenXRFbPassthroughExtensionWrapper::_geometry_instance_initialize_rt(RID p_geometry_instance, const Array &p_array_mesh, const Transform3D &p_transform) {
	GeometryInstance *geometry_instance = geometry_instances.get_or_null(p_geometry_instance);

	if (geometry_instance == nullptr) {
		return;
	}

	if (!render_state.passthrough_started) {
		UtilityFunctions::print("Tried to create geometry instance, but passthrough isn't started!");
		return;
	}

	if (render_state.passthrough_layer[LAYER_PURPOSE_PROJECTED] == XR_NULL_HANDLE) {
		UtilityFunctions::print("Tried to create geometry instance, but there's no projected passthrough layer");
		return;
	}

	Array vertex_array = p_array_mesh[Mesh::ARRAY_VERTEX];
	LocalVector<XrVector3f> vertices;
	vertices.resize(vertex_array.size());
	for (int j = 0; j < vertex_array.size(); j++) {
		Vector3 vertex = vertex_array[j];
		vertices[j] = {
			static_cast<float>(vertex.x),
			static_cast<float>(vertex.y),
			static_cast<float>(vertex.z)
		};
	}

	Array index_array = p_array_mesh[Mesh::ARRAY_INDEX];
	LocalVector<uint32_t> indices;
	indices.resize(index_array.size());
	for (int j = 0; j < index_array.size(); j++) {
		indices[j] = index_array[j];
	}

	XrTriangleMeshFB mesh = XR_NULL_HANDLE;
	XrTriangleMeshCreateInfoFB triangle_mesh_info = {
		XR_TYPE_TRIANGLE_MESH_CREATE_INFO_FB, // type
		nullptr, // next
		0, // flags
		XR_WINDING_ORDER_CW_FB, // windingOrder
		(uint32_t)vertex_array.size(), // vertexCount
		vertices.ptr(), // vertexBuffer
		(uint32_t)index_array.size(), // triangleCount
		indices.ptr(), // indexBuffer
	};

	XrResult result = xrCreateTriangleMeshFB(SESSION, &triangle_mesh_info, &mesh);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create triangle mesh, error code: ", result);
		return;
	}

	Transform3D reference_frame = XRServer::get_singleton()->get_reference_frame();
	Transform3D transform = reference_frame.inverse() * p_transform;

	Quaternion quat = transform.basis.get_rotation_quaternion();
	Vector3 scale = transform.basis.get_scale();

	XrQuaternionf xr_orientation = {
		static_cast<float>(quat.x),
		static_cast<float>(quat.y),
		static_cast<float>(quat.z),
		static_cast<float>(quat.w)
	};
	XrVector3f xr_position = {
		static_cast<float>(transform.origin.x),
		static_cast<float>(transform.origin.y),
		static_cast<float>(transform.origin.z)
	};
	XrPosef xr_pose = { xr_orientation, xr_position };
	XrVector3f xr_scale = {
		static_cast<float>(scale.x),
		static_cast<float>(scale.y),
		static_cast<float>(scale.z)
	};

	XrGeometryInstanceCreateInfoFB geometry_instance_info = {
		XR_TYPE_GEOMETRY_INSTANCE_CREATE_INFO_FB, // type
		nullptr, // next
		render_state.passthrough_layer[LAYER_PURPOSE_PROJECTED], // layer
		mesh, // mesh
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
		xr_pose, // pose
		xr_scale, // scale
	};

	result = xrCreateGeometryInstanceFB(SESSION, &geometry_instance_info, &geometry_instance->handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create geometry instance, error code: ", result);
		return;
	}
}

void OpenXRFbPassthroughExtensionWrapper::geometry_instance_set_transform(RID p_geometry_instance, const Transform3D &p_transform) {
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_geometry_instance_set_transform_rt).bind(p_geometry_instance, p_transform));
}

void OpenXRFbPassthroughExtensionWrapper::_geometry_instance_set_transform_rt(RID p_geometry_instance, const Transform3D &p_transform) {
	GeometryInstance *geometry_instance = geometry_instances.get_or_null(p_geometry_instance);

	if (geometry_instance == nullptr) {
		return;
	}

	Transform3D reference_frame = XRServer::get_singleton()->get_reference_frame();
	Transform3D transform = reference_frame.inverse() * p_transform;

	Quaternion quat = transform.basis.get_rotation_quaternion();
	Vector3 scale = transform.basis.get_scale();

	XrQuaternionf xr_orientation = {
		static_cast<float>(quat.x),
		static_cast<float>(quat.y),
		static_cast<float>(quat.z),
		static_cast<float>(quat.w)
	};
	XrVector3f xr_position = {
		static_cast<float>(transform.origin.x),
		static_cast<float>(transform.origin.y),
		static_cast<float>(transform.origin.z)
	};
	XrPosef xr_pose = { xr_orientation, xr_position };
	XrVector3f xr_scale = {
		static_cast<float>(scale.x),
		static_cast<float>(scale.y),
		static_cast<float>(scale.z)
	};

	XrGeometryInstanceTransformFB xr_transform = {
		XR_TYPE_GEOMETRY_INSTANCE_TRANSFORM_FB, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
		xr_pose, // pose
		xr_scale, // scale
	};

	XrResult result = xrGeometryInstanceSetTransformFB(geometry_instance->handle, &xr_transform);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to set geometry instance transform, error code: ", result);
	}
}

void OpenXRFbPassthroughExtensionWrapper::geometry_instance_free(RID p_geometry_instance) {
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_geometry_instance_free_rt).bind(p_geometry_instance));
}

void OpenXRFbPassthroughExtensionWrapper::_geometry_instance_free_rt(RID p_geometry_instance) {
	GeometryInstance *geometry_instance = geometry_instances.get_or_null(p_geometry_instance);

	if (geometry_instance == nullptr) {
		return;
	}

	XrResult result = xrDestroyGeometryInstanceFB(geometry_instance->handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to destroy geometry instance, error code: ", result);
	}

	geometry_instances.free(p_geometry_instance);
}

void OpenXRFbPassthroughExtensionWrapper::set_texture_opacity_factor(float p_value) {
	texture_opacity_factor = p_value;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_texture_opacity_factor_rt).bind(p_value));
}

void OpenXRFbPassthroughExtensionWrapper::_set_texture_opacity_factor_rt(float p_value) {
	render_state.passthrough_style.textureOpacityFactor = p_value;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

float OpenXRFbPassthroughExtensionWrapper::get_texture_opacity_factor() {
	return texture_opacity_factor;
}

void OpenXRFbPassthroughExtensionWrapper::set_edge_color(Color p_color) {
	edge_color = p_color;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_edge_color_rt).bind(p_color));
}

void OpenXRFbPassthroughExtensionWrapper::_set_edge_color_rt(Color p_color) {
	render_state.passthrough_style.edgeColor = { p_color.r, p_color.g, p_color.b, p_color.a };

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

Color OpenXRFbPassthroughExtensionWrapper::get_edge_color() {
	return edge_color;
}

void OpenXRFbPassthroughExtensionWrapper::set_passthrough_filter(PassthroughFilter p_filter) {
	current_passthrough_filter = p_filter;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_passthrough_filter_rt).bind(p_filter));
}

void OpenXRFbPassthroughExtensionWrapper::_set_passthrough_filter_rt(PassthroughFilter p_filter) {
	switch (p_filter) {
		case PASSTHROUGH_FILTER_DISABLED: {
			render_state.passthrough_style.next = nullptr;
		} break;
		case PASSTHROUGH_FILTER_COLOR_MAP: {
			render_state.passthrough_style.next = &render_state.color_map;
		} break;
		case PASSTHROUGH_FILTER_MONO_MAP: {
			render_state.passthrough_style.next = &render_state.mono_map;
		} break;
		case PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION: {
			render_state.passthrough_style.next = &render_state.brightness_contrast_saturation;
		} break;
		case PASSTHROUGH_FILTER_COLOR_MAP_LUT: {
			if (render_state.color_lut_handle == XR_NULL_HANDLE) {
				UtilityFunctions::print("Cannot set filter to color map LUT, color LUT has not been previously set");
				return;
			}
			render_state.passthrough_style.next = &render_state.color_map_lut;
		} break;
		case PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT: {
			if (render_state.source_color_lut_handle == XR_NULL_HANDLE || render_state.target_color_lut_handle == XR_NULL_HANDLE) {
				UtilityFunctions::print("Cannot set filter to color map interpolated LUT, interpolated color LUT has not been previously set");
				return;
			}
			render_state.passthrough_style.next = &render_state.color_map_interpolated_lut;
		} break;
	}

	render_state.current_passthrough_filter = p_filter;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_color_map(const Ref<Gradient> &p_gradient) {
	ERR_FAIL_COND(p_gradient.is_null());
	current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_color_map_rt).bind(p_gradient));
}

void OpenXRFbPassthroughExtensionWrapper::_set_color_map_rt(const Ref<Gradient> &p_gradient) {
	for (int i = 0; i < XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB; i++) {
		Color sample_color = p_gradient->sample((double)i / (double)XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB);
		render_state.color_map.textureColorMap[i] = { sample_color.r, sample_color.g, sample_color.b, sample_color.a };
	}

	render_state.current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP;
	render_state.passthrough_style.next = &render_state.color_map;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_mono_map(const Ref<Curve> &p_curve) {
	ERR_FAIL_COND(p_curve.is_null());
	current_passthrough_filter = PASSTHROUGH_FILTER_MONO_MAP;
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_mono_map_rt).bind(p_curve));
}

void OpenXRFbPassthroughExtensionWrapper::_set_mono_map_rt(const Ref<Curve> &p_curve) {
	for (int i = 0; i < XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB; i++) {
		render_state.mono_map.textureColorMap[i] = p_curve->sample((double)i / (double)XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB) * XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB;
	}

	render_state.current_passthrough_filter = PASSTHROUGH_FILTER_MONO_MAP;
	render_state.passthrough_style.next = &render_state.mono_map;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_brightness_contrast_saturation(float p_brightness, float p_contrast, float p_saturation) {
	const float BRIGHT_MIN = -100.0;
	const float BRIGHT_MAX = 100.0;
	ERR_FAIL_COND_MSG(p_brightness < BRIGHT_MIN || p_brightness > BRIGHT_MAX, vformat("Brighness value %d is not within bounds of %d and %d", p_brightness, BRIGHT_MIN, BRIGHT_MAX));
	ERR_FAIL_COND_MSG(p_contrast < 0.0, vformat("Contrast value %d is not greater than or equal to zero", p_contrast));
	ERR_FAIL_COND_MSG(p_saturation < 0.0, vformat("Saturation value %d is not greater than or equal to zero", p_saturation));

	current_passthrough_filter = PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION;

	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_brightness_contrast_saturation_rt).bind(p_brightness, p_contrast, p_saturation));
}

void OpenXRFbPassthroughExtensionWrapper::_set_brightness_contrast_saturation_rt(float p_brightness, float p_contrast, float p_saturation) {
	render_state.brightness_contrast_saturation.brightness = p_brightness;
	render_state.brightness_contrast_saturation.contrast = p_contrast;
	render_state.brightness_contrast_saturation.saturation = p_saturation;

	render_state.current_passthrough_filter = PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION;
	render_state.passthrough_style.next = &render_state.brightness_contrast_saturation;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

bool OpenXRFbPassthroughExtensionWrapper::has_passthrough_capability() {
	return system_passthrough_properties.capabilities & XR_PASSTHROUGH_CAPABILITY_BIT_FB;
}

bool OpenXRFbPassthroughExtensionWrapper::has_color_passthrough_capability() {
	return (system_passthrough_properties.capabilities & XR_PASSTHROUGH_CAPABILITY_BIT_FB) && (system_passthrough_properties.capabilities & XR_PASSTHROUGH_CAPABILITY_COLOR_BIT_FB);
}

bool OpenXRFbPassthroughExtensionWrapper::has_layer_depth_passthrough_capability() {
	return (system_passthrough_properties.capabilities & XR_PASSTHROUGH_CAPABILITY_BIT_FB) && (system_passthrough_properties.capabilities & XR_PASSTHROUGH_CAPABILITY_LAYER_DEPTH_BIT_FB);
}

bool OpenXRFbPassthroughExtensionWrapper::is_passthrough_preferred() {
	if (!meta_passthrough_preferences_ext) {
		UtilityFunctions::print("Meta passthrough preferences extension is not enabled");
		return false;
	}

	XrPassthroughPreferencesMETA passthrough_preferences = {
		XR_TYPE_PASSTHROUGH_PREFERENCES_META, // type
		nullptr, // next
		0, // flags
	};

	XrResult result = xrGetPassthroughPreferencesMETA(SESSION, &passthrough_preferences);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to get passthrough preferences, error code: ", result);
		return false;
	}

	return passthrough_preferences.flags & XR_PASSTHROUGH_PREFERENCE_DEFAULT_TO_ACTIVE_BIT_META;
}

void OpenXRFbPassthroughExtensionWrapper::set_color_lut(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_color_lut) {
	if (!meta_passthrough_color_lut_ext) {
		UtilityFunctions::print("Passthrough color LUT extension not enabled!");
		return;
	}

	current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP_LUT;

	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_color_lut_rt).bind(p_weight, p_color_lut));
}

void OpenXRFbPassthroughExtensionWrapper::_set_color_lut_rt(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_color_lut) {
	render_state.color_lut_handle = _color_lut_get_handle_rt(p_color_lut->get_handle());

	render_state.current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP_LUT;
	render_state.color_map_lut.colorLut = render_state.color_lut_handle;
	render_state.color_map_lut.weight = CLAMP(p_weight, 0.0, 1.0);
	render_state.passthrough_style.next = &render_state.color_map_lut;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_interpolated_color_lut(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_source_color_lut, const Ref<OpenXRMetaPassthroughColorLut> &p_target_color_lut) {
	if (!meta_passthrough_color_lut_ext) {
		UtilityFunctions::print("Passthrough color LUT extension not enabled!");
		return;
	}

	ERR_FAIL_COND(p_source_color_lut.is_null());
	ERR_FAIL_COND(p_target_color_lut.is_null());

	current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT;

	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_set_interpolated_color_lut_rt).bind(p_weight, p_source_color_lut, p_target_color_lut));
}

void OpenXRFbPassthroughExtensionWrapper::_set_interpolated_color_lut_rt(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_source_color_lut, const Ref<OpenXRMetaPassthroughColorLut> &p_target_color_lut) {
	render_state.source_color_lut_handle = _color_lut_get_handle_rt(p_source_color_lut->get_handle());
	render_state.target_color_lut_handle = _color_lut_get_handle_rt(p_target_color_lut->get_handle());

	render_state.current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT;
	render_state.color_map_interpolated_lut.sourceColorLut = render_state.source_color_lut_handle;
	render_state.color_map_interpolated_lut.targetColorLut = render_state.target_color_lut_handle;
	render_state.color_map_interpolated_lut.weight = CLAMP(p_weight, 0.0, 1.0);
	render_state.passthrough_style.next = &render_state.color_map_interpolated_lut;

	if (render_state.passthrough_started) {
		XrResult result = xrPassthroughLayerSetStyleFB(render_state.passthrough_layer[render_state.current_passthrough_layer], &render_state.passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to set passthrough style, error code: ", result);
		}
	}
}

RID OpenXRFbPassthroughExtensionWrapper::color_lut_create(OpenXRMetaPassthroughColorLut::ColorLutChannels p_channels, uint32_t p_image_cell_resolution, const PackedByteArray &p_buffer) {
	if (p_image_cell_resolution > system_passthrough_color_lut_properties.maxColorLutResolution) {
		UtilityFunctions::print("Color LUT cell resolution cannot be greater than the maximum resolution supported by this system: ", system_passthrough_color_lut_properties.maxColorLutResolution);
		return RID();
	}

	XrPassthroughColorLutChannelsMETA channels;
	switch (p_channels) {
		case OpenXRMetaPassthroughColorLut::COLOR_LUT_CHANNELS_RGB: {
			channels = XR_PASSTHROUGH_COLOR_LUT_CHANNELS_RGB_META;
		} break;
		case OpenXRMetaPassthroughColorLut::COLOR_LUT_CHANNELS_RGBA: {
			channels = XR_PASSTHROUGH_COLOR_LUT_CHANNELS_RGBA_META;
		} break;
	}

	return color_luts.make_rid({ channels, p_image_cell_resolution, p_buffer });
}

XrPassthroughColorLutMETA OpenXRFbPassthroughExtensionWrapper::_color_lut_get_handle_rt(RID p_color_lut) {
	ColorLut *color_lut = color_luts.get_or_null(p_color_lut);
	if (color_lut == nullptr) {
		return XR_NULL_HANDLE;
	}

	if (color_lut->handle != XR_NULL_HANDLE) {
		return color_lut->handle;
	}

	XrPassthroughColorLutDataMETA color_lut_data = {
		(uint32_t)color_lut->buffer.size(), // bufferSize
		color_lut->buffer.ptr(), // buffer
	};

	XrPassthroughColorLutCreateInfoMETA color_lut_create_info = {
		XR_TYPE_PASSTHROUGH_COLOR_LUT_CREATE_INFO_META, // type
		nullptr, // next
		color_lut->channels, // channels
		color_lut->image_cell_resolution, // resolution
		color_lut_data, // data
	};

	XrResult result = xrCreatePassthroughColorLutMETA(render_state.passthrough_handle, &color_lut_create_info, &color_lut->handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create passthrough color LUT, error code: ", result);
		return XR_NULL_HANDLE;
	}

	return color_lut->handle;
}

void OpenXRFbPassthroughExtensionWrapper::color_lut_free(RID p_color_lut) {
	RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &OpenXRFbPassthroughExtensionWrapper::_color_lut_free_rt).bind(p_color_lut));
}

void OpenXRFbPassthroughExtensionWrapper::_color_lut_free_rt(RID p_color_lut) {
	ColorLut *color_lut = color_luts.get_or_null(p_color_lut);
	XrPassthroughColorLutMETA handle = color_lut ? color_lut->handle : XR_NULL_HANDLE;

	if (handle == XR_NULL_HANDLE) {
		UtilityFunctions::print("Cannot delete invalid color LUT");
		return;
	}

	XrResult result = xrDestroyPassthroughColorLutMETA(handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to destroy passthrough color LUT, error code: ", result);
		return;
	}

	if (render_state.color_lut_handle == handle) {
		render_state.color_lut_handle = XR_NULL_HANDLE;
		if (render_state.current_passthrough_filter == PASSTHROUGH_FILTER_COLOR_MAP_LUT) {
			_set_passthrough_filter_rt(PASSTHROUGH_FILTER_DISABLED);
		}
	}
	if (render_state.source_color_lut_handle == handle) {
		render_state.source_color_lut_handle = XR_NULL_HANDLE;
		if (render_state.current_passthrough_filter == PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT) {
			_set_passthrough_filter_rt(PASSTHROUGH_FILTER_DISABLED);
		}
	}
	if (render_state.target_color_lut_handle == handle) {
		render_state.target_color_lut_handle = XR_NULL_HANDLE;
		if (render_state.current_passthrough_filter == PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT) {
			_set_passthrough_filter_rt(PASSTHROUGH_FILTER_DISABLED);
		}
	}

	color_luts.free(p_color_lut);
}

int OpenXRFbPassthroughExtensionWrapper::get_max_color_lut_resolution() {
	if (!meta_passthrough_color_lut_ext) {
		UtilityFunctions::print("Passthrough color LUT extension not enabled!");
		return 0;
	}

	return system_passthrough_color_lut_properties.maxColorLutResolution;
}

XRInterface::EnvironmentBlendMode OpenXRFbPassthroughExtensionWrapper::get_blend_mode() {
	Ref<XRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	if (xr_interface.is_valid()) {
		return xr_interface->get_environment_blend_mode();
	}
	return XRInterface::XR_ENV_BLEND_MODE_OPAQUE;
}

bool OpenXRFbPassthroughExtensionWrapper::initialize_fb_passthrough_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePassthroughFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyPassthroughFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughStartFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughPauseFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePassthroughLayerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyPassthroughLayerFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughLayerPauseFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughLayerResumeFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrPassthroughLayerSetStyleFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateGeometryInstanceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyGeometryInstanceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGeometryInstanceSetTransformFB);

	return true;
}

bool OpenXRFbPassthroughExtensionWrapper::initialize_fb_triangle_mesh_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateTriangleMeshFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyTriangleMeshFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshGetVertexBufferFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshGetIndexBufferFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshBeginUpdateFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshEndUpdateFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshBeginVertexBufferUpdateFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrTriangleMeshEndVertexBufferUpdateFB);

	return true;
}

bool OpenXRFbPassthroughExtensionWrapper::initialize_meta_passthrough_preferences_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetPassthroughPreferencesMETA);

	return true;
}

bool OpenXRFbPassthroughExtensionWrapper::initialize_meta_passthrough_color_lut_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreatePassthroughColorLutMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyPassthroughColorLutMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrUpdatePassthroughColorLutMETA);

	return true;
}
