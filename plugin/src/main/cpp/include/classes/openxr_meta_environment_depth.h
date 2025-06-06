/**************************************************************************/
/*  openxr_meta_environment_depth.h                                       */
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

#include <godot_cpp/classes/visual_instance3d.hpp>

namespace godot {
class OpenXRMetaEnvironmentDepth : public VisualInstance3D {
	GDCLASS(OpenXRMetaEnvironmentDepth, VisualInstance3D);

	void _update_visibility();

	void _on_openxr_session_begun();
	void _on_openxr_session_stopping();
	void _on_environment_depth_started();
	void _on_environment_depth_stopped();

protected:
	void _notification(int p_what);

	static void _bind_methods();

public:
	PackedStringArray _get_configuration_warnings() const override;

	void set_render_priority(int p_render_priority);
	int get_render_priority() const;

	void set_reprojection_offset_scale(float p_offset_exponent);
	float get_reprojection_offset_scale() const;

	void set_reprojection_offset_exponent(float p_offset_exponent);
	float get_reprojection_offset_exponent() const;

	OpenXRMetaEnvironmentDepth();
	~OpenXRMetaEnvironmentDepth();
};
}; // namespace godot
