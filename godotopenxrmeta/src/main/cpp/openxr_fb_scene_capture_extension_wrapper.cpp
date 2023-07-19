#include "include/openxr_fb_scene_capture_extension_wrapper.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/image.hpp>

using namespace godot;

OpenXRFbSceneCaptureExtensionWrapper *OpenXRFbSceneCaptureExtensionWrapper::singleton = nullptr;

OpenXRFbSceneCaptureExtensionWrapper *OpenXRFbSceneCaptureExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSceneCaptureExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSceneCaptureExtensionWrapper::OpenXRFbSceneCaptureExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {

	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSceneCaptureExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SCENE_CAPTURE_EXTENSION_NAME] = &fb_scene_capture_ext;
	singleton = this;
}

OpenXRFbSceneCaptureExtensionWrapper::~OpenXRFbSceneCaptureExtensionWrapper() {
	cleanup();
}

void OpenXRFbSceneCaptureExtensionWrapper::_bind_methods() {

	ClassDB::bind_static_method("OpenXRFbSceneCaptureExtensionWrapper", D_METHOD("get_singleton"), &OpenXRFbSceneCaptureExtensionWrapper::get_singleton);
	ClassDB::bind_method(D_METHOD("is_scene_capture_supported"), &OpenXRFbSceneCaptureExtensionWrapper::is_scene_capture_supported);
	ClassDB::bind_method(D_METHOD("is_scene_capture_enabled"), &OpenXRFbSceneCaptureExtensionWrapper::is_scene_capture_enabled);
	ClassDB::bind_method(D_METHOD("request_scene_capture"), &OpenXRFbSceneCaptureExtensionWrapper::request_scene_capture);

	ADD_SIGNAL(MethodInfo("scene_capture_completed"));

}

void OpenXRFbSceneCaptureExtensionWrapper::cleanup() {
	fb_scene_capture_ext = false;
}

godot::Dictionary OpenXRFbSceneCaptureExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext: request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbSceneCaptureExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_scene_capture_ext) {
		bool result = initialize_fb_scene_capture_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_scene_capture extension");
			fb_scene_capture_ext = false;
		}
	}
}

void OpenXRFbSceneCaptureExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSceneCaptureExtensionWrapper::request_scene_capture() {
	XrAsyncRequestIdFB requestId;
	XrSceneCaptureRequestInfoFB request;
	request.type = XR_TYPE_SCENE_CAPTURE_REQUEST_INFO_FB;
	request.next = nullptr;
	request.requestByteCount = 0;
	request.request = nullptr;
	XrResult result = xrRequestSceneCaptureFB((XrSession)get_openxr_api()->get_session(), &request, &requestId);
	scene_capture_enabled = (result == XR_SUCCESS);
	return scene_capture_enabled;
}

bool OpenXRFbSceneCaptureExtensionWrapper::is_scene_capture_enabled() {
	return scene_capture_enabled;
}

bool OpenXRFbSceneCaptureExtensionWrapper::initialize_fb_scene_capture_extension(const XrInstance p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrRequestSceneCaptureFB);

	return true;
}

bool OpenXRFbSceneCaptureExtensionWrapper::_on_event_polled(const void *event) {

	if (static_cast<const XrEventDataBuffer*>(event)->type == XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB) {
		scene_capture_enabled = false;
		Object::emit_signal("scene_capture_completed");
		return true;
	}
	return false;
}
