/**************************************************************************/
/*  openxr_meta_environment_depth.cpp                                     */
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

#include "classes/openxr_meta_environment_depth.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_camera3d.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "extensions/openxr_meta_environment_depth_extension_wrapper.h"

using namespace godot;

void OpenXRMetaEnvironmentDepth::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_render_priority", "render_priority"), &OpenXRMetaEnvironmentDepth::set_render_priority);
	ClassDB::bind_method(D_METHOD("get_render_priority"), &OpenXRMetaEnvironmentDepth::get_render_priority);

	ClassDB::bind_method(D_METHOD("set_reprojection_offset_scale", "offset_scale"), &OpenXRMetaEnvironmentDepth::set_reprojection_offset_scale);
	ClassDB::bind_method(D_METHOD("get_reprojection_offset_scale"), &OpenXRMetaEnvironmentDepth::get_reprojection_offset_scale);

	ClassDB::bind_method(D_METHOD("set_reprojection_offset_exponent", "offset_exponent"), &OpenXRMetaEnvironmentDepth::set_reprojection_offset_exponent);
	ClassDB::bind_method(D_METHOD("get_reprojection_offset_exponent"), &OpenXRMetaEnvironmentDepth::get_reprojection_offset_exponent);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "render_priority"), "set_render_priority", "get_render_priority");

	ADD_GROUP("Reprojection Offset", "reprojection_offset_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "reprojection_offset_scale"), "set_reprojection_offset_scale", "get_reprojection_offset_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "reprojection_offset_exponent"), "set_reprojection_offset_exponent", "get_reprojection_offset_exponent");
}

void OpenXRMetaEnvironmentDepth::set_render_priority(int p_render_priority) {
	OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton()->set_reprojection_render_priority(p_render_priority);
}

int OpenXRMetaEnvironmentDepth::get_render_priority() const {
	return OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton()->get_reprojection_render_priority();
}

void OpenXRMetaEnvironmentDepth::set_reprojection_offset_scale(float p_offset_scale) {
	OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton()->set_reprojection_offset_scale(p_offset_scale);
}

float OpenXRMetaEnvironmentDepth::get_reprojection_offset_scale() const {
	return OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton()->get_reprojection_offset_scale();
}

void OpenXRMetaEnvironmentDepth::set_reprojection_offset_exponent(float p_offset_exponent) {
	OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton()->set_reprojection_offset_exponent(p_offset_exponent);
}

float OpenXRMetaEnvironmentDepth::get_reprojection_offset_exponent() const {
	return OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton()->get_reprojection_offset_exponent();
}

void OpenXRMetaEnvironmentDepth::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_EXIT_TREE:
		case NOTIFICATION_VISIBILITY_CHANGED: {
			_update_visibility();
		} break;
	}
}

void OpenXRMetaEnvironmentDepth::_update_visibility() {
	bool is_visible = false;
	OpenXRMetaEnvironmentDepthExtensionWrapper *env_depth_ext = OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton();
	if (env_depth_ext) {
		Ref<OpenXRAPIExtension> openxr_api = env_depth_ext->get_openxr_api();
		is_visible = is_visible_in_tree() && openxr_api->is_running() && env_depth_ext->is_environment_depth_started();
	}

	if (is_visible) {
		set_base(env_depth_ext->get_reprojection_mesh());
	} else {
		set_base(RID());
	}
}

void OpenXRMetaEnvironmentDepth::_on_openxr_session_begun() {
	_update_visibility();
}

void OpenXRMetaEnvironmentDepth::_on_openxr_session_stopping() {
	_update_visibility();
}

void OpenXRMetaEnvironmentDepth::_on_environment_depth_started() {
	_update_visibility();
}

void OpenXRMetaEnvironmentDepth::_on_environment_depth_stopped() {
	_update_visibility();
}

PackedStringArray OpenXRMetaEnvironmentDepth::_get_configuration_warnings() const {
	PackedStringArray warnings = Node::_get_configuration_warnings();

	if (is_visible() && is_inside_tree()) {
		XRCamera3D *camera = Object::cast_to<XRCamera3D>(get_parent());
		if (camera == nullptr) {
			warnings.push_back("OpenXRMetaEnvironmentDepth must be a child of an XRCamera3D node.");
		}
	}

	return warnings;
}

OpenXRMetaEnvironmentDepth::OpenXRMetaEnvironmentDepth() {
	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server) {
		Ref<XRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
		if (openxr_interface.is_valid()) {
			openxr_interface->connect("session_begun", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_begun));
			openxr_interface->connect("session_stopping", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_stopping));
		}
	}

	OpenXRMetaEnvironmentDepthExtensionWrapper *env_depth_ext = OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton();
	if (env_depth_ext) {
		env_depth_ext->connect("openxr_meta_environment_depth_started", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_environment_depth_started));
		env_depth_ext->connect("openxr_meta_environment_depth_stopped", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_environment_depth_stopped));
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		rs->instance_geometry_set_cast_shadows_setting(get_instance(), RenderingServer::SHADOW_CASTING_SETTING_OFF);
	}
}

OpenXRMetaEnvironmentDepth::~OpenXRMetaEnvironmentDepth() {
	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server) {
		Ref<XRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
		if (openxr_interface.is_valid()) {
			openxr_interface->disconnect("session_begun", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_begun));
			openxr_interface->disconnect("session_stopping", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_stopping));
		}
	}

	OpenXRMetaEnvironmentDepthExtensionWrapper *env_depth_ext = OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton();
	if (env_depth_ext) {
		env_depth_ext->disconnect("openxr_meta_environment_depth_started", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_environment_depth_started));
		env_depth_ext->disconnect("openxr_meta_environment_depth_stopped", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_environment_depth_stopped));
	}
}
