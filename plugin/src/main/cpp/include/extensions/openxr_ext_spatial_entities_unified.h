/**************************************************************************/
/*  openxr_ext_spatial_entities_unified.h                                 */
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

#pragma once

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/classes/open_xr_future_result.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_anchor.hpp>
#include <godot_cpp/classes/open_xr_spatial_capability_configuration_base_header.hpp>
#include <godot_cpp/classes/open_xr_spatial_component_data.hpp>
#include <godot_cpp/classes/open_xr_structure_base.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

// Unified anchor capability configuration
class OpenXRSpatialEntitiesUnifiedConfigurationAnchor : public OpenXRSpatialCapabilityConfigurationBaseHeader {
	GDCLASS(OpenXRSpatialEntitiesUnifiedConfigurationAnchor, OpenXRSpatialCapabilityConfigurationBaseHeader);

public:
	virtual uint64_t _get_configuration() override;

	void init(bool p_enable_persistence);

protected:
	static void _bind_methods();

private:
	PackedInt64Array _get_enabled_components() const;

	LocalVector<XrSpatialComponentTypeEXT> anchor_enabled_components;
	XrSpatialCapabilityConfigurationAnchorEXT anchor_config = { XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_ANCHOR_EXT, nullptr, XR_SPATIAL_CAPABILITY_ANCHOR_EXT, 0, nullptr };
};

// Unified plane capability configuration
class OpenXRSpatialEntitiesUnifiedConfigurationPlane : public OpenXRSpatialCapabilityConfigurationBaseHeader {
	GDCLASS(OpenXRSpatialEntitiesUnifiedConfigurationPlane, OpenXRSpatialCapabilityConfigurationBaseHeader);

public:
	virtual uint64_t _get_configuration() override;

	void init(bool p_supports_mesh_2d, bool p_supports_polygons, bool p_supports_labels);

protected:
	static void _bind_methods();

private:
	PackedInt64Array _get_enabled_components() const;

	LocalVector<XrSpatialComponentTypeEXT> plane_enabled_components;
	XrSpatialCapabilityConfigurationPlaneTrackingEXT plane_config = { XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_PLANE_TRACKING_EXT, nullptr, XR_SPATIAL_CAPABILITY_PLANE_TRACKING_EXT, 0, nullptr };
};

class OpenXRSpatialEntitiesUnified : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRSpatialEntitiesUnified, OpenXRExtensionWrapper);

