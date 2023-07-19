#ifndef OPENXR_FB_SCENE_CAPTURE_EXTENSION_WRAPPER_H
#define OPENXR_FB_SCENE_CAPTURE_EXTENSION_WRAPPER_H

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <openxr/openxr.h>
#include <openxr/fb_scene_capture.h>
#include <godot_cpp/variant/utility_functions.hpp>

#include "../util.h"

#include <map>

using namespace godot;

// Wrapper for the set of Facebook XR scene capture extension.
class OpenXRFbSceneCaptureExtensionWrapper : public OpenXRExtensionWrapperExtension {
	GDCLASS(OpenXRFbSceneCaptureExtensionWrapper, OpenXRExtensionWrapperExtension);

public:
	godot::Dictionary _get_requested_extensions() override;

	void _on_instance_created(uint64_t instance) override;

	void _on_instance_destroyed() override;

	bool is_scene_capture_supported() {
		return fb_scene_capture_ext;
	}

	bool request_scene_capture();
	bool is_scene_capture_enabled();

	virtual bool _on_event_polled(const void *event) override;

	static OpenXRFbSceneCaptureExtensionWrapper *get_singleton();

	OpenXRFbSceneCaptureExtensionWrapper();
	~OpenXRFbSceneCaptureExtensionWrapper();

protected:
	static void _bind_methods();

private:
	EXT_PROTO_XRRESULT_FUNC3(xrRequestSceneCaptureFB,
			(XrSession), session,
			(const XrSceneCaptureRequestInfoFB *), request,
			(XrAsyncRequestIdFB *), requestId)

	bool initialize_fb_scene_capture_extension(const XrInstance instance);

	std::map<godot::String, bool *> request_extensions;

	void cleanup();

	static OpenXRFbSceneCaptureExtensionWrapper *singleton;

	bool fb_scene_capture_ext = false;

	bool scene_capture_enabled = false;
};

#endif // OPENXR_FB_SCENE_CAPTURE_EXTENSION_WRAPPER_H
