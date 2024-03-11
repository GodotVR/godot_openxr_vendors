/**************************************************************************/
/*  openxr_fb_spatial_entity_query_extension_wrapper.h                    */
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
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util.h"

using namespace godot;

// Wrapper for the set of Facebook XR spatial entity query extension.
class OpenXRFbSpatialEntityQueryExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSpatialEntityQueryExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	Dictionary _get_requested_extensions() override;
	void _on_instance_created(uint64_t instance) override;
	void _on_instance_destroyed() override;
	virtual bool _on_event_polled(const void *event) override;

	bool is_spatial_entity_query_supported() {
		return fb_spatial_entity_query_ext;
	}

	static OpenXRFbSpatialEntityQueryExtensionWrapper *get_singleton();

	typedef void (*QueryCompleteCallback)(const Vector<XrSpaceQueryResultFB> &p_results, void *p_userdata);

	// Attempts to query spatial entities given an XrSpaceQueryInfoFB. The callback will run to
	// deliver results when they are available.
	bool query_spatial_entities(const XrSpaceQueryInfoBaseHeaderFB *p_info, QueryCompleteCallback p_callback, void *p_userdata);

	OpenXRFbSpatialEntityQueryExtensionWrapper();
	~OpenXRFbSpatialEntityQueryExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrQuerySpacesFB,
			(XrSession), session,
			(const XrSpaceQueryInfoBaseHeaderFB *), info,
			(XrAsyncRequestIdFB *), requestId)

	EXT_PROTO_XRRESULT_FUNC3(xrRetrieveSpaceQueryResultsFB,
			(XrSession), session,
			(XrAsyncRequestIdFB), requestId,
			(XrSpaceQueryResultsFB *), results)

	bool initialize_fb_spatial_entity_query_extension(const XrInstance &instance);
	void on_space_query_results(const XrEventDataSpaceQueryResultsAvailableFB *event);
	void on_space_query_complete(const XrEventDataSpaceQueryCompleteFB *event);

	HashMap<String, bool *> request_extensions;

	struct QueryInfo {
		QueryCompleteCallback callback = nullptr;
		void *userdata = nullptr;
		Vector<XrSpaceQueryResultFB> results;

		QueryInfo() { }

		QueryInfo(QueryCompleteCallback p_callback, void *p_userdata) {
			callback = p_callback;
			userdata = p_userdata;
		}
	};

	HashMap<XrAsyncRequestIdFB, QueryInfo> queries;

	void cleanup();

	static OpenXRFbSpatialEntityQueryExtensionWrapper *singleton;

	bool fb_spatial_entity_query_ext = false;
};
