/**************************************************************************/
/*  openxr_meta_passthrough_color_lut.cpp                                 */
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

#include "classes/openxr_meta_passthrough_color_lut.h"

#include "extensions/openxr_fb_passthrough_extension_wrapper.h"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void OpenXRMetaPassthroughColorLut::_bind_methods() {
	ClassDB::bind_static_method("OpenXRMetaPassthroughColorLut", D_METHOD("create_from_image"), &OpenXRMetaPassthroughColorLut::create_from_image);

	BIND_ENUM_CONSTANT(COLOR_LUT_CHANNELS_RGB);
	BIND_ENUM_CONSTANT(COLOR_LUT_CHANNELS_RGBA);
}

OpenXRMetaPassthroughColorLut::~OpenXRMetaPassthroughColorLut() {
	OpenXRFbPassthroughExtensionWrapper::get_singleton()->destroy_color_lut(this);
}

Ref<OpenXRMetaPassthroughColorLut> OpenXRMetaPassthroughColorLut::create_from_image(Ref<Image> p_image, ColorLutChannels p_channels) {
	Ref<OpenXRMetaPassthroughColorLut> passthrough_color_lut;
	passthrough_color_lut.instantiate();
	passthrough_color_lut->populate_buffer(p_image, p_channels);
	return passthrough_color_lut;
}

void OpenXRMetaPassthroughColorLut::populate_buffer(const Ref<Image> &p_image, ColorLutChannels p_channels) {
	int height = p_image->get_height();
	int width = p_image->get_width();

	if (height != width) { // Rectangular image
		if ((height & (height - 1)) != 0) {
			UtilityFunctions::print("Color LUT cell resolution must be a power of 2, current resolution: ", height);
			return;
		}

		if (width != (height * height)) {
			UtilityFunctions::print("Color LUT image is incorrect size");
			return;
		}

		image_cell_resolution = height;
	} else { // Square image
		switch (height) {
			case 8: {
				image_cell_resolution = 4;
			} break;
			case 64: {
				image_cell_resolution = 16;
			} break;
			case 512: {
				image_cell_resolution = 64;
			} break;
			default: {
				UtilityFunctions::print("Square color LUT image must be of total resolution 8x8, 64x64, or 512x512");
				return;
			} break;
		}
	}

	channels = p_channels;
	switch (p_channels) {
		case COLOR_LUT_CHANNELS_RGB: {
			if (p_image->get_format() != Image::FORMAT_RGB8) {
				p_image->convert(Image::FORMAT_RGB8);
			}
		} break;
		case COLOR_LUT_CHANNELS_RGBA: {
			if (p_image->get_format() != Image::FORMAT_RGBA8) {
				p_image->convert(Image::FORMAT_RGBA8);
			}
		} break;
	}

	PackedByteArray image_data = p_image->get_data();
	const uint8_t *image_ptr = image_data.ptr();

	int res_sq = pow(image_cell_resolution, 2);
	int res_sqrt = sqrt(image_cell_resolution);

	int buffer_size = pow(image_cell_resolution, 3) * p_channels;
	buffer.resize(buffer_size);
	uint8_t *color_lut_buffer_ptr = buffer.ptrw();

	for (int y = 0; y < height; y++) {
		int y_val = res_sqrt * res_sq * (y / image_cell_resolution) + image_cell_resolution * (y % image_cell_resolution);
		for (int x = 0; x < width; x++) {
			const uint8_t *pixel = image_ptr + (y * width * p_channels) + (x * p_channels);
			int x_val = res_sq * (x / image_cell_resolution) + (x % image_cell_resolution);
			int start_index = (x_val + y_val) * p_channels;

			for (int i = 0; i < p_channels; i++) {
				color_lut_buffer_ptr[start_index + i] = pixel[i];
			}
		}
	}
}
