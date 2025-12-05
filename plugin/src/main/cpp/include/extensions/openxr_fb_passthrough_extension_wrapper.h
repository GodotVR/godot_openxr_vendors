/**************************************************************************/
/*  openxr_fb_passthrough_extension_wrapper.h                             */
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

#ifndef OPENXR_FB_PASSTHROUGH_EXTENSION_WRAPPER_H
#define OPENXR_FB_PASSTHROUGH_EXTENSION_WRAPPER_H

#include "classes/openxr_fb_passthrough_geometry.h"
#include "classes/openxr_meta_passthrough_color_lut.h"

#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/gradient.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openxr/openxr.h>

#include "util.h"

#include <map>

using namespace godot;

// Wrapper for the set of Facebook XR passthrough extensions.
class OpenXRFbPassthroughExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbPassthroughExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	enum LayerPurpose {
		LAYER_PURPOSE_NONE = -1,
		LAYER_PURPOSE_RECONSTRUCTION,
		LAYER_PURPOSE_PROJECTED,
		LAYER_PURPOSE_MAX,
	};

	enum PassthroughFilter {
		PASSTHROUGH_FILTER_DISABLED,
		PASSTHROUGH_FILTER_COLOR_MAP,
		PASSTHROUGH_FILTER_MONO_MAP,
		PASSTHROUGH_FILTER_BRIGHTNESS_CONTRAST_SATURATION,
		PASSTHROUGH_FILTER_COLOR_MAP_LUT,
		PASSTHROUGH_FILTER_COLOR_MAP_INTERPOLATED_LUT,
	};

	enum PassthroughStateChangedEvent {
		PASSTHROUGH_ERROR_NON_RECOVERABLE,
		PASSTHROUGH_ERROR_RECOVERABLE,
		PASSTHROUGH_ERROR_RESTORED,
	};

	OpenXRFbPassthroughExtensionWrapper();
	~OpenXRFbPassthroughExtensionWrapper();

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	void _on_instance_created(uint64_t p_instance) override;
	void _on_session_created(uint64_t p_session) override;
	void _on_session_destroyed() override;
	void _on_instance_destroyed() override;
	void _on_state_ready() override;
	void _on_process() override;
	bool _on_event_polled(const void *p_event) override;

	int _get_composition_layer_count() override;
	uint64_t _get_composition_layer(int p_index) override;
	int _get_composition_layer_order(int p_index) override;

	bool is_passthrough_supported() {
		return fb_passthrough_ext;
	}

	bool is_passthrough_started() {
		return passthrough_started;
	}

	void start_passthrough();
	void stop_passthrough();

	void start_passthrough_layer(LayerPurpose p_layer_purpose);
	LayerPurpose get_current_layer_purpose() { return current_passthrough_layer; }

	RID geometry_instance_create(const Array &p_array_mesh, const Transform3D &p_transform);
	void geometry_instance_set_transform(RID p_geometry_instance, const Transform3D &p_transform);
	void geometry_instance_free(RID p_geometry_instance);

	RID color_lut_create(OpenXRMetaPassthroughColorLut::ColorLutChannels p_channels, uint32_t p_image_cell_resolution, const PackedByteArray &p_buffer);
	void color_lut_free(RID p_color_lut);

	void set_texture_opacity_factor(float p_value);
	float get_texture_opacity_factor();

	void set_edge_color(Color p_color);
	Color get_edge_color();

	void set_passthrough_filter(PassthroughFilter p_filter);
	PassthroughFilter get_current_passthrough_filter() { return current_passthrough_filter; }
	void set_color_map(const Ref<Gradient> &p_gradient);
	void set_mono_map(const Ref<Curve> &p_curve);
	void set_brightness_contrast_saturation(float p_brightness, float p_contrast, float p_saturation);

	bool has_passthrough_capability();
	bool has_color_passthrough_capability();
	bool has_layer_depth_passthrough_capability();

	bool is_passthrough_preferred();

	void set_color_lut(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_color_lut);
	void set_interpolated_color_lut(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_source_color_lut, const Ref<OpenXRMetaPassthroughColorLut> &p_target_color_lut);
	int get_max_color_lut_resolution();

	static OpenXRFbPassthroughExtensionWrapper *get_singleton();

