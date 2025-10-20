/**************************************************************************/
/*  DeviceUtils.kt                                                        */
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

/**
 * Contains utility methods for detecting specific devices.
 */
@file:JvmName("DeviceUtils")

package org.godotengine.openxr.vendors.utils

import android.content.Context
import android.os.Build

// Set of supported OS
/**
 * Label used to identity the Meta Horizon OS in the Godot engine.
 */
const val META_HORIZON_OS = "horizonos"

/**
 * Label used to identify the Pico OS in the Godot engine.
 */
const val PICO_OS = "picoos"

/**
 * Returns true if running on Meta Horizon OS.
 */
fun isHorizonOSDevice(context: Context): Boolean {
	return context.packageManager.hasSystemFeature("oculus.hardware.standalone_vr")
}

/**
 * Returns true if running on PICO OS.
 */
fun isPicoOSDevice(): Boolean {
	return ("Pico".equals(Build.BRAND, true))
}

/**
 * Returns true if running on a native Android XR device.
 */
fun isNativeXRDevice(context: Context): Boolean {
	return isHorizonOSDevice(context) || isPicoOSDevice()
}
