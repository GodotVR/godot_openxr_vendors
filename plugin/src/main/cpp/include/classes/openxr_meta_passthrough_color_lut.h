/**************************************************************************/
/*  openxr_meta_passthrough_color_lut.h                                   */
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

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {
class OpenXRMetaPassthroughColorLut : public RefCounted {
	GDCLASS(OpenXRMetaPassthroughColorLut, RefCounted);

public:
	enum ColorLutChannels {
		COLOR_LUT_CHANNELS_RGB = 3,
		COLOR_LUT_CHANNELS_RGBA = 4,
	};

private:
	RID color_lut_handle;
	int image_cell_resolution = 0;
	ColorLutChannels channels;
	PackedByteArray buffer;

protected:
	static void _bind_methods();

	void populate_buffer(const Ref<Image> &p_image, ColorLutChannels p_channels);

public:
	static Ref<OpenXRMetaPassthroughColorLut> create_from_image(Ref<Image> p_image, ColorLutChannels p_channels);

	RID get_handle() const { return color_lut_handle; }

	int get_image_cell_resolution() const { return image_cell_resolution; }
	PackedByteArray get_buffer() const { return buffer; }
	ColorLutChannels get_channels() const { return channels; }

	~OpenXRMetaPassthroughColorLut();
};
} // namespace godot

VARIANT_ENUM_CAST(OpenXRMetaPassthroughColorLut::ColorLutChannels);
