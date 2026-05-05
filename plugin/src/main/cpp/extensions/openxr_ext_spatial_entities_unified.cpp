/**************************************************************************/
/*  openxr_ext_spatial_entities_unified.cpp                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "extensions/openxr_ext_spatial_entities_unified.h"

#include <godot_cpp/classes/open_xr_future_result.hpp>
#include <godot_cpp/classes/open_xr_spatial_anchor_capability.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_april_tag.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_aruco.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_micro_qr_code.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_plane_tracking.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_qr_code.hpp>
#include <godot_cpp/classes/open_xr_spatial_context_persistence_config.hpp>
#include <godot_cpp/classes/open_xr_spatial_entity_extension.hpp>
#include <godot_cpp/classes/open_xr_spatial_marker_tracking_capability.hpp>
#include <godot_cpp/classes/open_xr_spatial_plane_tracking_capability.hpp>
#include <godot_cpp/classes/open_xr_spatial_query_result_data.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

namespace {
void _append_component_data(TypedArray<OpenXRSpatialComponentData> &p_components, bool p_enable, const StringName &p_class_name) {
	// The first must be OpenXRSpatialQueryResultData
	if (p_components.is_empty()) {
		Ref<OpenXRSpatialQueryResultData> query_result_data;
		query_result_data.instantiate();
		p_components.push_back(query_result_data);
	}

	// Start at '1' since index 0 is OpenXRSpatialQueryResultData
	for (int i = 1; i < p_components.size(); ++i) {
		Ref<OpenXRSpatialComponentData> component = p_components[i];
		if (component->get_class() == p_class_name) {
			// the component is already in the list...

			if (!p_enable) {
				// disabling, remove it from the list
				p_components.erase(component);
			}

			// (if p_enable is true, then keep it in the list)
			return;
		}
	}

	// couldn't find the component in the list

	if (!p_enable) {
		// the component is already not in the list, nothing to do
		return;
	}

	// add the component
	p_components.push_back(ClassDB::instantiate(p_class_name));
}

BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> _set_automatic_update(BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> &capability_flags_automatic_query, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_flags, bool p_enable) {
	if (p_enable) {
		capability_flags_automatic_query = capability_flags_automatic_query | p_flags;
	} else {
		capability_flags_automatic_query = capability_flags_automatic_query & ~p_flags;
	}

	return capability_flags_automatic_query;
}

bool _can_skip_query(const HashMap<OpenXRSpatialEntitiesUnified::CapabilityOptions, bool> &p_options, const LocalVector<OpenXRSpatialEntitiesUnified::CapabilityOptions> &p_required_options, const TypedArray<OpenXRSpatialComponentData> &p_components) {
	// Skip if not configured (this conditional may never be true, but check anyway)
	if (unlikely(p_options.is_empty() || p_components.is_empty())) {
		return true;
	}

	// If all p_options are not required...
	if (p_required_options.is_empty()) {
		// ... then query if there is at least one enabled option
		for (auto it = p_options.begin(); it != p_options.end(); ++it) {
			if (it->value) {
				return false;
			}
		}

		// all p_options are disabled, skip
		return true;
	}

	// Skip if at least one required option is disabled
	for (const OpenXRSpatialEntitiesUnified::CapabilityOptions required_option : p_required_options) {
		// Programmer error if 'p_options' doesn't have 'required_option';
		// p_options and required p_options should've been set at the same time
		ERR_FAIL_COND_V(!p_options.has(required_option), true);

		if (!p_options[required_option]) {
			return true;
		}
	}

	// all required p_options are enabled, don't skip
	return false;
}
} //namespace

////////////////////////////////////////////////////////////////////////////
// OpenXRSpatialEntitiesUnifiedConfigurationAnchor

void OpenXRSpatialEntitiesUnifiedConfigurationAnchor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "enable_persistence"), &OpenXRSpatialEntitiesUnifiedConfigurationAnchor::init);
	ClassDB::bind_method(D_METHOD("get_enabled_components"), &OpenXRSpatialEntitiesUnifiedConfigurationAnchor::_get_enabled_components);
}

uint64_t OpenXRSpatialEntitiesUnifiedConfigurationAnchor::_get_configuration() {
	return (uint64_t)&anchor_config;
}

void OpenXRSpatialEntitiesUnifiedConfigurationAnchor::init(bool p_enable_persistence) {
	anchor_enabled_components.clear();

	// copy the base configuration
	Ref<OpenXRSpatialCapabilityConfigurationAnchor> base;
	base.instantiate();
	XrSpatialCapabilityConfigurationBaseHeaderEXT *base_configuration = (XrSpatialCapabilityConfigurationBaseHeaderEXT *)base->get_configuration();
	for (int i = 0; i < base_configuration->enabledComponentCount; ++i) {
		if (base_configuration->enabledComponents[i] == XR_SPATIAL_COMPONENT_TYPE_PERSISTENCE_EXT) {
			if (p_enable_persistence) {
				anchor_enabled_components.push_back(XR_SPATIAL_COMPONENT_TYPE_PERSISTENCE_EXT);
			}

			continue;
		}

		anchor_enabled_components.push_back(base_configuration->enabledComponents[i]);
	}

	anchor_config.enabledComponentCount = anchor_enabled_components.size();
	anchor_config.enabledComponents = anchor_enabled_components.ptr();
}

PackedInt64Array OpenXRSpatialEntitiesUnifiedConfigurationAnchor::_get_enabled_components() const {
	PackedInt64Array components;

	for (const XrSpatialComponentTypeEXT &component_type : anchor_enabled_components) {
		components.push_back((int64_t)component_type);
	}

	return components;
}

////////////////////////////////////////////////////////////////////////////
// OpenXRSpatialEntitiesUnifiedConfigurationPlane

void OpenXRSpatialEntitiesUnifiedConfigurationPlane::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "supports_mesh_2d", "supports_polygons", "supports_labels"), &OpenXRSpatialEntitiesUnifiedConfigurationPlane::init);
	ClassDB::bind_method(D_METHOD("get_enabled_components"), &OpenXRSpatialEntitiesUnifiedConfigurationPlane::_get_enabled_components);
}

uint64_t OpenXRSpatialEntitiesUnifiedConfigurationPlane::_get_configuration() {
	return (uint64_t)&plane_config;
}

void OpenXRSpatialEntitiesUnifiedConfigurationPlane::init(bool p_supports_mesh_2d, bool p_supports_polygons, bool p_supports_labels) {
	plane_enabled_components.clear();

	plane_enabled_components.push_back(XR_SPATIAL_COMPONENT_TYPE_BOUNDED_2D_EXT);
	plane_enabled_components.push_back(XR_SPATIAL_COMPONENT_TYPE_PLANE_ALIGNMENT_EXT);

	if (p_supports_mesh_2d) {
		plane_enabled_components.push_back(XR_SPATIAL_COMPONENT_TYPE_MESH_2D_EXT);
	}

	if (p_supports_polygons) {
		plane_enabled_components.push_back(XR_SPATIAL_COMPONENT_TYPE_POLYGON_2D_EXT);
	}

	if (p_supports_labels) {
		plane_enabled_components.push_back(XR_SPATIAL_COMPONENT_TYPE_PLANE_SEMANTIC_LABEL_EXT);
	}

	// Set up our enabled components.
	plane_config.enabledComponentCount = plane_enabled_components.size();
	plane_config.enabledComponents = plane_enabled_components.ptr();
}

PackedInt64Array OpenXRSpatialEntitiesUnifiedConfigurationPlane::_get_enabled_components() const {
	PackedInt64Array components;

	for (const XrSpatialComponentTypeEXT &component_type : plane_enabled_components) {
		components.push_back((int64_t)component_type);
	}

	return components;
}

////////////////////////////////////////////////////////////////////////////
// OpenXRSpatialEntitiesUnified

OpenXRSpatialEntitiesUnified *OpenXRSpatialEntitiesUnified::singleton = nullptr;

OpenXRSpatialEntitiesUnified *OpenXRSpatialEntitiesUnified::get_singleton() {
	if (singleton == nullptr) {
		memnew(OpenXRSpatialEntitiesUnified());
	}
	return singleton;
}

OpenXRSpatialEntitiesUnified::OpenXRSpatialEntitiesUnified() {
	singleton = this;
}

OpenXRSpatialEntitiesUnified::~OpenXRSpatialEntitiesUnified() {
	singleton = nullptr;
}

void OpenXRSpatialEntitiesUnified::_on_session_created(uint64_t p_session) {
	OpenXRSpatialEntityExtension *se_extension = OpenXRSpatialEntityExtension::get_singleton();
	ERR_FAIL_NULL(se_extension);

	se_extension->connect("spatial_discovery_recommended", callable_mp(this, &OpenXRSpatialEntitiesUnified::_on_spatial_discovery_recommended));
}

void OpenXRSpatialEntitiesUnified::_on_process() {
	for (auto &[unified_spatial_context, unified_spatial_context_data] : unified_spatial_contexts) {
		// discover persisted anchors
		_try_start_discovery<OpenXRSpatialAnchorCapability>(unified_spatial_context, unified_spatial_context_data, CAPABILITY_FLAGS_ANCHOR | CAPABILITY_FLAGS_ANCHOR_PERSISTENCE, nullptr, nullptr);

		// discover planes
		_try_start_discovery<OpenXRSpatialPlaneTrackingCapability>(unified_spatial_context, unified_spatial_context_data, CAPABILITY_FLAGS_PLANE, nullptr, nullptr);

		// discover markers (all markers are queried with a single query)
		_try_start_discovery<OpenXRSpatialMarkerTrackingCapability>(unified_spatial_context, unified_spatial_context_data, unified_spatial_context_data->capability_flags & CAPABILITY_FLAGS_MARKER_TRACKING, nullptr, nullptr);

		// update anchors
		_try_do_update<OpenXRSpatialAnchorCapability>(unified_spatial_context, unified_spatial_context_data, CAPABILITY_FLAGS_ANCHOR, nullptr, nullptr);

		// update markers (all markers are queried with a single query)
		_try_do_update<OpenXRSpatialMarkerTrackingCapability>(unified_spatial_context, unified_spatial_context_data, unified_spatial_context_data->capability_flags & CAPABILITY_FLAGS_MARKER_TRACKING, nullptr, nullptr);
	}
}

void OpenXRSpatialEntitiesUnified::_on_session_destroyed() {
	OpenXRSpatialEntityExtension *se_extension = OpenXRSpatialEntityExtension::get_singleton();
	ERR_FAIL_NULL(se_extension);

	for (auto &[unified_spatial_context, unified_spatial_context_data] : unified_spatial_contexts) {
		se_extension->free_spatial_entity(unified_spatial_context);
		memdelete(unified_spatial_context_data);
	}
	unified_spatial_contexts.clear();

	se_extension->disconnect("spatial_discovery_recommended", callable_mp(this, &OpenXRSpatialEntitiesUnified::_on_spatial_discovery_recommended));
}

void OpenXRSpatialEntitiesUnified::create_unified_spatial_context(BitField<CapabilityFlags> p_capability_flags, const Callable &p_user_callback, const Dictionary &p_capability_options, BitField<CapabilityFlags> p_capability_flags_automatic_discovery_query, BitField<CapabilityFlags> p_capability_flags_automatic_update_query) {
	ERR_FAIL_COND(p_user_callback.is_null());

	UnifiedSpatialContextData *unified_spatial_context_data;
	unified_spatial_context_data = memnew(UnifiedSpatialContextData);
	unified_spatial_context_data->capability_flags = p_capability_flags;
	unified_spatial_context_data->capability_flags_automatic_discovery_query = p_capability_flags_automatic_discovery_query;
	unified_spatial_context_data->capability_flags_automatic_update_query = p_capability_flags_automatic_update_query;

	TypedArray<OpenXRSpatialCapabilityConfigurationBaseHeader> capability_configurations;
	Ref<OpenXRStructureBase> next;

	_on_create_unified_spatial_context_dependency_completed(RID(), (uint64_t)unified_spatial_context_data, p_user_callback, p_capability_options, capability_configurations, next, CAPABILITY_FLAGS_NONE, CAPABILITY_FLAGS_NONE);
}

void OpenXRSpatialEntitiesUnified::destroy_unified_spatial_context(RID p_unified_spatial_context) {
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return;
	}

	OpenXRSpatialEntityExtension *se_extension = OpenXRSpatialEntityExtension::get_singleton();
	ERR_FAIL_NULL(se_extension);

	se_extension->free_spatial_entity(p_unified_spatial_context);

	memdelete(unified_spatial_contexts[p_unified_spatial_context]);
	unified_spatial_contexts.erase(p_unified_spatial_context);
}

BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> OpenXRSpatialEntitiesUnified::get_capability_flags(RID p_unified_spatial_context) const {
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return CAPABILITY_FLAGS_NONE;
	}

	return unified_spatial_contexts[p_unified_spatial_context]->capability_flags;
}

RID OpenXRSpatialEntitiesUnified::get_persistence_context(RID p_unified_spatial_context) const {
	if (!is_capability_supported(p_unified_spatial_context, CAPABILITY_FLAGS_ANCHOR_PERSISTENCE)) {
		return RID();
	}

	return unified_spatial_contexts[p_unified_spatial_context]->persistence_context;
}

BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> OpenXRSpatialEntitiesUnified::set_automatic_update(RID p_unified_spatial_context, BitField<CapabilityFlags> p_capability_flags, bool p_enable, bool p_discovery) {
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return CAPABILITY_FLAGS_NONE;
	}

	UnifiedSpatialContextData *unified_spatial_context_data = unified_spatial_contexts[p_unified_spatial_context];
	int flags = unified_spatial_context_data->capability_flags & p_capability_flags;
	return p_discovery ? _set_automatic_update(unified_spatial_context_data->capability_flags_automatic_discovery_query, flags, p_enable) : _set_automatic_update(unified_spatial_context_data->capability_flags_automatic_update_query, flags, p_enable);
}

BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> OpenXRSpatialEntitiesUnified::get_automatic_update(RID p_unified_spatial_context, bool p_discovery) const {
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return CAPABILITY_FLAGS_NONE;
	}

	return p_discovery ? unified_spatial_contexts[p_unified_spatial_context]->capability_flags_automatic_discovery_query : unified_spatial_contexts[p_unified_spatial_context]->capability_flags_automatic_update_query;
}

bool OpenXRSpatialEntitiesUnified::is_automatic_update(RID p_unified_spatial_context, BitField<CapabilityFlags> p_capability_flags, bool p_discovery) const {
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return false;
	}

	return p_capability_flags == (p_capability_flags & (p_discovery ? unified_spatial_contexts[p_unified_spatial_context]->capability_flags_automatic_discovery_query : unified_spatial_contexts[p_unified_spatial_context]->capability_flags_automatic_update_query));
}

bool OpenXRSpatialEntitiesUnified::is_capability_supported(RID p_unified_spatial_context, BitField<CapabilityFlags> p_capability_flags) const {
	return unified_spatial_contexts.has(p_unified_spatial_context) && p_capability_flags == (unified_spatial_contexts[p_unified_spatial_context]->capability_flags & p_capability_flags);
}

Array OpenXRSpatialEntitiesUnified::get_capability_options(RID p_unified_spatial_context) const {
	Array ret;
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return ret;
	}

	UnifiedSpatialContextData *context_data = unified_spatial_contexts[p_unified_spatial_context];
	ERR_FAIL_NULL_V(context_data, ret);

	ret.resize(2);

	{
		Dictionary discovery_info;
		for (const auto &[capability_flags, discovery_data] : context_data->discovery_datas) {
			Dictionary options;

			for (const auto &[capability_option, enabled] : discovery_data.options) {
				options.set(capability_option, enabled);
			}

			discovery_info.set(capability_flags, options);
		}
		ret[0] = discovery_info;
	}

	{
		Dictionary update_info;
		for (const auto &[capability_flags, update_data] : context_data->update_datas) {
			Dictionary options;

			for (const auto &[capability_option, enabled] : update_data.options) {
				options.set(capability_option, enabled);
			}

			update_info.set(capability_flags, options);
		}
		ret[1] = update_info;
	}

	return ret;
}

void OpenXRSpatialEntitiesUnified::enable_discovery_capability_option(RID p_unified_spatial_context, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, bool p_enable) {
	_enable_capability_option<true>(p_unified_spatial_context, p_capability_flags, p_capability_option, p_enable);
}

void OpenXRSpatialEntitiesUnified::enable_update_capability_option(RID p_unified_spatial_context, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, bool p_enable) {
	_enable_capability_option<false>(p_unified_spatial_context, p_capability_flags, p_capability_option, p_enable);
}

void OpenXRSpatialEntitiesUnified::_on_create_unified_spatial_context_dependency_completed(RID p_completed_dependency, uint64_t p_unified_spatial_context_data, const Callable &p_user_callback, const Dictionary &p_capability_options, TypedArray<OpenXRSpatialCapabilityConfigurationBaseHeader> p_capability_configurations, Ref<OpenXRStructureBase> p_next, BitField<CapabilityFlags> p_capability_flags_completed, BitField<CapabilityFlags> p_capability_flags_just_completed) {
	UnifiedSpatialContextData *unified_spatial_context_data = (UnifiedSpatialContextData *)p_unified_spatial_context_data;
	if (CAPABILITY_FLAGS_ANCHOR_TRACKING != (p_capability_flags_completed & CAPABILITY_FLAGS_ANCHOR_TRACKING) && unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_ANCHOR_TRACKING)) {
		OpenXRSpatialAnchorCapability *capability = OpenXRSpatialAnchorCapability::get_singleton();

		if (capability != nullptr && capability->is_spatial_anchor_supported() && unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_ANCHOR)) {
			if (capability->is_spatial_persistence_supported() && unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_ANCHOR_PERSISTENCE)) {
				if (!p_capability_flags_completed.has_flag(CAPABILITY_FLAGS_ANCHOR_PERSISTENCE)) {
					if (p_capability_flags_just_completed == CAPABILITY_FLAGS_ANCHOR_PERSISTENCE) {
						unified_spatial_context_data->persistence_context = p_completed_dependency;

						Ref<OpenXRSpatialContextPersistenceConfig> persistence_configuration;
						persistence_configuration.instantiate();
						persistence_configuration->add_persistence_context(p_completed_dependency);

						// Add persistence configuration to the next-chain
						persistence_configuration->set_next(p_next);
						p_next = persistence_configuration;

						unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_ANCHOR | CAPABILITY_FLAGS_ANCHOR_PERSISTENCE, CAPABILITY_OPTIONS_ENABLE_ANCHOR, p_capability_options);
						unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_ANCHOR | CAPABILITY_FLAGS_ANCHOR_PERSISTENCE, CAPABILITY_OPTIONS_ENABLE_ANCHOR_PERSISTENCE, p_capability_options);

						// So we don't do this again in any other potential recursive calls
						p_capability_flags_completed.set_flag(CAPABILITY_FLAGS_ANCHOR_PERSISTENCE);
					} else {
						// Wait for the persistence context to complete before continuing

						Callable callable = callable_mp(this, &OpenXRSpatialEntitiesUnified::_on_create_unified_spatial_context_dependency_completed).bind(p_unified_spatial_context_data, p_user_callback, p_capability_options, p_capability_configurations, p_next, p_capability_flags_completed, CAPABILITY_FLAGS_ANCHOR_PERSISTENCE);
						if (p_capability_options.has(CAPABILITY_OPTIONS_ANCHOR_PERSISTENCE_SCOPE)) {
							capability->create_persistence_context((OpenXRSpatialAnchorCapability::PersistenceScope)((int)p_capability_options[CAPABILITY_OPTIONS_ANCHOR_PERSISTENCE_SCOPE]), callable);
							return;
						}

						capability->create_default_persistence_context(callable);
						return;
					}
				}
			} else {
				// Remove the flag since the resulting context will not have this extension
				unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_ANCHOR_PERSISTENCE);
			}

			unified_spatial_context_data->add_update_capability_option(CAPABILITY_FLAGS_ANCHOR, CAPABILITY_OPTIONS_ENABLE_ANCHOR, p_capability_options);

			Ref<OpenXRSpatialEntitiesUnifiedConfigurationAnchor> config;
			config.instantiate();
			config->init(unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_ANCHOR_PERSISTENCE));
			p_capability_configurations.push_back(config);
		} else {
			// Remove all anchor flags since the resulting context will not have these capabilities
			unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_ANCHOR_TRACKING);
		}

		// So we don't do this again in any other potential recursive calls
		p_capability_flags_completed.set_flag(CAPABILITY_FLAGS_ANCHOR_TRACKING);
	}

	if (CAPABILITY_FLAGS_PLANE_TRACKING != (p_capability_flags_completed & CAPABILITY_FLAGS_PLANE_TRACKING) && unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_PLANE_TRACKING)) {
		OpenXRSpatialPlaneTrackingCapability *capability = OpenXRSpatialPlaneTrackingCapability::get_singleton();

		if (capability != nullptr && capability->is_supported() && unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_PLANE)) {
			// This is the only (easy) way to find out if mesh, polygons, and/or labels are supported
			Ref<OpenXRSpatialCapabilityConfigurationPlaneTracking> plane_config;
			plane_config.instantiate();

			unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_PLANE, CAPABILITY_OPTIONS_ENABLE_PLANE_BOUNDED_2D, p_capability_options, false);
			unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_PLANE, CAPABILITY_OPTIONS_ENABLE_PLANE_ALIGNMENT, p_capability_options, false);

			// only add these options if the XR runtime supports them
			if (plane_config->supports_mesh_2d()) {
				unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_PLANE, CAPABILITY_OPTIONS_ENABLE_PLANE_MESH_2D, p_capability_options, false);
			}

			if (plane_config->supports_polygons()) {
				unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_PLANE, CAPABILITY_OPTIONS_ENABLE_PLANE_POLYGON_2D, p_capability_options, false);
			}

			if (plane_config->supports_labels()) {
				unified_spatial_context_data->add_discovery_capability_option(CAPABILITY_FLAGS_PLANE, CAPABILITY_OPTIONS_ENABLE_PLANE_SEMANTIC_LABEL, p_capability_options, false);
			}

			Ref<OpenXRSpatialEntitiesUnifiedConfigurationPlane> unified_plane_config;
			unified_plane_config.instantiate();
			unified_plane_config->init(plane_config->supports_mesh_2d(), plane_config->supports_polygons(), plane_config->supports_labels());
			p_capability_configurations.push_back(unified_plane_config);

			p_capability_flags_completed.set_flag(CAPABILITY_FLAGS_PLANE);
		} else {
			// Remove all plane flags since the resulting context will not have these capabilities
			unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_PLANE_TRACKING);
		}

		// So we don't do this again in any other potential recursive calls
		p_capability_flags_completed.set_flag(CAPABILITY_FLAGS_PLANE_TRACKING);
	}

	if (CAPABILITY_FLAGS_MARKER_TRACKING != (p_capability_flags_completed & CAPABILITY_FLAGS_MARKER_TRACKING) && unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_TRACKING)) {
		OpenXRSpatialMarkerTrackingCapability *marker_capability = OpenXRSpatialMarkerTrackingCapability::get_singleton();
		if (marker_capability == nullptr) {
			unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_MARKER_TRACKING);
		} else {
			if (unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_QRCODE) && marker_capability->is_qrcode_supported()) {
				Ref<OpenXRSpatialCapabilityConfigurationQrCode> config;
				config.instantiate();
				p_capability_configurations.push_back(config);
			} else {
				unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_MARKER_QRCODE);
			}

			if (unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_MICRO_QRCODE) && marker_capability->is_micro_qrcode_supported()) {
				Ref<OpenXRSpatialCapabilityConfigurationMicroQrCode> config;
				config.instantiate();
				p_capability_configurations.push_back(config);
			} else {
				unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_MARKER_MICRO_QRCODE);
			}

			if (unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_ARUCO) && marker_capability->is_aruco_supported()) {
				Ref<OpenXRSpatialCapabilityConfigurationAruco> config;
				config.instantiate();

				if (p_capability_options.has(CAPABILITY_OPTIONS_MARKER_ARUCO_DICT)) {
					config->set_aruco_dict((OpenXRSpatialCapabilityConfigurationAruco::ArucoDict)((int)p_capability_options[CAPABILITY_OPTIONS_MARKER_ARUCO_DICT]));
				}

				p_capability_configurations.push_back(config);
			} else {
				unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_MARKER_ARUCO);
			}

			if (unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_APRIL) && marker_capability->is_april_tag_supported()) {
				Ref<OpenXRSpatialCapabilityConfigurationAprilTag> config;
				config.instantiate();

				if (p_capability_options.has(CAPABILITY_OPTIONS_MARKER_APRIL_DICT)) {
					config->set_april_dict((OpenXRSpatialCapabilityConfigurationAprilTag::AprilTagDict)((int)p_capability_options[CAPABILITY_OPTIONS_MARKER_APRIL_DICT]));
				}

				p_capability_configurations.push_back(config);
			} else {
				unified_spatial_context_data->capability_flags.clear_flag(CAPABILITY_FLAGS_MARKER_APRIL);
			}

			if (unified_spatial_context_data->capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_TRACKING)) {
				// All marker capabilities can be queried with one query (for both discovery and update), so group them together
				BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> marker_capability_flags = unified_spatial_context_data->capability_flags & CAPABILITY_FLAGS_MARKER_TRACKING;

				if (marker_capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_QRCODE)) {
					unified_spatial_context_data->add_discovery_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_QRCODE, p_capability_options, false);
					unified_spatial_context_data->add_update_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_QRCODE, p_capability_options, false);
				}

				if (marker_capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_MICRO_QRCODE)) {
					unified_spatial_context_data->add_discovery_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_MICRO_QRCODE, p_capability_options, false);
					unified_spatial_context_data->add_update_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_MICRO_QRCODE, p_capability_options, false);
				}

				if (marker_capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_ARUCO)) {
					unified_spatial_context_data->add_discovery_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_ARUCO, p_capability_options, false);
					unified_spatial_context_data->add_update_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_ARUCO, p_capability_options, false);
				}

				if (marker_capability_flags.has_flag(CAPABILITY_FLAGS_MARKER_APRIL)) {
					unified_spatial_context_data->add_discovery_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_APRIL, p_capability_options, false);
					unified_spatial_context_data->add_update_capability_option(marker_capability_flags, CAPABILITY_OPTIONS_ENABLE_MARKER_APRIL, p_capability_options, false);
				}
			}
		}

		// So we don't do this again in any other potential recursive calls
		p_capability_flags_completed.set_flag(CAPABILITY_FLAGS_MARKER_TRACKING);
	}

	// Now that capability_flags are finalized, ensure the automatic update flags are set correctly
	unified_spatial_context_data->capability_flags_automatic_discovery_query = unified_spatial_context_data->capability_flags & unified_spatial_context_data->capability_flags_automatic_discovery_query;
	unified_spatial_context_data->capability_flags_automatic_update_query = unified_spatial_context_data->capability_flags & unified_spatial_context_data->capability_flags_automatic_update_query;

	OpenXRSpatialEntityExtension *se_extension = OpenXRSpatialEntityExtension::get_singleton();
	ERR_FAIL_NULL(se_extension);
	se_extension->create_spatial_context(p_capability_configurations, p_next, callable_mp(this, &OpenXRSpatialEntitiesUnified::_on_spatial_context_created).bind(p_unified_spatial_context_data, p_user_callback));
}

void OpenXRSpatialEntitiesUnified::_on_spatial_context_created(RID p_spatial_context, uint64_t p_unified_spatial_context_data, const Callable &p_user_callback) {
	ERR_FAIL_COND(p_user_callback.is_null());

	unified_spatial_contexts[p_spatial_context] = (UnifiedSpatialContextData *)p_unified_spatial_context_data;

	p_user_callback.call(p_spatial_context);
}

void OpenXRSpatialEntitiesUnified::_on_spatial_discovery_recommended(RID p_spatial_context) {
	if (unified_spatial_contexts.has(p_spatial_context)) {
		// Trigger new discovery.
		for (auto &[_, discovery_data] : unified_spatial_contexts[p_spatial_context]->discovery_datas) {
			discovery_data.need_discovery = true;
		}
	}
}

template <bool IsDiscovery>
void OpenXRSpatialEntitiesUnified::_enable_capability_option(RID p_unified_spatial_context, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, bool p_enable) {
	if (!unified_spatial_contexts.has(p_unified_spatial_context)) {
		return;
	}

	UnifiedSpatialContextData *context_data = unified_spatial_contexts[p_unified_spatial_context];

	// Have to use a lambda since there is no constexpr ternary operator
	auto &datas = [context_data]() -> auto & {
		if constexpr (IsDiscovery) {
			return context_data->discovery_datas;
		} else {
			return context_data->update_datas;
		}
	}
	();

	for (auto &[capability_flags, data] : datas) {
		// enable the option when at least p_capability_flags matches
		if (p_capability_flags == (capability_flags & p_capability_flags)) {
			data.enable_capability_option(p_capability_option, p_enable, false, false);
		}
	}
}

template <typename T>
void OpenXRSpatialEntitiesUnified::_try_start_discovery(RID p_unified_spatial_context, UnifiedSpatialContextData *p_unified_spatial_context_data, BitField<CapabilityFlags> p_capability_flags, Ref<OpenXRStructureBase> p_next_snapshot_create, Ref<OpenXRStructureBase> p_next_snapshot_query) {
	if (0 == p_capability_flags || p_capability_flags != (p_unified_spatial_context_data->capability_flags & p_capability_flags) || p_capability_flags != (p_unified_spatial_context_data->capability_flags_automatic_discovery_query & p_capability_flags)) {
		return;
	}

	UnifiedSpatialContextData::DiscoveryData &discovery_data = p_unified_spatial_context_data->discovery_datas[p_capability_flags];
	if (_can_skip_query(discovery_data.options, discovery_data.required_options, discovery_data.components)) {
		// set to -1 to ensure this capability is updated as soon as it gets a new component to query
		discovery_data.discovery_cooldown = -1;
		return;
	}

	discovery_data.discovery_cooldown--;
	if (discovery_data.need_discovery && discovery_data.discovery_cooldown <= 0) {
		if (discovery_data.discovery_query_result.is_valid() && discovery_data.discovery_query_result->get_status() == OpenXRFutureResult::RESULT_RUNNING) {
			WARN_PRINT(vformat("A previous discovery_data future for [%s] is taking a long time to finish", T::get_singleton()->get_class()));
			return;
		}

		discovery_data.discovery_query_result = T::get_singleton()->start_entity_discovery(p_unified_spatial_context, discovery_data.components, p_next_snapshot_create, p_next_snapshot_query);
		discovery_data.need_discovery = false;
		discovery_data.discovery_cooldown = 60;
	}
}

template <typename T>
void OpenXRSpatialEntitiesUnified::_try_do_update(RID p_unified_spatial_context, UnifiedSpatialContextData *p_unified_spatial_context_data, BitField<CapabilityFlags> p_capability_flags, Ref<OpenXRStructureBase> p_next_snapshot_create, Ref<OpenXRStructureBase> p_next_snapshot_query) {
	if (0 == p_capability_flags || p_capability_flags != (p_unified_spatial_context_data->capability_flags & p_capability_flags) || p_capability_flags != (p_unified_spatial_context_data->capability_flags_automatic_update_query & p_capability_flags)) {
		return;
	}

	UnifiedSpatialContextData::UpdateData &update_data = p_unified_spatial_context_data->update_datas[p_capability_flags];
	if (_can_skip_query(update_data.options, update_data.required_options, update_data.components)) {
		return;
	}

	T::get_singleton()->do_entity_update(p_unified_spatial_context, update_data.components, nullptr, nullptr);
}

void OpenXRSpatialEntitiesUnified::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_unified_spatial_context", "capability_flags", "callback", "capability_options", "enable_automatic_discovery_query", "enable_automatic_update_query"), &OpenXRSpatialEntitiesUnified::create_unified_spatial_context, DEFVAL(Dictionary()), DEFVAL(CAPABILITY_FLAGS_ALL), DEFVAL(CAPABILITY_FLAGS_ALL));
	ClassDB::bind_method(D_METHOD("destroy_unified_spatial_context", "unified_spatial_context"), &OpenXRSpatialEntitiesUnified::destroy_unified_spatial_context);
	ClassDB::bind_method(D_METHOD("is_capability_supported", "unified_spatial_context", "capability_flags"), &OpenXRSpatialEntitiesUnified::is_capability_supported);
	ClassDB::bind_method(D_METHOD("get_capability_options", "unified_spatial_context"), &OpenXRSpatialEntitiesUnified::get_capability_options);
	ClassDB::bind_method(D_METHOD("get_capability_flags", "unified_spatial_context"), &OpenXRSpatialEntitiesUnified::get_capability_flags);
	ClassDB::bind_method(D_METHOD("get_persistence_context", "unified_spatial_context"), &OpenXRSpatialEntitiesUnified::get_persistence_context);
	ClassDB::bind_method(D_METHOD("set_automatic_update", "unified_spatial_context", "capability_flags", "enable", "discovery"), &OpenXRSpatialEntitiesUnified::set_automatic_update);
	ClassDB::bind_method(D_METHOD("get_automatic_update", "unified_spatial_context", "discovery"), &OpenXRSpatialEntitiesUnified::get_automatic_update);
	ClassDB::bind_method(D_METHOD("is_automatic_update", "unified_spatial_context", "capability_flags", "discovery"), &OpenXRSpatialEntitiesUnified::is_automatic_update);
	ClassDB::bind_method(D_METHOD("enable_discovery_capability_option", "unified_spatial_context", "capability_flags", "capability_option", "enable"), &OpenXRSpatialEntitiesUnified::enable_discovery_capability_option);
	ClassDB::bind_method(D_METHOD("enable_update_capability_option", "unified_spatial_context", "capability_flags", "capability_option", "enable"), &OpenXRSpatialEntitiesUnified::enable_update_capability_option);

	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_NONE);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_ANCHOR);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_ANCHOR_PERSISTENCE);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_ANCHOR_TRACKING);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_PLANE);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_PLANE_TRACKING);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_MARKER_QRCODE);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_MARKER_MICRO_QRCODE);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_MARKER_ARUCO);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_MARKER_APRIL);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_MARKER_TRACKING);
	BIND_BITFIELD_FLAG(CAPABILITY_FLAGS_ALL);

	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ANCHOR_PERSISTENCE_SCOPE);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_MARKER_ARUCO_DICT);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_MARKER_APRIL_DICT);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_ANCHOR);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_ANCHOR_PERSISTENCE);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_PLANE_BOUNDED_2D);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_PLANE_ALIGNMENT);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_PLANE_MESH_2D);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_PLANE_POLYGON_2D);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_PLANE_SEMANTIC_LABEL);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_MARKER_QRCODE);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_MARKER_MICRO_QRCODE);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_MARKER_ARUCO);
	BIND_ENUM_CONSTANT(CAPABILITY_OPTIONS_ENABLE_MARKER_APRIL);
}

bool OpenXRSpatialEntitiesUnified::UnifiedSpatialContextData::add_discovery_capability_option(BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, const Dictionary &p_capability_options, bool p_adding_required) {
	return discovery_datas[p_capability_flags].enable_capability_option(p_capability_option, !p_capability_options.has(p_capability_option) || (bool)p_capability_options[p_capability_option], true, p_adding_required);
}

bool OpenXRSpatialEntitiesUnified::UnifiedSpatialContextData::add_update_capability_option(BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, const Dictionary &p_capability_options, bool p_adding_required) {
	return update_datas[p_capability_flags].enable_capability_option(p_capability_option, !p_capability_options.has(p_capability_option) || !(bool)p_capability_options[p_capability_option], true, p_adding_required);
}

bool OpenXRSpatialEntitiesUnified::UnifiedSpatialContextData::DiscoveryData::enable_capability_option(CapabilityOptions p_capability_option, bool p_enable, bool p_adding, bool p_adding_required) {
	if (!p_adding && !options.has(p_capability_option)) {
		return false;
	}

	if (p_adding_required) {
		// programmer error if p_adding is false (the only time we can specify an option is required is
		// when adding new options)
		ERR_FAIL_COND_V(!p_adding, false);
		required_options.push_back(p_capability_option);
	}

	// programmer error if we've already added this option.
	// As of this writing options are added exactly once
	ERR_FAIL_COND_V(p_adding && options.has(p_capability_option), false);

	options[p_capability_option] = p_enable;

	switch (p_capability_option) {
		case CAPABILITY_OPTIONS_ANCHOR_PERSISTENCE_SCOPE:
		case CAPABILITY_OPTIONS_MARKER_ARUCO_DICT:
		case CAPABILITY_OPTIONS_MARKER_APRIL_DICT:
			// Programmer error.
			// These options can only be configured when creating the spatial context (see
			// _on_create_unified_spatial_context_dependency_completed())
			UtilityFunctions::printerr("OpenXR: Received capability option that is only configurable at initialization: ", p_capability_option);
			return false;
		case CAPABILITY_OPTIONS_ENABLE_ANCHOR:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentAnchorList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_ANCHOR_PERSISTENCE:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentPersistenceList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_PLANE_BOUNDED_2D:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentBounded2DList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_PLANE_ALIGNMENT:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentPlaneAlignmentList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_PLANE_MESH_2D:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentMesh2DList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_PLANE_POLYGON_2D:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentPolygon2DList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_PLANE_SEMANTIC_LABEL:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentPlaneSemanticLabelList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_QRCODE:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationQrCode");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_MICRO_QRCODE:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationMicroQrCode");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_ARUCO:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationAruco");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_APRIL:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationAprilTag");
			break;
	}

	return true;
}

bool OpenXRSpatialEntitiesUnified::UnifiedSpatialContextData::UpdateData::enable_capability_option(CapabilityOptions p_capability_option, bool p_enable, bool p_adding, bool p_adding_required) {
	if (!p_adding && !options.has(p_capability_option)) {
		return false;
	}

	if (p_adding_required) {
		// programmer error if p_adding is false (the only time required options are specified is while
		// adding new options)
		ERR_FAIL_COND_V(!p_adding, false);
		required_options.push_back(p_capability_option);
	}

	// programmer error if we've already added this option.
	// As of this writing options are added exactly once
	ERR_FAIL_COND_V(p_adding && options.has(p_capability_option), false);

	options[p_capability_option] = p_enable;
	switch (p_capability_option) {
		case CAPABILITY_OPTIONS_ENABLE_ANCHOR:
			_append_component_data(components, p_enable, "OpenXRSpatialComponentAnchorList");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_QRCODE:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationQrCode");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_MICRO_QRCODE:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationMicroQrCode");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_ARUCO:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationAruco");
			break;
		case CAPABILITY_OPTIONS_ENABLE_MARKER_APRIL:
			_append_component_data(components, p_enable, "OpenXRSpatialCapabilityConfigurationAprilTag");
			break;
		default:
			UtilityFunctions::printerr("OpenXR: Received unsupported capability option for update query: ", p_capability_option);
			break;
	}

	return true;
}
