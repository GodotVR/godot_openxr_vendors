/**************************************************************************/
/*  xr_project_setup_dialog.h                                             */
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

#include <godot_cpp/classes/accept_dialog.hpp>
#include <godot_cpp/templates/local_vector.hpp>

namespace godot {
class HBoxContainer;
class OptionButton;
class VBoxContainer;
class Label;
} //namespace godot

namespace godot_openxr_vendors {
class Recommendation;
}

using namespace godot;
using namespace godot_openxr_vendors;

class XrProjectSetupDialog : public AcceptDialog {
	GDCLASS(XrProjectSetupDialog, AcceptDialog);

	LocalVector<Recommendation *> recommendations;

	OptionButton *project_type_selector = nullptr;
	OptionButton *vendor_type_selector = nullptr;
	HBoxContainer *restart_editor_hbox = nullptr;
	VBoxContainer *scroll_vbox = nullptr;
	Label *rec_list_empty_label = nullptr;

	void add_window_entry(Recommendation *p_recommendation);
	void filter_recommendations();
	void _on_recommendation_button_pressed(uint64_t p_recommendation);
	void _on_filter_selected(int p_item_index);

	Ref<Texture2D> error_texture;
	Ref<Texture2D> warning_texture;
	Color error_color = Color(1.0, 0.0, 0.0);
	Color warning_color = Color(1.0, 1.0, 0.0);

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	void open();

	XrProjectSetupDialog();
	~XrProjectSetupDialog();
};
