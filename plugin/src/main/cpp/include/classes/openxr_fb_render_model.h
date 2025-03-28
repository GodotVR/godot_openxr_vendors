/**************************************************************************/
/*  openxr_fb_render_model.h                                              */
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

#ifndef OPENXR_FB_RENDER_MODEL_H
#define OPENXR_FB_RENDER_MODEL_H

#include <godot_cpp/classes/node3d.hpp>

namespace godot {
class OpenXRFbRenderModel : public Node3D {
	GDCLASS(OpenXRFbRenderModel, Node3D)

public:
	enum Model {
		MODEL_CONTROLLER_LEFT,
		MODEL_CONTROLLER_RIGHT,
	};

private:
	Model render_model_type = MODEL_CONTROLLER_LEFT;
	Node3D *render_model_node = nullptr;

	void load_render_model();

protected:
	void _notification(int p_what);

	static void _bind_methods();

public:
	void set_render_model_type(Model p_model);
	Model get_render_model_type();

	bool has_render_model_node();
	Node3D *get_render_model_node();

	virtual PackedStringArray _get_configuration_warnings() const override;
};
} //namespace godot

VARIANT_ENUM_CAST(OpenXRFbRenderModel::Model);

#endif
