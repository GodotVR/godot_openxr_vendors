/**************************************************************************/
/*  openxr_meta_colocation_discovery_extension_wrapper.h                  */
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

#include <openxr/openxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <map>

#include "util.h"

using namespace godot;

// Wrapper for the XR_META_colocation_discovery extension.
class OpenXRMetaColocationDiscoveryExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRMetaColocationDiscoveryExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	static OpenXRMetaColocationDiscoveryExtensionWrapper *get_singleton();

	OpenXRMetaColocationDiscoveryExtensionWrapper();
	~OpenXRMetaColocationDiscoveryExtensionWrapper();

	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t instance) override;
	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	void _on_instance_destroyed() override;
	bool _on_event_polled(const void *event) override;

	bool is_colocation_discovery_supported() const;

	bool start_advertisement(const PackedByteArray &p_buffer);
	bool stop_advertisement();
	bool start_discovery();
	bool stop_discovery();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrStartColocationAdvertisementMETA,
			(XrSession), session,
			(const XrColocationAdvertisementStartInfoMETA *), info,
			(XrAsyncRequestIdFB *), advertisementRequestId)

	EXT_PROTO_XRRESULT_FUNC3(xrStopColocationAdvertisementMETA,
			(XrSession), session,
			(const XrColocationAdvertisementStopInfoMETA *), info,
			(XrAsyncRequestIdFB *), requestId)

	EXT_PROTO_XRRESULT_FUNC3(xrStartColocationDiscoveryMETA,
			(XrSession), session,
			(const XrColocationDiscoveryStartInfoMETA *), info,
			(XrAsyncRequestIdFB *), discoveryRequestId)

	EXT_PROTO_XRRESULT_FUNC3(xrStopColocationDiscoveryMETA,
			(XrSession), session,
			(const XrColocationDiscoveryStopInfoMETA *), info,
			(XrAsyncRequestIdFB *), requestId)

	bool initialize_extension(const XrInstance instance);
	void cleanup();

	void on_start_advertisement_complete(const XrEventDataStartColocationAdvertisementCompleteMETA *event);
	void on_stop_advertisement_complete(const XrEventDataStopColocationAdvertisementCompleteMETA *event);
	void on_advertisement_complete(const XrEventDataColocationAdvertisementCompleteMETA *event);
	void on_start_discovery_complete(const XrEventDataStartColocationDiscoveryCompleteMETA *event);
	void on_discovery_result(const XrEventDataColocationDiscoveryResultMETA *event);
	void on_discovery_complete(const XrEventDataColocationDiscoveryCompleteMETA *event);
	void on_stop_discovery_complete(const XrEventDataStopColocationDiscoveryCompleteMETA *event);

	static OpenXRMetaColocationDiscoveryExtensionWrapper *singleton;

	std::map<godot::String, bool *> request_extensions;
	bool meta_colocation_discovery_ext = false;

	XrSystemColocationDiscoveryPropertiesMETA colocation_discovery_properties = {
		XR_TYPE_SYSTEM_COLOCATION_DISCOVERY_PROPERTIES_META, // type
		nullptr, // next
		XR_FALSE // supportsColocationDiscovery
	};

	bool is_advertising = false;
	bool is_discovering = false;
};
