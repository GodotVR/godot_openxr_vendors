/**************************************************************************/
/*  openxr_fb_environment_depth_extension_wrapper.cpp                     */
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

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openxr/openxr_platform.h>

#include "extensions/openxr_fb_environment_depth_extension_wrapper.h"

/////////////// xr_linear.h

typedef enum GraphicsAPI { GRAPHICS_VULKAN, GRAPHICS_OPENGL, GRAPHICS_OPENGL_ES, GRAPHICS_D3D, GRAPHICS_METAL } GraphicsAPI;

typedef struct XrMatrix4x4f {
	float m[16];
} XrMatrix4x4f;

static void XrPosef_Invert(XrPosef* result, const XrPosef* a);
static void XrMatrix4x4f_CreateFromRigidTransform(XrMatrix4x4f* result, const XrPosef* s);
static void XrMatrix4x4f_CreateProjectionFov(XrMatrix4x4f* result, GraphicsAPI graphicsApi, const XrFovf fov, const float nearZ, const float farZ);

/////////////// /xr_linear.h

using namespace godot;

OpenXRFbEnvironmentDepthExtensionWrapper *OpenXRFbEnvironmentDepthExtensionWrapper::singleton = nullptr;

OpenXRFbEnvironmentDepthExtensionWrapper *OpenXRFbEnvironmentDepthExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbEnvironmentDepthExtensionWrapper());
	}
	return singleton;
}

OpenXRFbEnvironmentDepthExtensionWrapper::OpenXRFbEnvironmentDepthExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbEnvironmentDepthExtensionWrapper singleton already exists.");

	request_extensions[XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME] = &fb_environment_depth_ext;
	singleton = this;
}

OpenXRFbEnvironmentDepthExtensionWrapper::~OpenXRFbEnvironmentDepthExtensionWrapper() {
	cleanup();
}

void OpenXRFbEnvironmentDepthExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_is_extension_supported"), &OpenXRFbEnvironmentDepthExtensionWrapper::get_is_extension_supported);
	ClassDB::bind_method(D_METHOD("get_supports_environment_depth"), &OpenXRFbEnvironmentDepthExtensionWrapper::get_supports_environment_depth);
	ClassDB::bind_method(D_METHOD("get_supports_hand_removal"), &OpenXRFbEnvironmentDepthExtensionWrapper::get_supports_hand_removal);
	ClassDB::bind_method(D_METHOD("get_started"), &OpenXRFbEnvironmentDepthExtensionWrapper::get_started);
	ClassDB::bind_method(D_METHOD("set_started", "p_started"), &OpenXRFbEnvironmentDepthExtensionWrapper::set_started);
	ClassDB::bind_method(D_METHOD("get_enable_hand_removal"), &OpenXRFbEnvironmentDepthExtensionWrapper::get_enable_hand_removal);
	ClassDB::bind_method(D_METHOD("set_enable_hand_removal", "p_enable_hand_removal"), &OpenXRFbEnvironmentDepthExtensionWrapper::set_enable_hand_removal);
	ClassDB::bind_method(D_METHOD("prepare"), &OpenXRFbEnvironmentDepthExtensionWrapper::prepare);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_extension_supported", PROPERTY_HINT_NONE, ""), "", "get_is_extension_supported");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "supports_environment_depth", PROPERTY_HINT_NONE, ""), "", "get_supports_environment_depth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "supports_hand_removal", PROPERTY_HINT_NONE, ""), "", "get_supports_hand_removal");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "started", PROPERTY_HINT_NONE, ""), "set_started", "get_started");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_hand_removal", PROPERTY_HINT_NONE, ""), "set_enable_hand_removal", "get_enable_hand_removal");
}

void OpenXRFbEnvironmentDepthExtensionWrapper::cleanup() {
	if (swapchain != nullptr) {
		xrDestroyEnvironmentDepthSwapchainMETA(swapchain);
		swapchain = nullptr;
	}

	for (auto rid : swapchain_image_rids) {
		RenderingServer::get_singleton()->free_rid(rid);
	}

	if (provider != nullptr) {
		xrDestroyEnvironmentDepthProviderMETA(provider);
		provider = nullptr;
	}

	fb_environment_depth_ext = false;
}

