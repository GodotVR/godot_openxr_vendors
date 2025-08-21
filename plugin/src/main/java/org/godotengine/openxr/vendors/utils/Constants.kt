/**************************************************************************/
/*  Constants.kt                                                          */
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

@file:JvmName("Constants")

package org.godotengine.openxr.vendors.utils

// Set of supported vendors
internal const val ANDROID_XR_VENDOR_NAME = "androidxr"
internal const val META_VENDOR_NAME = "meta"
internal const val PICO_VENDOR_NAME = "pico"
internal const val LYNX_VENDOR_NAME = "lynx"
internal const val KHRONOS_VENDOR_NAME = "khronos"
internal const val MAGICLEAP_VENDOR_NAME = "magicleap"

// Feature tags
internal const val EYE_GAZE_INTERACTION_FEATURE_TAG = "PERMISSION_XR_EXT_eye_gaze_interaction"

// Android permissions
/**
 * Representing the user's eye pose, status, and orientation, such as for use with avatars.
 * Use this permission when low-precision eye tracking data is needed.
 */
internal const val ANDROID_XR_EYE_TRACKING_COARSE_PERMISSION = "android.permission.EYE_TRACKING_COARSE"

/**
 * Eye gaze for selection, input, and interactions.
 */
internal const val ANDROID_XR_EYE_TRACKING_FINE_PERMISSION = "android.permission.EYE_TRACKING_FINE"

/**
 * Tracking and rendering facial expressions.
 */
internal const val ANDROID_XR_FACE_TRACKING_PERMISSION = "android.permission.FACE_TRACKING"

/**
 * Tracking hand joint poses and angular and linear velocities; Using a mesh representation of
 * the user's hands.
 */
internal const val ANDROID_XR_HAND_TRACKING_PERMISSION = "android.permission.HAND_TRACKING"

/**
 * - Light estimation
 * - projecting passthrough onto mesh surfaces
 * - performing raycasts against trackables in the environment
 * - plane tracking
 * - object tracking
 * - persistent anchors
 */
internal const val ANDROID_XR_SCENE_UNDERSTANDING_COARSE_PERMISSION = "android.permission.SCENE_UNDERSTANDING_COARSE"

/**
 * Depth texture.
 */
internal const val ANDROID_XR_SCENE_UNDERSTANDING_FINE_PERMISSION = "android.permission.SCENE_UNDERSTANDING_FINE"

internal val ANDROID_XR_PERMISSIONS_LIST = listOf(
    ANDROID_XR_EYE_TRACKING_COARSE_PERMISSION,
    ANDROID_XR_EYE_TRACKING_FINE_PERMISSION,
    ANDROID_XR_FACE_TRACKING_PERMISSION,
    ANDROID_XR_HAND_TRACKING_PERMISSION,
    ANDROID_XR_SCENE_UNDERSTANDING_COARSE_PERMISSION,
    ANDROID_XR_SCENE_UNDERSTANDING_FINE_PERMISSION,
)

// Meta permissions
internal const val META_BODY_TRACKING_PERMISSION = "com.oculus.permission.BODY_TRACKING"
internal const val META_EYE_TRACKING_PERMISSION = "com.oculus.permission.EYE_TRACKING"
internal const val META_FACE_TRACKING_PERMISSION = "com.oculus.permission.FACE_TRACKING"
internal const val META_SCENE_PERMISSION = "com.oculus.permission.USE_SCENE"

internal val META_PERMISSIONS_LIST = listOf(
    META_BODY_TRACKING_PERMISSION,
    META_EYE_TRACKING_PERMISSION,
    META_FACE_TRACKING_PERMISSION,
    META_SCENE_PERMISSION,
)

// Pico permissions
internal const val PICO_EYE_TRACKING_PERMISSION = "com.picovr.permission.EYE_TRACKING"
internal const val PICO_FACE_TRACKING_PERMISSION = "com.picovr.permission.FACE_TRACKING"
internal const val PICO_SPATIAL_DATA_PERMISSION = "com.picovr.permission.SPATIAL_DATA"

internal val PICO_PERMISSIONS_LIST = listOf(
    PICO_EYE_TRACKING_PERMISSION,
    PICO_FACE_TRACKING_PERMISSION,
    PICO_SPATIAL_DATA_PERMISSION,
)

internal fun getVendorPermissions(vendorName: String): List<String> {
    return when (vendorName) {
        ANDROID_XR_VENDOR_NAME -> ANDROID_XR_PERMISSIONS_LIST
        META_VENDOR_NAME -> META_PERMISSIONS_LIST
        PICO_VENDOR_NAME -> PICO_PERMISSIONS_LIST
        else -> emptyList()
    }
}
