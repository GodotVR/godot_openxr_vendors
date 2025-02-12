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

godot::Dictionary OpenXRFbPassthroughExtensionWrapper::_get_requested_extensions() {
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
		// If it's already supported, then we don't want to emulate it.
		if (get_openxr_api()->is_environment_blend_mode_alpha_supported() != OpenXRAPIExtension::OPENXR_ALPHA_BLEND_MODE_SUPPORT_NONE) {
			return;
		}

		// Create the passthrough feature and start it.
		XrPassthroughCreateInfoFB passthrough_create_info = {
			XR_TYPE_PASSTHROUGH_CREATE_INFO_FB, // type
			nullptr, // next
			0, // flags
		};

		XrResult result = xrCreatePassthroughFB(session, &passthrough_create_info, &passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create passthrough");
			passthrough_handle = XR_NULL_HANDLE;
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
	} else if (blend_mode != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && passthrough_geometry_nodes.size() > 0 && current_passthrough_layer != LAYER_PURPOSE_PROJECTED) {
		start_passthrough_layer(LAYER_PURPOSE_PROJECTED);
	} else if (blend_mode != XRInterface::XR_ENV_BLEND_MODE_ALPHA_BLEND && passthrough_geometry_nodes.size() == 0 && current_passthrough_layer != LAYER_PURPOSE_NONE) {
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
		stop_passthrough();

		XrResult result;
		if (passthrough_handle != XR_NULL_HANDLE) {
			result = xrDestroyPassthroughFB(passthrough_handle);
			if (XR_FAILED(result)) {
				UtilityFunctions::print("Unable to destroy passthrough feature");
			}
			passthrough_handle = XR_NULL_HANDLE;

			get_openxr_api()->unregister_composition_layer_provider(this);
			get_openxr_api()->set_emulate_environment_blend_mode_alpha_blend(false);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRFbPassthroughExtensionWrapper::register_geometry_node(OpenXRFbPassthroughGeometry *p_node) {
	passthrough_geometry_nodes.append(p_node);
}

void OpenXRFbPassthroughExtensionWrapper::unregister_geometry_node(OpenXRFbPassthroughGeometry *p_node) {
	passthrough_geometry_nodes.erase(p_node);
}

void OpenXRFbPassthroughExtensionWrapper::start_passthrough() {
	if (passthrough_handle == XR_NULL_HANDLE) {
		UtilityFunctions::print("Cannot start passthrough before passthrough handle is created");
		return;
	}

	XrResult result = xrPassthroughStartFB(passthrough_handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to start passthrough, error code: ", result);
		stop_passthrough();
		return;
	}

	passthrough_started = true;
}

void OpenXRFbPassthroughExtensionWrapper::start_passthrough_layer(LayerPurpose p_layer_purpose) {
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
			return;
	}

	// If passthrough hasn't started, start it.
	if (!is_passthrough_started()) {
		start_passthrough();
		if (!is_passthrough_started()) {
			return;
		}
	}

	// If a different layer is active, pause it.
	if (current_passthrough_layer >= 0 && current_passthrough_layer != p_layer_purpose) {
		XrResult result = xrPassthroughLayerPauseFB(passthrough_layer[current_passthrough_layer]);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to pause current passthrough layer, error code: ", result);
			return;
		}
	}

	// If layer does not exist, create it.
	if (passthrough_layer[p_layer_purpose] == XR_NULL_HANDLE) {
		XrPassthroughLayerCreateInfoFB passthrough_layer_config = {
			XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB, // type
			nullptr, // next
			passthrough_handle, // passthrough
			XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB, // flags
			xr_layer_purpose, // purpose
		};

		XrResult result = xrCreatePassthroughLayerFB(SESSION, &passthrough_layer_config, &passthrough_layer[p_layer_purpose]);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to create passthrough layer ", p_layer_purpose, ", error code: ", result);
			stop_passthrough();
			return;
		}

		if (p_layer_purpose == LAYER_PURPOSE_PROJECTED) {
			emit_signal("openxr_fb_projected_passthrough_layer_created");
		}
	} else { // Else resume already created layer.
		XrResult result = xrPassthroughLayerResumeFB(passthrough_layer[p_layer_purpose]);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to resume passthrough layer ", p_layer_purpose, ", error code: ", result);
			return;
		}
	}

	current_passthrough_layer = p_layer_purpose;

	// Apply passthrough style to layer
	XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
	}
}