uint64_t OpenXRFbEnvironmentDepthExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	system_environment_depth_properties.next = p_next_pointer;
	return reinterpret_cast<uint64_t>(&system_environment_depth_properties);
}

Dictionary OpenXRFbEnvironmentDepthExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbEnvironmentDepthExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_environment_depth_ext) {
		bool result = initialize_fb_environment_depth_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_environment_depth extension");
			fb_environment_depth_ext = false;
		}
	}
}

void OpenXRFbEnvironmentDepthExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbEnvironmentDepthExtensionWrapper::initialize_fb_environment_depth_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStartEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetEnvironmentDepthHandRemovalMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetEnvironmentDepthSwapchainStateMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateEnvironmentDepthSwapchainImagesMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrAcquireEnvironmentDepthImageMETA);

	return true;
}

bool OpenXRFbEnvironmentDepthExtensionWrapper::prepare() {
	// TODO: Verify that the shader parameters exist, if possible

	if (!fb_environment_depth_ext) {
		UtilityFunctions::print("fb_environment_depth extension not initialized");
		return false;
	}
	if (provider != nullptr) {
		return true;
	}

	XrEnvironmentDepthProviderCreateInfoMETA provider_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	XrResult result;
	result = xrCreateEnvironmentDepthProviderMETA(SESSION, &provider_create_info, &provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("xrCreateEnvironmentDepthProviderMETA failed!");
		UtilityFunctions::print(get_openxr_api()->get_error_string(result));
		return false;
	}

	XrEnvironmentDepthSwapchainCreateInfoMETA swapchain_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	result = xrCreateEnvironmentDepthSwapchainMETA(provider, &swapchain_create_info, &swapchain);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("xrCreateEnvironmentDepthSwapchainMETA failed!");
		UtilityFunctions::print(get_openxr_api()->get_error_string(result));
		xrDestroyEnvironmentDepthProviderMETA(provider);
		provider = nullptr;
		return false;
	}

	XrEnvironmentDepthSwapchainStateMETA swapchain_state = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_STATE_META, // type
		nullptr, // next
	};

	result = xrGetEnvironmentDepthSwapchainStateMETA(swapchain, &swapchain_state);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("xrGetEnvironmentDepthSwapchainStateMETA failed!");
		UtilityFunctions::print(get_openxr_api()->get_error_string(result));
		xrDestroyEnvironmentDepthSwapchainMETA(swapchain);
		swapchain = nullptr;
		xrDestroyEnvironmentDepthProviderMETA(provider);
		provider = nullptr;
		return false;
	}

	uint32_t image_count;
	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(swapchain, 0, &image_count, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("xrEnumerateEnvironmentDepthSwapchainImagesMETA failed to get count!");
		UtilityFunctions::print(get_openxr_api()->get_error_string(result));
		xrDestroyEnvironmentDepthSwapchainMETA(swapchain);
		swapchain = nullptr;
		xrDestroyEnvironmentDepthProviderMETA(provider);
		provider = nullptr;
		return false;
	}

	struct XrSwapchainImageOpenGLKHR {
		XrStructureType       type;
		void* XR_MAY_ALIAS    next;
		uint32_t              image;
	};

	Vector<XrSwapchainImageOpenGLKHR> swapchain_images;
	for (int i = 0; i < image_count; i++) {
		swapchain_images.push_back(XrSwapchainImageOpenGLKHR {
			XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR, // type
			nullptr, // next
			0, // image
		});
	}

	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(swapchain, image_count, &image_count, (XrSwapchainImageBaseHeader *)&swapchain_images[0]);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("xrEnumerateEnvironmentDepthSwapchainImagesMETA failed to get data!");
		UtilityFunctions::print(get_openxr_api()->get_error_string(result));
		xrDestroyEnvironmentDepthSwapchainMETA(swapchain);
		swapchain = nullptr;
		xrDestroyEnvironmentDepthProviderMETA(provider);
		provider = nullptr;
		return false;
	}

	swapchain_image_rids.clear();
	for (int i = 0; i < image_count; i++) {
		swapchain_image_rids.push_back(
			RenderingServer::get_singleton()->texture_create_from_native_handle(
				RenderingServer::TextureType::TEXTURE_TYPE_LAYERED,
				Image::Format::FORMAT_RH, // GL_DEPTH_COMPONENT16 according to the docs
				swapchain_images[i].image,
				swapchain_state.width,
				swapchain_state.height,
				0,
				2,
				RenderingServer::TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY
			)
		);
	}

	set_enable_hand_removal(enable_hand_removal);

	return true;
}