protected:
	static void _bind_methods();

private:
	// Create a passthrough feature
	EXT_PROTO_XRRESULT_FUNC3(xrCreatePassthroughFB,
			(XrSession), session,
			(const XrPassthroughCreateInfoFB *), create_info,
			(XrPassthroughFB *), feature_out)

	// Destroy a previously created passthrough feature
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyPassthroughFB, (XrPassthroughFB), feature)

	//*** Passthrough feature state management functions *********
	// Start the passthrough feature
	EXT_PROTO_XRRESULT_FUNC1(xrPassthroughStartFB, (XrPassthroughFB), passthrough)
	// Pause the passthrough feature
	EXT_PROTO_XRRESULT_FUNC1(xrPassthroughPauseFB, (XrPassthroughFB), passthrough)

	EXT_PROTO_XRRESULT_FUNC3(xrCreatePassthroughLayerFB, (XrSession), session,
			(const XrPassthroughLayerCreateInfoFB *), config,
			(XrPassthroughLayerFB *), layer_out)

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyPassthroughLayerFB, (XrPassthroughLayerFB), layer)

	EXT_PROTO_XRRESULT_FUNC1(xrPassthroughLayerPauseFB, (XrPassthroughLayerFB), layer)
	EXT_PROTO_XRRESULT_FUNC1(xrPassthroughLayerResumeFB, (XrPassthroughLayerFB), layer)

	// Set the style of an existing passthrough layer. If the enabled feature set
	// doesn’t change, this is a lightweight operation that can be called in every
	// frame to animate the style. Changes that may incur a bigger cost:
	// - Enabling/disabling the color mapping, or changing the type of mapping
	//   (monochromatic to RGBA or back).
	// - Changing `textureOpacityFactor` from 0 to non-zero or vice versa
	// - Changing `edgeColor[3]` from 0 to non-zero or vice versa
	// NOTE: For XR_FB_passthrough, all color values are treated as linear.
	EXT_PROTO_XRRESULT_FUNC2(xrPassthroughLayerSetStyleFB,
			(XrPassthroughLayerFB), layer,
			(const XrPassthroughStyleFB *), style)

	// Create a geometry instance to be used as a projection surface for passthrough.
	// A geometry instance assigns a triangle mesh as part of the specified layer's
	// projection surface.
	// The operation is only valid if the passthrough layer's purpose has been set to
	// `XR_PASSTHROUGH_LAYER_PURPOSE_PROJECTED_FB`. Otherwise, the call this function will
	// result in an error. In the specified layer, Passthrough will be visible where the view
	// is covered by the user-specified geometries.
	//
	// A triangle mesh object can be instantiated multiple times - in the same or different layers'
	// projection surface. Each instantiation has its own transformation, which
	// can be updated using `xrGeometryInstanceSetTransformFB`.
	EXT_PROTO_XRRESULT_FUNC3(xrCreateGeometryInstanceFB,
			(XrSession), session,
			(const XrGeometryInstanceCreateInfoFB *), create_info,
			(XrGeometryInstanceFB *), out_geometry_instance)

	// Destroys a previously created geometry instance from passthrough rendering.
	// This removes the geometry instance from passthrough rendering.
	// The operation has no effect on other instances or the underlying mesh.
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyGeometryInstanceFB, (XrGeometryInstanceFB), instance)

	// Update the transformation of a passthrough geometry instance.
	EXT_PROTO_XRRESULT_FUNC2(xrGeometryInstanceSetTransformFB,
			(XrGeometryInstanceFB), instance,
			(const XrGeometryInstanceTransformFB *), transformation)

	// Create a triangle mesh geometry object.
	// Depending on the behavior flags, the mesh could be created immutable (data is assigned
	// at creation and cannot be changed) or mutable (the mesh is created empty and can be updated
	// by calling begin/end update functions).
	EXT_PROTO_XRRESULT_FUNC3(xrCreateTriangleMeshFB,
			(XrSession), session,
			(const XrTriangleMeshCreateInfoFB *), create_info,
			(XrTriangleMeshFB *), out_triangle_mesh)

	// Destroy an `XrTriangleMeshFB` object along with its data. The mesh buffers must not be
	// accessed anymore after their parent mesh object has been destroyed.
	EXT_PROTO_XRRESULT_FUNC1(xrDestroyTriangleMeshFB, (XrTriangleMeshFB), mesh)

	// Retrieve a pointer to the vertex buffer. The vertex buffer is structured as an array of 3 floats
	// per vertex representing x, y, and z: `[x0, y0, z0, x1, y1, z1, ...]`. The size of the buffer is
	// `maxVertexCount * 3` floats. The application must call `xrTriangleMeshBeginUpdateFB` or
	// `xrTriangleMeshBeginVertexBufferUpdateFB` before making modifications to the vertex
	// buffer. The buffer location is guaranteed to remain constant over the lifecycle of the mesh
	// object.
	EXT_PROTO_XRRESULT_FUNC2(xrTriangleMeshGetVertexBufferFB,
			(XrTriangleMeshFB), mesh,
			(XrVector3f **), out_vertex_buffer)

	// Retrieve the index buffer that defines the topology of the triangle mesh. Each triplet of
	// consecutive elements point to three vertices in the vertex buffer and thus form a triangle. The
	// size of each element is `indexElementSize` bytes, and thus the size of the buffer is
	// `maxTriangleCount * 3 * indexElementSize` bytes. The application must call
	// `xrTriangleMeshBeginUpdateFB` before making modifications to the index buffer. The buffer
	// location is guaranteed to remain constant over the lifecycle of the mesh object.
	EXT_PROTO_XRRESULT_FUNC2(xrTriangleMeshGetIndexBufferFB,
			(XrTriangleMeshFB), mesh,
			(uint32_t **), out_index_buffer)

	// Begin updating the mesh buffer data. The application must call this function before it makes any
	// modifications to the buffers retrieved by `xrTriangleMeshGetVertexBufferFB` and
	// `xrTriangleMeshGetIndexBufferFB`. If only the vertex buffer needs to be updated,
	// `xrTriangleMeshBeginVertexBufferUpdateFB` can be used instead. To commit the
	// modifications, the application must call `xrTriangleMeshEndUpdateFB`.
	EXT_PROTO_XRRESULT_FUNC1(xrTriangleMeshBeginUpdateFB, (XrTriangleMeshFB), mesh)

	// Signal the API that the application has finished updating the mesh buffers after a call to
	// `xrTriangleMeshBeginUpdateFB`. `vertexCount` and `triangleCount` specify the actual
	// number of primitives that make up the mesh after the update. They must be larger than zero but
	// smaller or equal to the maximum counts defined at create time. Buffer data beyond these counts
	// is ignored.
	EXT_PROTO_XRRESULT_FUNC3(xrTriangleMeshEndUpdateFB,
			(XrTriangleMeshFB), mesh,
			(uint32_t), vertexCount,
			(uint32_t), triangle_count)

	// Update the vertex positions of a triangle mesh. Can only be called once the mesh topology has
	// been set using `xrTriangleMeshBeginUpdateFB`/`xrTriangleMeshEndUpdateFB`. The
	// vertex count is defined by the last invocation to `xrTriangleMeshEndUpdateFB`. Once the
	// modification is done, `xrTriangleMeshEndVertexBufferUpdateFB` must be called.
	EXT_PROTO_XRRESULT_FUNC2(xrTriangleMeshBeginVertexBufferUpdateFB,
			(XrTriangleMeshFB), mesh,
			(uint32_t *), out_vertex_count)

	// Signal the API that the contents of the vertex buffer data has been updated
	// after a call to `xrTriangleMeshBeginVertexBufferUpdateFB`.
	EXT_PROTO_XRRESULT_FUNC1(xrTriangleMeshEndVertexBufferUpdateFB, (XrTriangleMeshFB), mesh)

	EXT_PROTO_XRRESULT_FUNC2(xrGetPassthroughPreferencesMETA,
			(XrSession), session,
			(XrPassthroughPreferencesMETA *), preferences)

	EXT_PROTO_XRRESULT_FUNC3(xrCreatePassthroughColorLutMETA,
			(XrPassthroughFB), passthrough,
			(const XrPassthroughColorLutCreateInfoMETA *), createInfo,
			(XrPassthroughColorLutMETA *), colorLut)

	EXT_PROTO_XRRESULT_FUNC1(xrDestroyPassthroughColorLutMETA, (XrPassthroughColorLutMETA), colorLut)

	EXT_PROTO_XRRESULT_FUNC2(xrUpdatePassthroughColorLutMETA,
			(XrPassthroughColorLutMETA), colorLut,
			(const XrPassthroughColorLutUpdateInfoMETA *), updateInfo)

	XRInterface::EnvironmentBlendMode get_blend_mode();

	bool initialize_fb_passthrough_extension(const XrInstance p_instance);
	bool initialize_fb_triangle_mesh_extension(const XrInstance p_instance);
	bool initialize_meta_passthrough_preferences_extension(const XrInstance p_instance);
	bool initialize_meta_passthrough_color_lut_extension(const XrInstance p_instance);

	void cleanup();

	static OpenXRFbPassthroughExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;

	bool fb_passthrough_ext = false; // required for any passthrough functionality
	bool fb_triangle_mesh_ext = false; // only use for projected passthrough
	bool meta_passthrough_preferences_ext = false;
	bool meta_passthrough_color_lut_ext = false;

	XrSystemPassthroughProperties2FB system_passthrough_properties = {
		XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES2_FB, // type
		nullptr, // next
		0, // capabilities
	};

	XrSystemPassthroughColorLutPropertiesMETA system_passthrough_color_lut_properties = {
		XR_TYPE_SYSTEM_PASSTHROUGH_COLOR_LUT_PROPERTIES_META, // type
		nullptr, // next
		0, // maxColorLutResolution
	};

	struct {
		XrPassthroughFB passthrough_handle = XR_NULL_HANDLE;
		XrPassthroughLayerFB passthrough_layer[LAYER_PURPOSE_MAX] = { XR_NULL_HANDLE };

		XrPassthroughColorLutMETA color_lut_handle;
		XrPassthroughColorLutMETA source_color_lut_handle;
		XrPassthroughColorLutMETA target_color_lut_handle;

		bool passthrough_started = false;
		LayerPurpose current_passthrough_layer = LAYER_PURPOSE_NONE;
		PassthroughFilter current_passthrough_filter = PASSTHROUGH_FILTER_DISABLED;

		XrPassthroughStyleFB passthrough_style = {
			XR_TYPE_PASSTHROUGH_STYLE_FB, // type
			nullptr, // next
			1.0, // textureOpacityFactor
			{ 0.0, 0.0, 0.0, 0.0 }, // edgeColor
		};

		XrPassthroughColorMapMonoToRgbaFB color_map = {
			XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_RGBA_FB, // type
			nullptr, // next
		};

		XrPassthroughColorMapMonoToMonoFB mono_map = {
			XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_MONO_FB, // type
			nullptr, // next
		};

		XrPassthroughBrightnessContrastSaturationFB brightness_contrast_saturation = {
			XR_TYPE_PASSTHROUGH_BRIGHTNESS_CONTRAST_SATURATION_FB, // type
			nullptr, // next
			0.0, // brightness
			1.0, // contrast
			1.0, // saturation
		};

		XrCompositionLayerPassthroughFB composition_passthrough_layer = {
			XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB, // type
			nullptr, // next
			XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT, // flags
			XR_NULL_HANDLE, // space
			XR_NULL_HANDLE, // layerHandle
		};

		XrPassthroughColorMapLutMETA color_map_lut = {
			XR_TYPE_PASSTHROUGH_COLOR_MAP_LUT_META, // type
			nullptr, // next
			XR_NULL_HANDLE, // colorLut
			1.0, // weight
		};

		XrPassthroughColorMapInterpolatedLutMETA color_map_interpolated_lut = {
			XR_TYPE_PASSTHROUGH_COLOR_MAP_INTERPOLATED_LUT_META, // type
			nullptr, // next
			XR_NULL_HANDLE, //sourceColorLut
			XR_NULL_HANDLE, //targetColorLut
			1.0, // weight
		};

	} render_state;

	bool passthrough_started = false;
	float texture_opacity_factor = 1.0;
	Color edge_color;
	LayerPurpose current_passthrough_layer = LAYER_PURPOSE_NONE;
	PassthroughFilter current_passthrough_filter = PASSTHROUGH_FILTER_DISABLED;

	struct GeometryInstance {
		XrGeometryInstanceFB handle = XR_NULL_HANDLE;
	};

	RID_Owner<GeometryInstance, true> geometry_instances;

	struct ColorLut {
		XrPassthroughColorLutChannelsMETA channels;
		uint32_t image_cell_resolution;
		PackedByteArray buffer;

		XrPassthroughColorLutMETA handle = XR_NULL_HANDLE;
	};

	RID_Owner<ColorLut, true> color_luts;

	void _set_passthrough_started(bool p_started) {
		passthrough_started = p_started;
	}

	void _set_current_passthrough_layer(LayerPurpose p_layer) {
		current_passthrough_layer = p_layer;
	}

	void _emit_signal(const StringName &p_name) {
		emit_signal(p_name);
	}

	void _start_passthrough_rt();
	void _stop_passthrough_rt();
	void _start_passthrough_layer_rt(LayerPurpose p_layer_purpose);

	void _set_texture_opacity_factor_rt(float p_value);
	void _set_edge_color_rt(Color p_color);
	void _set_passthrough_filter_rt(PassthroughFilter p_filter);
	void _set_color_map_rt(const Ref<Gradient> &p_gradient);
	void _set_mono_map_rt(const Ref<Curve> &p_curve);
	void _set_brightness_contrast_saturation_rt(float p_brightness, float p_contrast, float p_saturation);

	void _set_color_lut_rt(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_color_lut);
	void _set_interpolated_color_lut_rt(float p_weight, const Ref<OpenXRMetaPassthroughColorLut> &p_source_color_lut, const Ref<OpenXRMetaPassthroughColorLut> &p_target_color_lut);

	XrPassthroughColorLutMETA _color_lut_get_handle_rt(RID p_color_lut);
	void _color_lut_free_rt(RID p_color_lut);

	void _geometry_instance_initialize_rt(RID p_geometry_instance, const Array &p_array_mesh, const Transform3D &p_transform);
	void _geometry_instance_set_transform_rt(RID p_geometry_instance, const Transform3D &p_transform);
	void _geometry_instance_free_rt(RID p_geometry_instance);
};

VARIANT_ENUM_CAST(OpenXRFbPassthroughExtensionWrapper::LayerPurpose);
VARIANT_ENUM_CAST(OpenXRFbPassthroughExtensionWrapper::PassthroughFilter);
VARIANT_ENUM_CAST(OpenXRFbPassthroughExtensionWrapper::PassthroughStateChangedEvent);

#endif // OPENXR_FB_PASSTHROUGH_EXTENSION_WRAPPER_H
