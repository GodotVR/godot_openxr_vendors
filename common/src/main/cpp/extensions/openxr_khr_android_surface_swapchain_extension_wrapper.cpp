/**************************************************************************/
/*  openxr_khr_android_surface_swapchain_extension_wrapper.cpp            */
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

#ifdef ANDROID

#include <algorithm>
#include "extensions/openxr_khr_android_surface_swapchain_extension_wrapper.h"
#include <godot_cpp/classes/open_xrapi_extension.hpp>

using namespace godot;

namespace {

// get_jni_env() is not available in gdextension, so use JNI_OnLoad hook to grab it manually
// https://github.com/godotengine/godot-proposals/issues/6734
static JavaVM *jvm = NULL;

// this will be called when Android plugin initialize
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	JNIEnv *env;
	if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
		return JNI_ERR;
	jvm = vm;
	return JNI_VERSION_1_6;
}

static JNIEnv* get_jni_env() {
	JNIEnv *env;
	if (jvm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
		return nullptr;
	return env;
}

static int deliverSurfaceToJava(jobject surface) {
	JNIEnv *env = get_jni_env();
	// Java lookups
	jclass surfaceStorageClass = env->FindClass("org/godotengine/openxr/vendors/meta/SurfaceStorage");
	if (surfaceStorageClass == nullptr) {
		WARN_PRINT("JNI: Failed to find SurfaceStorage class");
		return -1;
	}
	jmethodID storeSurfaceMid = env->GetStaticMethodID(surfaceStorageClass, "storeSurfaceForSwapchain", "(Landroid/view/Surface;)I");
	if (storeSurfaceMid == nullptr) {
		WARN_PRINT("JNI: Failed to find storeSurfaceForSwapchain method");
		return -1;
	}

	// Execute
	jint handle = env->CallStaticIntMethod(surfaceStorageClass, storeSurfaceMid, surface);
	if (env->ExceptionOccurred()) {
		WARN_PRINT("JNI: Exception ocurred!");
		return -1;
	}
	return handle;
}

static void releaseSurfaceFromJava(int handle) {
	JNIEnv *env = get_jni_env();

	// Java lookups
	jclass surfaceStorageClass = env->FindClass("org/godotengine/openxr/vendors/meta/SurfaceStorage");
	if (surfaceStorageClass == nullptr) {
		WARN_PRINT("JNI: Failed to find SurfaceStorage class");
		return;
	}
	jmethodID releaseSurfaceMid = env->GetStaticMethodID(surfaceStorageClass, "releaseSurface", "(I)V");
	if (releaseSurfaceMid == nullptr) {
		WARN_PRINT("JNI: Failed to find storeSurfaceForSwapchain method");
		return;
	}

	// Execute
	env->CallStaticVoidMethod(surfaceStorageClass, releaseSurfaceMid, handle);
	if (env->ExceptionOccurred()) {
		WARN_PRINT("JNI: Exception ocurred!");
		return;
	}
}

} // anonymous namespace

OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper *OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::singleton = nullptr;

OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper *OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper());
	}
	return singleton;
}

OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper singleton already exists.");

	request_extensions[XR_KHR_ANDROID_SURFACE_SWAPCHAIN_EXTENSION_NAME] = &khr_android_surface_swapchain_ext;
	request_extensions[XR_FB_ANDROID_SURFACE_SWAPCHAIN_CREATE_EXTENSION_NAME] = &fb_android_surface_swapchain_create_ext;
	request_extensions[XR_FB_COMPOSITION_LAYER_IMAGE_LAYOUT_EXTENSION_NAME] = &fb_composition_layer_image_layout_ext;
	singleton = this;
}

OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::~OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper() {
	cleanup();
}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_bind_methods() {}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::cleanup() {
	khr_android_surface_swapchain_ext = false;
	fb_android_surface_swapchain_create_ext = false;
	fb_composition_layer_image_layout_ext = false;
}

Dictionary OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (khr_android_surface_swapchain_ext && fb_android_surface_swapchain_create_ext) {
		bool result = initialize_khr_android_surface_swapchain_extension((XrInstance)instance);
		if (!result) {
			WARN_PRINT("Failed to initialize khr_android_surface_swapchain_ext extension");
			khr_android_surface_swapchain_ext = false;
			fb_android_surface_swapchain_create_ext = false;
			fb_composition_layer_image_layout_ext = false;
		}
	}
}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_on_session_created(uint64_t p_session) {
	if (khr_android_surface_swapchain_ext && fb_android_surface_swapchain_create_ext && fb_composition_layer_image_layout_ext) {
		get_openxr_api()->register_composition_layer_provider(this);
	}
};

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_on_session_destroyed() {
	if (khr_android_surface_swapchain_ext && fb_android_surface_swapchain_create_ext && fb_composition_layer_image_layout_ext) {
		get_openxr_api()->unregister_composition_layer_provider(this);
	}
};

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

int OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_get_composition_layer_count() {
	return layers.size();
};

uint64_t OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_get_composition_layer(int p_index) {
	XrCompositionLayerQuad* layer = &layers[p_index]->layer;

	composition_layout_ext.type = XR_TYPE_COMPOSITION_LAYER_IMAGE_LAYOUT_FB;
	composition_layout_ext.next = nullptr;
	composition_layout_ext.flags = XR_COMPOSITION_LAYER_IMAGE_LAYOUT_VERTICAL_FLIP_BIT_FB;

	layer->next = &composition_layout_ext;
	layer->space = (XrSpace) get_openxr_api()->get_play_space();
	layer->subImage.swapchain = layers[p_index]->swapchain;
	layer->subImage.imageArrayIndex = 0;

	return reinterpret_cast<uint64_t>(layer);
};

int OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::_get_composition_layer_order(int p_index) {
	auto layer = layers[p_index];
	return layer->order;
};

bool OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::initialize_khr_android_surface_swapchain_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSwapchainAndroidSurfaceKHR);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroySwapchain);

	return true;
}

int OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::allocate_swapchain(
		std::shared_ptr<QuadSurfaceLayer> layer,
		uint32_t widthPx,
		uint32_t heightPx) {
	XrAndroidSurfaceSwapchainCreateInfoFB swapchainExt;
	swapchainExt.type = XR_TYPE_ANDROID_SURFACE_SWAPCHAIN_CREATE_INFO_FB;
	swapchainExt.createFlags = 0;
	swapchainExt.createFlags |= XR_ANDROID_SURFACE_SWAPCHAIN_SYNCHRONOUS_BIT_FB;

	XrSwapchainCreateInfo swapchain_create_info = {
		XR_TYPE_SWAPCHAIN_CREATE_INFO, // type
		&swapchainExt, // next
		0, // createFlags
		XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_MUTABLE_FORMAT_BIT, // usageFlags
		0, // format (required by xrCreateSwapchainAndroidSurfaceKHR)
		0, // sampleCount (required by xrCreateSwapchainAndroidSurfaceKHR)
		widthPx, // width
		heightPx, // height
		0, // faceCount (required by xrCreateSwapchainAndroidSurfaceKHR)
		0, // arraySize (required by xrCreateSwapchainAndroidSurfaceKHR)
		0 // mipCount (required by xrCreateSwapchainAndroidSurfaceKHR)
	};

	jobject surface_obj;
	XrSwapchain new_swapchain;
	XrResult result = xrCreateSwapchainAndroidSurfaceKHR(SESSION, &swapchain_create_info, &new_swapchain, &surface_obj);
	if (XR_FAILED(result)) {
		WARN_PRINT("OpenXR: Failed to get swapchain [" + get_openxr_api()->get_error_string(result) + "]");
		return -1;
	}

	int handle = deliverSurfaceToJava(surface_obj);
	if (handle == -1) {
		WARN_PRINT("OpenXR: Failed to store surface");
		return -1;
	}

	layer->handle = handle;
	layer->swapchain = new_swapchain;

	return handle;
}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::free_swapchain(std::shared_ptr<QuadSurfaceLayer> layer) {
	if (layer->handle == -1) {
		WARN_PRINT("Tried releasing an invalid layer");
		return;
	}

	releaseSurfaceFromJava(layer->handle);
	xrDestroySwapchain(layer->swapchain);
	layer->handle = -1;
}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::start_drawing_layer(std::shared_ptr<QuadSurfaceLayer> layer) {
	// WARN_PRINT("Start drawing layer. " +  String("Count:{0}").format(Array::make(layers.size())));
	layers.push_back(layer);
}

void OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::stop_drawing_layer(std::shared_ptr<QuadSurfaceLayer> layer) {
	auto remove = std::remove_if(
		layers.begin(), layers.end(),
		[layer](std::shared_ptr<QuadSurfaceLayer> layer_b) {return layer_b->handle == layer->handle;});
	if (remove == layers.end()) {
		WARN_PRINT("Unable to remove layer");
	} else {
		layers.erase(remove);
	}
}

#endif // ANDROID