void OpenXRFbEnvironmentDepthExtensionWrapper::set_started(bool p_started) {
	if (started == p_started) {
		return;
	}

	XrResult result;
	if (p_started) {
		if (provider == nullptr) {
			if (!prepare()) {
				return;
			}
		}

		result = xrStartEnvironmentDepthProviderMETA(provider);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("xrStartEnvironmentDepthProviderMETA failed!");
			UtilityFunctions::print(get_openxr_api()->get_error_string(result));
			return;
		}

		UtilityFunctions::print("started!");
	}
	else {
		result = xrStopEnvironmentDepthProviderMETA(provider);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("xrStopEnvironmentDepthProviderMETA failed!");
			UtilityFunctions::print(get_openxr_api()->get_error_string(result));
			return;
		}
	}
	started = p_started;
}

void OpenXRFbEnvironmentDepthExtensionWrapper::set_enable_hand_removal(bool p_enable_hand_removal) {
	if (provider != nullptr) {
		XrResult result;
		XrEnvironmentDepthHandRemovalSetInfoMETA set_info = {
			XR_TYPE_ENVIRONMENT_DEPTH_HAND_REMOVAL_SET_INFO_META, // type
			nullptr, // next
			p_enable_hand_removal, // enabled
		};
		result = xrSetEnvironmentDepthHandRemovalMETA(provider, &set_info);
		if (XR_FAILED(result)) {
			UtilityFunctions::print("xrSetEnvironmentDepthHandRemovalMETA failed!");
			UtilityFunctions::print(get_openxr_api()->get_error_string(result));
			return;
		}
	}
	enable_hand_removal = p_enable_hand_removal;
}

void set_projection(Projection &r_projection, const XrMatrix4x4f &r_matrix) {
	r_projection = Projection(
		Vector4(r_matrix.m[0], r_matrix.m[1], r_matrix.m[2], r_matrix.m[3]),
		Vector4(r_matrix.m[4], r_matrix.m[5], r_matrix.m[6], r_matrix.m[7]),
		Vector4(r_matrix.m[8], r_matrix.m[9], r_matrix.m[10], r_matrix.m[11]),
		Vector4(r_matrix.m[12], r_matrix.m[13], r_matrix.m[14], r_matrix.m[15])
	);
}

void OpenXRFbEnvironmentDepthExtensionWrapper::_on_pre_draw_viewport(const RID &p_viewport) {
	if (!started) {
		//get_openxr_api()->set_environment_depth_data(0, 0, 0, {}, {}, {}, {});
		return;
	}

	// https://github.com/meta-quest/Meta-OpenXR-SDK/blob/b986077c113eeb6b45cd21384cc2d426511485cf/Samples/XrSamples/XrPassthroughOcclusion/Src/XrPassthroughOcclusion.cpp#L151
	XrEnvironmentDepthImageAcquireInfoMETA acquire_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // space
		get_openxr_api()->get_predicted_display_time() // displayTime
	};
	XrEnvironmentDepthImageMETA depth_image = {
		XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META, // type
		nullptr, // next
		0, // swapchainIndex
		0, // nearZ
		0, // farZ
		{	// views
			{
				XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META, // type
				nullptr, // next
			},
			{
				XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META, // type
				nullptr, // next
			}
		}
	};

	XrResult result;
	result = xrAcquireEnvironmentDepthImageMETA(provider, &acquire_info, &depth_image);
	if (XR_FAILED(result)) {
		UtilityFunctions::print("xrAcquireEnvironmentDepthImageMETA failed!");
		UtilityFunctions::print(get_openxr_api()->get_error_string(result));
		return;
	}

	double near_z = depth_image.nearZ;
	double far_z = depth_image.farZ;

	Projection view_projections[2];
	Projection projection_projections[2];

	for (int i = 0; i < 2; ++i) {
		const XrPosef local_from_depth_eye = depth_image.views[i].pose;
		XrPosef depth_eye_from_local;
		XrPosef_Invert(&depth_eye_from_local, &local_from_depth_eye);
		XrMatrix4x4f view_mat;
		XrMatrix4x4f_CreateFromRigidTransform(&view_mat, &depth_eye_from_local);
		XrMatrix4x4f projection_mat;
		XrMatrix4x4f_CreateProjectionFov(&projection_mat, GRAPHICS_OPENGL_ES, depth_image.views[i].fov, near_z, std::isfinite(far_z) ? far_z : 0);
		set_projection(view_projections[i], view_mat);
		set_projection(projection_projections[i], projection_mat);
	}
	

	RenderingServer *rendering_server = RenderingServer::get_singleton();
	rendering_server->global_shader_parameter_set(texture_shader_parameter_name, swapchain_image_rids[depth_image.swapchainIndex]);
	rendering_server->global_shader_parameter_set(view_projection_left_shader_parameter_name, view_projections[0]);
	rendering_server->global_shader_parameter_set(view_projection_right_shader_parameter_name, view_projections[1]);
	rendering_server->global_shader_parameter_set(projection_projection_left_shader_parameter_name, projection_projections[0]);
	rendering_server->global_shader_parameter_set(projection_projection_right_shader_parameter_name, projection_projections[1]);
}