public:
	static OpenXRSpatialEntitiesUnified *get_singleton();

	OpenXRSpatialEntitiesUnified();
	virtual ~OpenXRSpatialEntitiesUnified() override;

	virtual void _on_session_created(uint64_t p_session) override;
	virtual void _on_process() override;
	virtual void _on_session_destroyed() override;

	enum CapabilityFlags {
		CAPABILITY_FLAGS_NONE = 0,

		CAPABILITY_FLAGS_ANCHOR = 1 << 0,
		CAPABILITY_FLAGS_ANCHOR_PERSISTENCE = 1 << 1,

		CAPABILITY_FLAGS_PLANE = 1 << 2,

		CAPABILITY_FLAGS_MARKER_QRCODE = 1 << 3,
		CAPABILITY_FLAGS_MARKER_MICRO_QRCODE = 1 << 4,
		CAPABILITY_FLAGS_MARKER_ARUCO = 1 << 5,
		CAPABILITY_FLAGS_MARKER_APRIL = 1 << 6,

		CAPABILITY_FLAGS_ANCHOR_TRACKING = CAPABILITY_FLAGS_ANCHOR | CAPABILITY_FLAGS_ANCHOR_PERSISTENCE,
		CAPABILITY_FLAGS_PLANE_TRACKING = CAPABILITY_FLAGS_PLANE,
		CAPABILITY_FLAGS_MARKER_TRACKING = CAPABILITY_FLAGS_MARKER_QRCODE | CAPABILITY_FLAGS_MARKER_MICRO_QRCODE | CAPABILITY_FLAGS_MARKER_ARUCO | CAPABILITY_FLAGS_MARKER_APRIL,
		CAPABILITY_FLAGS_ALL = CAPABILITY_FLAGS_ANCHOR_TRACKING | CAPABILITY_FLAGS_PLANE_TRACKING | CAPABILITY_FLAGS_MARKER_TRACKING
	};

	enum CapabilityOptions {
		// Capability options only available at context creation (due to XR runtime constraint)
		// (missing options in the Dictionary are assumed to be their default value.  The "default
		// value" depends on the capability)
		CAPABILITY_OPTIONS_ANCHOR_PERSISTENCE_SCOPE,
		CAPABILITY_OPTIONS_MARKER_ARUCO_DICT,
		CAPABILITY_OPTIONS_MARKER_APRIL_DICT,

		// Capability options available at and after context creation
		// (missing options in the Dictionary are assumed to be 'true' for the related CapabilityFlag)
		// Note that it is valid to create a spatial context with a capability and with the
		// option to disable it.  All this means is that perhaps the user the doesn't want the
		// capability polled immediately after creation, but may turn it on sometime after creation (see
		// enable_capability_option())

		// CAPABILITY_FLAGS_ANCHOR
		CAPABILITY_OPTIONS_ENABLE_ANCHOR,

		// CAPABILITY_FLAGS_ANCHOR_PERSISTENCE
		CAPABILITY_OPTIONS_ENABLE_ANCHOR_PERSISTENCE,

		// CAPABILITY_FLAGS_PLANE
		CAPABILITY_OPTIONS_ENABLE_PLANE_BOUNDED_2D,
		CAPABILITY_OPTIONS_ENABLE_PLANE_ALIGNMENT,
		CAPABILITY_OPTIONS_ENABLE_PLANE_MESH_2D,
		CAPABILITY_OPTIONS_ENABLE_PLANE_POLYGON_2D,
		CAPABILITY_OPTIONS_ENABLE_PLANE_SEMANTIC_LABEL,

		// CAPABILITY_FLAGS_MARKER_QRCODE
		CAPABILITY_OPTIONS_ENABLE_MARKER_QRCODE,

		// CAPABILITY_FLAGS_MARKER_MICRO_QRCODE
		CAPABILITY_OPTIONS_ENABLE_MARKER_MICRO_QRCODE,

		// CAPABILITY_FLAGS_MARKER_ARUCO
		CAPABILITY_OPTIONS_ENABLE_MARKER_ARUCO,

		// CAPABILITY_FLAGS_MARKER_APRIL
		CAPABILITY_OPTIONS_ENABLE_MARKER_APRIL,
	};

	// NOTE: this does not return Ref<OpenXRFutureResult> because sometimes it has async operations
	// that must complete before the context is created.  A valid callback must be provide for this
	// function to be useful.
	void create_unified_spatial_context(BitField<CapabilityFlags> p_capabilities, const Callable &p_user_callback, const Dictionary &p_capability_options = Dictionary(), BitField<CapabilityFlags> p_capability_flags_automatic_discovery_query = CAPABILITY_FLAGS_ALL, BitField<CapabilityFlags> p_capability_flags_automatic_update_query = CAPABILITY_FLAGS_ALL);
	void destroy_unified_spatial_context(RID p_unified_spatial_context);
	BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> get_capability_flags(RID p_unified_spatial_context) const;
	RID get_persistence_context(RID p_unified_spatial_context) const;
	BitField<CapabilityFlags> set_automatic_update(RID p_unified_spatial_context, BitField<CapabilityFlags> p_capability_flags, bool p_enable, bool p_discovery = true);
	BitField<CapabilityFlags> get_automatic_update(RID p_unified_spatial_context, bool p_discovery = true) const;
	bool is_automatic_update(RID p_unified_spatial_context, BitField<CapabilityFlags> p_capability_flags, bool p_discovery = true) const;
	bool is_capability_supported(RID p_unified_spatial_context, BitField<CapabilityFlags> p_capability_flags) const;
	Array get_capability_options(RID p_unified_spatial_context) const;

	void enable_discovery_capability_option(RID p_unified_spatial_context, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, bool p_enable);
	void enable_update_capability_option(RID p_unified_spatial_context, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, bool p_enable);

