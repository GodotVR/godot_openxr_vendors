/**************************************************************************/
/*  GodotPanelApp.kt                                                      */
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

package org.godotengine.openxr.vendors

import android.os.Bundle
import android.util.Log
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import org.godotengine.godot.GodotActivity
import org.godotengine.godot.BuildConfig
import org.godotengine.openxr.vendors.utils.*

/**
 * Activity for the Panel app mode in a hybrid app.
 */
class GodotPanelApp : GodotActivity() {
	companion object {
		private val TAG = GodotPanelApp::class.java.simpleName

		// .NET libraries.
		init {
			if (BuildConfig.FLAVOR == "mono") {
				try {
					Log.v(TAG, "Loading System.Security.Cryptography.Native.Android library")
					System.loadLibrary("System.Security.Cryptography.Native.Android")
				} catch (e: UnsatisfiedLinkError) {
					Log.e(TAG, "Unable to load System.Security.Cryptography.Native.Android library")
				}
			}
		}
	}

	override fun onCreate(savedInstanceState: Bundle?) {
		installSplashScreen()
		super.onCreate(savedInstanceState)
	}

	override fun getCommandLine(): MutableList<String> {
		val oldCmdline = super.getCommandLine()
		return updateCommandLineForHybridLaunch(
			HybridMode.PANEL, oldCmdline
		)
	}

	override fun supportsFeature(featureTag: String): Boolean {
		return HYBRID_APP_PANEL_FEATURE == featureTag
	}
}
