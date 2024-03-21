/**************************************************************************/
/*  android_surface_layer.h                                               */
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

#include <memory>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "extensions/openxr_khr_android_surface_swapchain_extension_wrapper.h"

namespace godot {

class AndroidSurfaceLayer : public Node3D {
	GDCLASS(AndroidSurfaceLayer, Node3D)

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	AndroidSurfaceLayer();
	~AndroidSurfaceLayer();

	int get_android_surface_handle();
	void set_layer_resolution(int widthPx, int heightPx);
	void set_size_2d_meters(float width, float height);
	void set_sort_order(int sort_order);
	bool is_supported();

	void update_visibility();
	void update_transform();

private:
	std::shared_ptr<QuadSurfaceLayer> layer;
};

} //namespace godot
