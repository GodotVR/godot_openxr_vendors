/**************************************************************************/
/*  android_surface_layer.cpp                                             */
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

#include "classes/android_surface_layer.h"

using namespace godot;

AndroidSurfaceLayer::AndroidSurfaceLayer() {
	set_notify_transform(true);
	layer = std::make_shared<QuadSurfaceLayer>();
	layer->handle = -1; // Start off invalid
	layer->layer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
	layer->layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	layer->layer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
}

AndroidSurfaceLayer::~AndroidSurfaceLayer() {
#ifdef ANDROID
	OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton()->free_swapchain(layer);
	OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton()->stop_drawing_layer(layer);
#endif
}

void AndroidSurfaceLayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_supported"), &AndroidSurfaceLayer::is_supported);
	ClassDB::bind_method(D_METHOD("get_android_surface_handle"), &AndroidSurfaceLayer::get_android_surface_handle);
	ClassDB::bind_method(D_METHOD("set_layer_resolution"), &AndroidSurfaceLayer::set_layer_resolution);
	ClassDB::bind_method(D_METHOD("set_size_2d_meters"), &AndroidSurfaceLayer::set_size_2d_meters);
	ClassDB::bind_method(D_METHOD("set_sort_order"), &AndroidSurfaceLayer::set_sort_order);
}

bool AndroidSurfaceLayer::is_supported() {
#ifdef ANDROID
	return OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton()->is_android_surface_swapchain_supported();
#else
	return false;
#endif
}

int AndroidSurfaceLayer::get_android_surface_handle() {
	return layer->handle;
}

void AndroidSurfaceLayer::set_layer_resolution(int widthPx, int heightPx) {
	if (layer->handle != -1) {
		WARN_PRINT("Swapchain is already allocated! Bailing");
		return;
	}
#ifdef ANDROID
	layer->layer.subImage.imageRect = {{0, 0}, {widthPx, heightPx}};
	OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton()->allocate_swapchain(
			layer,
			static_cast<uint32_t>(widthPx),
			static_cast<uint32_t>(heightPx));
#endif
}

void AndroidSurfaceLayer::set_size_2d_meters(float p_width_m, float p_height_m) {
	layer->layer.size.width = p_width_m;
	layer->layer.size.height = p_height_m;
}

void AndroidSurfaceLayer::set_sort_order(int p_sort_order) {
	layer->order = p_sort_order;
}

void AndroidSurfaceLayer::update_transform() {
	auto transform = get_global_transform();
	auto rot = transform.get_basis().get_rotation_quaternion();
	auto pos = transform.get_origin();

	layer->layer.pose.orientation.x = rot.x;
	layer->layer.pose.orientation.y = rot.y;
	layer->layer.pose.orientation.z = rot.z;
	layer->layer.pose.orientation.w = rot.w;
	layer->layer.pose.position.x = pos.x;
	layer->layer.pose.position.y = pos.y;
	layer->layer.pose.position.z = pos.z;
}

void AndroidSurfaceLayer::update_visibility() {
#ifdef ANDROID
	if (is_visible_in_tree()) {
		OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton()->start_drawing_layer(layer);
	} else {
		OpenXRKhrAndroidSurfaceSwapchainExtensionWrapper::get_singleton()->stop_drawing_layer(layer);
	}
#endif
}

void AndroidSurfaceLayer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSFORM_CHANGED: {
			update_transform();
			break;
		}
		case NOTIFICATION_READY:
		case NOTIFICATION_VISIBILITY_CHANGED: {
			update_visibility();
			break;
		}
	}
}
