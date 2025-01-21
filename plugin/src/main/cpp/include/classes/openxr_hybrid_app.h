/**************************************************************************/
/*  openxr_hybrid_app.h                                                   */
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

#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

// Singleton providing an API for hybrid apps.
class OpenXRHybridApp : public Object {
	GDCLASS(OpenXRHybridApp, Object);

	static OpenXRHybridApp *singleton;

protected:
	static void _bind_methods();

public:
	enum HybridMode {
		HYBRID_MODE_NONE = -1,
		HYBRID_MODE_IMMERSIVE = 0,
		HYBRID_MODE_PANEL = 1,
	};

	static OpenXRHybridApp *get_singleton();

	bool is_hybrid_app() const;
	HybridMode get_mode() const;

	bool switch_mode(HybridMode p_mode, const String &p_data);
	String get_launch_data() const;

	OpenXRHybridApp();
	virtual ~OpenXRHybridApp();
};

VARIANT_ENUM_CAST(OpenXRHybridApp::HybridMode);