/////////////// xr_linear.h

static void XrQuaternionf_Invert(XrQuaternionf* result, const XrQuaternionf* q) {
    result->x = -q->x;
    result->y = -q->y;
    result->z = -q->z;
    result->w = q->w;
}

static void XrVector3f_Scale(XrVector3f* result, const XrVector3f* a, const float scaleFactor) {
    result->x = a->x * scaleFactor;
    result->y = a->y * scaleFactor;
    result->z = a->z * scaleFactor;
}

static void XrQuaternionf_Multiply(XrQuaternionf* result, const XrQuaternionf* a, const XrQuaternionf* b) {
    result->x = (b->w * a->x) + (b->x * a->w) + (b->y * a->z) - (b->z * a->y);
    result->y = (b->w * a->y) - (b->x * a->z) + (b->y * a->w) + (b->z * a->x);
    result->z = (b->w * a->z) + (b->x * a->y) - (b->y * a->x) + (b->z * a->w);
    result->w = (b->w * a->w) - (b->x * a->x) - (b->y * a->y) - (b->z * a->z);
}

static void XrQuaternionf_RotateVector3f(XrVector3f* result, const XrQuaternionf* a, const XrVector3f* v) {
    XrQuaternionf q = {v->x, v->y, v->z, 0.0f};
    XrQuaternionf aq;
    XrQuaternionf_Multiply(&aq, &q, a);
    XrQuaternionf aInv;
    XrQuaternionf_Invert(&aInv, a);
    XrQuaternionf aqaInv;
    XrQuaternionf_Multiply(&aqaInv, &aInv, &aq);

    result->x = aqaInv.x;
    result->y = aqaInv.y;
    result->z = aqaInv.z;
}

static void XrPosef_Invert(XrPosef* result, const XrPosef* a) {
    XrQuaternionf_Invert(&result->orientation, &a->orientation);
    XrVector3f aPosNeg;
    XrVector3f_Scale(&aPosNeg, &a->position, -1.0f);
    XrQuaternionf_RotateVector3f(&result->position, &result->orientation, &aPosNeg);
}

static void XrMatrix4x4f_CreateScale(XrMatrix4x4f* result, const float x, const float y, const float z) {
    result->m[0] = x;
    result->m[1] = 0.0f;
    result->m[2] = 0.0f;
    result->m[3] = 0.0f;
    result->m[4] = 0.0f;
    result->m[5] = y;
    result->m[6] = 0.0f;
    result->m[7] = 0.0f;
    result->m[8] = 0.0f;
    result->m[9] = 0.0f;
    result->m[10] = z;
    result->m[11] = 0.0f;
    result->m[12] = 0.0f;
    result->m[13] = 0.0f;
    result->m[14] = 0.0f;
    result->m[15] = 1.0f;
}