int OpenXRFbPassthroughExtensionWrapper::_get_composition_layer_count() {
	return is_passthrough_started() ? 1 : 0;
}

uint64_t OpenXRFbPassthroughExtensionWrapper::_get_composition_layer(int p_index) {
	if (p_index == 0) {
		composition_passthrough_layer.layerHandle = passthrough_layer[current_passthrough_layer];
		return reinterpret_cast<uint64_t>(&composition_passthrough_layer);
	} else {
		return 0;
	}
}

int OpenXRFbPassthroughExtensionWrapper::_get_composition_layer_order(int p_index) {
	// Ensure the passthrough layer will be behind the projection layer.
	return -100;
}

void OpenXRFbPassthroughExtensionWrapper::stop_passthrough() {
	if (!fb_passthrough_ext) {
		return;
	}

	XrResult result;
	for (int i = 0; i < LAYER_PURPOSE_MAX; i++) {
		if (passthrough_layer[i] != XR_NULL_HANDLE) {
			result = xrDestroyPassthroughLayerFB(passthrough_layer[i]);
			if (XR_FAILED(result)) {
				UtilityFunctions::print("Unable to destroy passthrough layer, error code: ", result);
			}
			passthrough_layer[i] = XR_NULL_HANDLE;
		}
	}

	if (passthrough_handle != XR_NULL_HANDLE) {
		result = xrPassthroughPauseFB(passthrough_handle);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Unable to stop passthrough feature, error code: ", result);
			return;
		}
	}

	current_passthrough_layer = LAYER_PURPOSE_NONE;
	passthrough_started = false;
	emit_signal("openxr_fb_passthrough_stopped");
}

XrGeometryInstanceFB OpenXRFbPassthroughExtensionWrapper::create_geometry_instance(const Ref<Mesh> &p_mesh, const Transform3D &p_transform) {
	ERR_FAIL_COND_V(p_mesh.is_null(), XR_NULL_HANDLE);

	if (!is_passthrough_started()) {
		UtilityFunctions::print("Tried to create geometry instance, but passthrough isn't started!");
		return XR_NULL_HANDLE;
	}

	Array surface_arrays = p_mesh->surface_get_arrays(0);

	Array vertex_array = surface_arrays[Mesh::ARRAY_VERTEX];
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

	Array index_array = surface_arrays[Mesh::ARRAY_INDEX];
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
		return XR_NULL_HANDLE;
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

	XrGeometryInstanceFB geometry_instance = XR_NULL_HANDLE;
	XrGeometryInstanceCreateInfoFB geometry_instance_info = {
		XR_TYPE_GEOMETRY_INSTANCE_CREATE_INFO_FB, // type
		nullptr, // next
		passthrough_layer[LAYER_PURPOSE_PROJECTED], // layer
		mesh, // mesh
		(XrSpace)get_openxr_api()->get_play_space(), // baseSpace
		xr_pose, // pose
		xr_scale, // scale
	};

	result = xrCreateGeometryInstanceFB(SESSION, &geometry_instance_info, &geometry_instance);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to create geometry instance, error code: ", result);
		return XR_NULL_HANDLE;
	}

	return geometry_instance;
}

void OpenXRFbPassthroughExtensionWrapper::set_geometry_instance_transform(XrGeometryInstanceFB p_geometry_instance, const Transform3D &p_transform) {
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

	XrResult result = xrGeometryInstanceSetTransformFB(p_geometry_instance, &xr_transform);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to set geometry instance transform, error code: ", result);
	}
}

