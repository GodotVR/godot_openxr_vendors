/**************************************************************************/
/*  SurfaceStorage.java                                                   */
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

package org.godotengine.openxr.vendors.meta;

import android.util.Log;
import android.util.SparseArray;
import android.view.Surface;

/**
 * Stores Android Surface objects created to back XrSwapchains
 */
public class SurfaceStorage {
    static {
        System.loadLibrary("godotopenxrvendors");
    }

  private static final String TAG = "SurfaceStorage";
  private static final SparseArray<Surface> surfaces = new SparseArray();
  private static int latestIndex = 0;

  // Called by Java requesting the Surface
  public static Surface getSurfaceForSwapchain(int index) {
      Log.d(TAG, "getSurfaceForSwapchain(" + index + ")" + "will return " + surfaces.get(index) + " from array " + surfaces);
      return surfaces.get(index);
  }

  // Called by Native storing the Surface
  private static int storeSurfaceForSwapchain(Surface surface) {
      int index = latestIndex;
      Log.d(TAG, "storeSurfaceForSwapchain: " + surface + " at index " + index + " of array " + surfaces);
      surfaces.put(index, surface);
      latestIndex++;
      return index;
  }

  private static void releaseSurface(int index) {
      // No need to manually release, that's handled by xrDestroySwapchain
      surfaces.delete(index);
  }
}