static void XrMatrix4x4f_CreateFromQuaternion(XrMatrix4x4f* result, const XrQuaternionf* quat) {
    const float x2 = quat->x + quat->x;
    const float y2 = quat->y + quat->y;
    const float z2 = quat->z + quat->z;

    const float xx2 = quat->x * x2;
    const float yy2 = quat->y * y2;
    const float zz2 = quat->z * z2;

    const float yz2 = quat->y * z2;
    const float wx2 = quat->w * x2;
    const float xy2 = quat->x * y2;
    const float wz2 = quat->w * z2;
    const float xz2 = quat->x * z2;
    const float wy2 = quat->w * y2;

    result->m[0] = 1.0f - yy2 - zz2;
    result->m[1] = xy2 + wz2;
    result->m[2] = xz2 - wy2;
    result->m[3] = 0.0f;

    result->m[4] = xy2 - wz2;
    result->m[5] = 1.0f - xx2 - zz2;
    result->m[6] = yz2 + wx2;
    result->m[7] = 0.0f;

    result->m[8] = xz2 + wy2;
    result->m[9] = yz2 - wx2;
    result->m[10] = 1.0f - xx2 - yy2;
    result->m[11] = 0.0f;

    result->m[12] = 0.0f;
    result->m[13] = 0.0f;
    result->m[14] = 0.0f;
    result->m[15] = 1.0f;
}

static void XrMatrix4x4f_CreateTranslation(XrMatrix4x4f* result, const float x, const float y, const float z) {
    result->m[0] = 1.0f;
    result->m[1] = 0.0f;
    result->m[2] = 0.0f;
    result->m[3] = 0.0f;
    result->m[4] = 0.0f;
    result->m[5] = 1.0f;
    result->m[6] = 0.0f;
    result->m[7] = 0.0f;
    result->m[8] = 0.0f;
    result->m[9] = 0.0f;
    result->m[10] = 1.0f;
    result->m[11] = 0.0f;
    result->m[12] = x;
    result->m[13] = y;
    result->m[14] = z;
    result->m[15] = 1.0f;
}

static void XrMatrix4x4f_Multiply(XrMatrix4x4f* result, const XrMatrix4x4f* a, const XrMatrix4x4f* b) {
    result->m[0] = a->m[0] * b->m[0] + a->m[4] * b->m[1] + a->m[8] * b->m[2] + a->m[12] * b->m[3];
    result->m[1] = a->m[1] * b->m[0] + a->m[5] * b->m[1] + a->m[9] * b->m[2] + a->m[13] * b->m[3];
    result->m[2] = a->m[2] * b->m[0] + a->m[6] * b->m[1] + a->m[10] * b->m[2] + a->m[14] * b->m[3];
    result->m[3] = a->m[3] * b->m[0] + a->m[7] * b->m[1] + a->m[11] * b->m[2] + a->m[15] * b->m[3];

    result->m[4] = a->m[0] * b->m[4] + a->m[4] * b->m[5] + a->m[8] * b->m[6] + a->m[12] * b->m[7];
    result->m[5] = a->m[1] * b->m[4] + a->m[5] * b->m[5] + a->m[9] * b->m[6] + a->m[13] * b->m[7];
    result->m[6] = a->m[2] * b->m[4] + a->m[6] * b->m[5] + a->m[10] * b->m[6] + a->m[14] * b->m[7];
    result->m[7] = a->m[3] * b->m[4] + a->m[7] * b->m[5] + a->m[11] * b->m[6] + a->m[15] * b->m[7];

    result->m[8] = a->m[0] * b->m[8] + a->m[4] * b->m[9] + a->m[8] * b->m[10] + a->m[12] * b->m[11];
    result->m[9] = a->m[1] * b->m[8] + a->m[5] * b->m[9] + a->m[9] * b->m[10] + a->m[13] * b->m[11];
    result->m[10] = a->m[2] * b->m[8] + a->m[6] * b->m[9] + a->m[10] * b->m[10] + a->m[14] * b->m[11];
    result->m[11] = a->m[3] * b->m[8] + a->m[7] * b->m[9] + a->m[11] * b->m[10] + a->m[15] * b->m[11];

    result->m[12] = a->m[0] * b->m[12] + a->m[4] * b->m[13] + a->m[8] * b->m[14] + a->m[12] * b->m[15];
    result->m[13] = a->m[1] * b->m[12] + a->m[5] * b->m[13] + a->m[9] * b->m[14] + a->m[13] * b->m[15];
    result->m[14] = a->m[2] * b->m[12] + a->m[6] * b->m[13] + a->m[10] * b->m[14] + a->m[14] * b->m[15];
    result->m[15] = a->m[3] * b->m[12] + a->m[7] * b->m[13] + a->m[11] * b->m[14] + a->m[15] * b->m[15];
}