protected:
	static void _bind_methods();

private:
	static OpenXRSpatialEntitiesUnified *singleton;

	struct UnifiedSpatialContextData {
		bool add_discovery_capability_option(BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, const Dictionary &p_capability_options, bool p_adding_required = true);
		bool add_update_capability_option(BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, const Dictionary &p_capability_options, bool p_adding_required = true);

		struct DiscoveryData {
			bool enable_capability_option(CapabilityOptions p_capability_option, bool p_enable, bool p_adding, bool p_adding_required);

			HashMap<CapabilityOptions, bool> options;
			LocalVector<CapabilityOptions> required_options;
			TypedArray<OpenXRSpatialComponentData> components;

			bool need_discovery = true;
			int discovery_cooldown = 0;
			Ref<OpenXRFutureResult> discovery_query_result;
		};

		struct UpdateData {
			bool enable_capability_option(CapabilityOptions p_capability_option, bool p_enable, bool p_adding, bool p_adding_required);

			HashMap<CapabilityOptions, bool> options;
			LocalVector<CapabilityOptions> required_options;
			TypedArray<OpenXRSpatialComponentData> components;
		};

		HashMap<BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags>, DiscoveryData> discovery_datas;
		HashMap<BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags>, UpdateData> update_datas;

		// Anchor persistence context; there can be only one
		RID persistence_context;

		// This is equal to, or a subset of, the flags passed in create_unified_spatial_context().
		// It can be a subset if the runtime does not support requested feature(s).
		BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> capability_flags = CAPABILITY_FLAGS_NONE;

		// These are equal to, or a subset of, capability_flags.
		// Each bit indicates whether the capability is automatically updated or not.  Queries are
		// skipped if at least one bit is not on for a group of capabilities (see _try_start_discovery()
		// and _try_do_update())
		BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> capability_flags_automatic_discovery_query = CAPABILITY_FLAGS_NONE;
		BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> capability_flags_automatic_update_query = CAPABILITY_FLAGS_NONE;
	};

	void _on_create_unified_spatial_context_dependency_completed(RID p_completed_dependency, uint64_t p_unified_spatial_context_data, const Callable &p_user_callback, const Dictionary &p_capability_options, TypedArray<OpenXRSpatialCapabilityConfigurationBaseHeader> p_capability_configurations, Ref<OpenXRStructureBase> p_next, BitField<CapabilityFlags> p_capability_flags_completed, BitField<CapabilityFlags> p_capability_flags_just_completed);
	void _on_spatial_context_created(RID p_spatial_context, uint64_t p_unified_spatial_context_data, const Callable &p_user_callback);
	void _on_spatial_discovery_recommended(RID p_spatial_context);

	template <bool IsDiscovery>
	void _enable_capability_option(RID p_unified_spatial_context, BitField<OpenXRSpatialEntitiesUnified::CapabilityFlags> p_capability_flags, CapabilityOptions p_capability_option, bool p_enable);

	template <typename T>
	void _try_start_discovery(RID p_unified_spatial_context, UnifiedSpatialContextData *p_unified_spatial_context_data, BitField<CapabilityFlags> p_capability_flags, Ref<OpenXRStructureBase> p_next_snapshot_create, Ref<OpenXRStructureBase> p_next_snapshot_query);

	template <typename T>
	void _try_do_update(RID p_unified_spatial_context, UnifiedSpatialContextData *p_unified_spatial_context_data, BitField<CapabilityFlags> p_capability_flags, Ref<OpenXRStructureBase> p_next_snapshot_create, Ref<OpenXRStructureBase> p_next_snapshot_query);

	HashMap<RID, UnifiedSpatialContextData *> unified_spatial_contexts;
};

VARIANT_BITFIELD_CAST(OpenXRSpatialEntitiesUnified::CapabilityFlags);
VARIANT_ENUM_CAST(OpenXRSpatialEntitiesUnified::CapabilityOptions);
