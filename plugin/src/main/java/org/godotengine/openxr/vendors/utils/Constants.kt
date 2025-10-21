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
internal const val META_VENDOR_NAME = "meta"
internal const val PICO_VENDOR_NAME = "pico"
internal const val LYNX_VENDOR_NAME = "lynx"
internal const val KHRONOS_VENDOR_NAME = "khronos"
internal const val MAGICLEAP_VENDOR_NAME = "magicleap"

// Feature tags
internal const val EYE_GAZE_INTERACTION_FEATURE_TAG = "PERMISSION_XR_EXT_eye_gaze_interaction"

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

internal val PICO_PERMISSIONS_LIST = listOf(
    PICO_EYE_TRACKING_PERMISSION,
    PICO_FACE_TRACKING_PERMISSION,
)

internal fun getVendorPermissions(vendorName: String): List<String> {
    return when (vendorName) {
        META_VENDOR_NAME -> META_PERMISSIONS_LIST
        PICO_VENDOR_NAME -> PICO_PERMISSIONS_LIST
        else -> emptyList()
    }
}