static void XrMatrix4x4f_CreateTranslationRotationScale(XrMatrix4x4f* result, const XrVector3f* translation, const XrQuaternionf* rotation, const XrVector3f* scale) {
    XrMatrix4x4f scaleMatrix;
    XrMatrix4x4f_CreateScale(&scaleMatrix, scale->x, scale->y, scale->z);

    XrMatrix4x4f rotationMatrix;
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, rotation);

    XrMatrix4x4f translationMatrix;
    XrMatrix4x4f_CreateTranslation(&translationMatrix, translation->x, translation->y, translation->z);

    XrMatrix4x4f combinedMatrix;
    XrMatrix4x4f_Multiply(&combinedMatrix, &rotationMatrix, &scaleMatrix);
    XrMatrix4x4f_Multiply(result, &translationMatrix, &combinedMatrix);
}

static void XrMatrix4x4f_CreateProjection(XrMatrix4x4f* result, GraphicsAPI graphicsApi, const float tanAngleLeft,
											const float tanAngleRight, const float tanAngleUp, float const tanAngleDown,
											const float nearZ, const float farZ) {
    const float tanAngleWidth = tanAngleRight - tanAngleLeft;

    // Set to tanAngleDown - tanAngleUp for a clip space with positive Y down (Vulkan).
    // Set to tanAngleUp - tanAngleDown for a clip space with positive Y up (OpenGL / D3D / Metal).
    const float tanAngleHeight = graphicsApi == GRAPHICS_VULKAN ? (tanAngleDown - tanAngleUp) : (tanAngleUp - tanAngleDown);

    // Set to nearZ for a [-1,1] Z clip space (OpenGL / OpenGL ES).
    // Set to zero for a [0,1] Z clip space (Vulkan / D3D / Metal).
    const float offsetZ = (graphicsApi == GRAPHICS_OPENGL || graphicsApi == GRAPHICS_OPENGL_ES) ? nearZ : 0;

    if (farZ <= nearZ) {
        // place the far plane at infinity
        result->m[0] = 2.0f / tanAngleWidth;
        result->m[4] = 0.0f;
        result->m[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result->m[12] = 0.0f;

        result->m[1] = 0.0f;
        result->m[5] = 2.0f / tanAngleHeight;
        result->m[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result->m[13] = 0.0f;

        result->m[2] = 0.0f;
        result->m[6] = 0.0f;
        result->m[10] = -1.0f;
        result->m[14] = -(nearZ + offsetZ);

        result->m[3] = 0.0f;
        result->m[7] = 0.0f;
        result->m[11] = -1.0f;
        result->m[15] = 0.0f;
    } else {
        // normal projection
        result->m[0] = 2.0f / tanAngleWidth;
        result->m[4] = 0.0f;
        result->m[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result->m[12] = 0.0f;

        result->m[1] = 0.0f;
        result->m[5] = 2.0f / tanAngleHeight;
        result->m[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result->m[13] = 0.0f;

        result->m[2] = 0.0f;
        result->m[6] = 0.0f;
        result->m[10] = -(farZ + offsetZ) / (farZ - nearZ);
        result->m[14] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

        result->m[3] = 0.0f;
        result->m[7] = 0.0f;
        result->m[11] = -1.0f;
        result->m[15] = 0.0f;
    }
}

static void XrMatrix4x4f_CreateProjectionFov(XrMatrix4x4f* result, GraphicsAPI graphicsApi, const XrFovf fov, const float nearZ, const float farZ) {
    const float tanLeft = tanf(fov.angleLeft);
    const float tanRight = tanf(fov.angleRight);

    const float tanDown = tanf(fov.angleDown);
    const float tanUp = tanf(fov.angleUp);

    XrMatrix4x4f_CreateProjection(result, graphicsApi, tanLeft, tanRight, tanUp, tanDown, nearZ, farZ);
}

static void XrMatrix4x4f_CreateFromRigidTransform(XrMatrix4x4f* result, const XrPosef* s) {
    const XrVector3f identityScale = {1.0f, 1.0f, 1.0f};
    XrMatrix4x4f_CreateTranslationRotationScale(result, &s->position, &s->orientation, &identityScale);
}
