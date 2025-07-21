/**************************************************************************/
/*  openxr_ml_marker_detector_ean_13_settings.h                           */
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

#ifndef OPENXR_ML_MARKER_DETECTOR_EAN_13_SETTINGS_H
#define OPENXR_ML_MARKER_DETECTOR_EAN_13_SETTINGS_H

#include "classes/openxr_ml_marker_detector_settings.h"

#include <godot_cpp/core/binder_common.hpp>

namespace godot {
class OpenXRMlMarkerDetectorEan13Settings : public OpenXRMlMarkerDetectorSettings {
	GDCLASS(OpenXRMlMarkerDetectorEan13Settings, OpenXRMlMarkerDetectorSettings);

protected:
	static void _bind_methods();

public:
	OpenXRMlMarkerDetectorEan13Settings();
};
} // namespace godot
#endif