void OpenXRFbPassthroughExtensionWrapper::destroy_geometry_instance(XrGeometryInstanceFB p_geometry_instance) {
	XrResult result = xrDestroyGeometryInstanceFB(p_geometry_instance);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("Failed to destroy geometry instance, error code: ", result);
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_texture_opacity_factor(float p_value) {
	passthrough_style.textureOpacityFactor = p_value;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

float OpenXRFbPassthroughExtensionWrapper::get_texture_opacity_factor() {
	return passthrough_style.textureOpacityFactor;
}

void OpenXRFbPassthroughExtensionWrapper::set_edge_color(Color p_color) {
	passthrough_style.edgeColor = { p_color.r, p_color.g, p_color.b, p_color.a };

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

Color OpenXRFbPassthroughExtensionWrapper::get_edge_color() {
	return { passthrough_style.edgeColor.r, passthrough_style.edgeColor.g, passthrough_style.edgeColor.b, passthrough_style.edgeColor.a };
}

void OpenXRFbPassthroughExtensionWrapper::set_passthrough_filter(PassthroughFilter p_filter) {
	switch (p_filter) {
		case PASSTHROUGH_FILTER_DISABLED: {
			passthrough_style.next = nullptr;
		} break;
		case PASSTHROUGH_FILTER_COLOR_MAP: {
			passthrough_style.next = &color_map;
		} break;
		case PASSTHROUGH_FILTER_MONO_MAP: {
			passthrough_style.next = &mono_map;
		} break;
		case PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION: {
			passthrough_style.next = &brightness_contrast_saturation;
		} break;
		case PASSTHROUGH_FILTER_COLOR_MAP_LUT: {
			if (color_lut_handle == XR_NULL_HANDLE) {
				UtilityFunctions::print("Cannot set filter to color map LUT, color LUT has not been previously set");
				return;
			}
			passthrough_style.next = &color_map_lut;
		} break;
		case PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT: {
			if (source_color_lut_handle == XR_NULL_HANDLE || target_color_lut_handle == XR_NULL_HANDLE) {
				UtilityFunctions::print("Cannot set filter to color map interpolated LUT, interpolated color LUT has not been previously set");
				return;
			}
			passthrough_style.next = &color_map_interpolated_lut;
		} break;
	}

	current_passthrough_filter = p_filter;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_color_map(const Ref<Gradient> &p_gradient) {
	if (p_gradient.is_null()) {
		return;
	}

	for (int i = 0; i < XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB; i++) {
		Color sample_color = p_gradient->sample((double)i / (double)XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB);
		color_map.textureColorMap[i] = { sample_color.r, sample_color.g, sample_color.b, sample_color.a };
	}

	current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP;
	passthrough_style.next = &color_map;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::set_mono_map(const Ref<Curve> &p_curve) {
	if (p_curve.is_null()) {
		return;
	}

	for (int i = 0; i < XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB; i++) {
		mono_map.textureColorMap[i] = p_curve->sample((double)i / (double)XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB) * XR_PASSTHROUGH_COLOR_MAP_MONO_SIZE_FB;
	}

	current_passthrough_filter = PASSTHROUGH_FILTER_MONO_MAP;
	passthrough_style.next = &mono_map;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
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

	brightness_contrast_saturation.brightness = p_brightness;
	brightness_contrast_saturation.contrast = p_contrast;
	brightness_contrast_saturation.saturation = p_saturation;

	current_passthrough_filter = PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION;
	passthrough_style.next = &brightness_contrast_saturation;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
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

	if (p_color_lut->get_handle() == XR_NULL_HANDLE) {
		create_color_lut(p_color_lut);
	}

	color_lut_handle = p_color_lut->get_handle();

	current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP_LUT;
	color_map_lut.colorLut = color_lut_handle;
	color_map_lut.weight = CLAMP(p_weight, 0.0, 1.0);
	passthrough_style.next = &color_map_lut;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
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

	if (!p_source_color_lut->get_handle()) {
		create_color_lut(p_source_color_lut);
	}

	if (!p_target_color_lut->get_handle()) {
		create_color_lut(p_target_color_lut);
	}

	source_color_lut_handle = p_source_color_lut->get_handle();
	target_color_lut_handle = p_target_color_lut->get_handle();

	current_passthrough_filter = PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT;
	color_map_interpolated_lut.sourceColorLut = source_color_lut_handle;
	color_map_interpolated_lut.targetColorLut = target_color_lut_handle;
	color_map_interpolated_lut.weight = CLAMP(p_weight, 0.0, 1.0);
	passthrough_style.next = &color_map_interpolated_lut;

	if (is_passthrough_started()) {
		XrResult result = xrPassthroughLayerSetStyleFB(passthrough_layer[current_passthrough_layer], &passthrough_style);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to set passthrough style, error code: ", result);
		}
	}
}

void OpenXRFbPassthroughExtensionWrapper::create_color_lut(const Ref<OpenXRMetaPassthroughColorLut> &p_color_lut) {
	if (p_color_lut->get_image_cell_resolution() > system_passthrough_color_lut_properties.maxColorLutResolution) {
		UtilityFunctions::print("Color LUT cell resolution cannot be greater than the maximum resolution supported by this system: ", system_passthrough_color_lut_properties.maxColorLutResolution);
		return;
	}

	XrPassthroughColorLutChannelsMETA channels;
	switch (p_color_lut->get_channels()) {
		case OpenXRMetaPassthroughColorLut::COLOR_LUT_CHANNELS_RGB: {
			channels = XR_PASSTHROUGH_COLOR_LUT_CHANNELS_RGB_META;
		} break;
		case OpenXRMetaPassthroughColorLut::COLOR_LUT_CHANNELS_RGBA: {
			channels = XR_PASSTHROUGH_COLOR_LUT_CHANNELS_RGBA_META;
		} break;
	}

	XrPassthroughColorLutDataMETA color_lut_data = {
		(uint32_t)p_color_lut->get_buffer().size(), // bufferSize
		p_color_lut->get_buffer().ptr(), // buffer
	};

	XrPassthroughColorLutCreateInfoMETA color_lut_create_info = {
		XR_TYPE_PASSTHROUGH_COLOR_LUT_CREATE_INFO_META, // type
		nullptr, // next
		channels, // channels
		(uint32_t)p_color_lut->get_image_cell_resolution(), // resolution
		color_lut_data, // data
	};

	XrPassthroughColorLutMETA handle;
	XrResult result = xrCreatePassthroughColorLutMETA(passthrough_handle, &color_lut_create_info, &handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create passthrough color LUT, error code: ", result);
		return;
	}

	p_color_lut->set_handle(handle);
}

void OpenXRFbPassthroughExtensionWrapper::destroy_color_lut(const Ref<OpenXRMetaPassthroughColorLut> &p_color_lut) {
	XrPassthroughColorLutMETA handle = p_color_lut->get_handle();

	if (handle == XR_NULL_HANDLE) {
		UtilityFunctions::print("Cannot delete invalid color LUT");
		return;
	}

	XrResult result = xrDestroyPassthroughColorLutMETA(handle);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to destroy passthrough color LUT, error code: ", result);
		return;
	}

	if (color_lut_handle == handle) {
		color_lut_handle = XR_NULL_HANDLE;
	}
	if (source_color_lut_handle == handle) {
		source_color_lut_handle = XR_NULL_HANDLE;
	}
	if (target_color_lut_handle == handle) {
		target_color_lut_handle = XR_NULL_HANDLE;
	}

	p_color_lut->set_handle(XR_NULL_HANDLE);
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
