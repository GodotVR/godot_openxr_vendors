#ifndef XR_ANDROID_PASSTHROUGH_CAMERA_STATE_H_
#define XR_ANDROID_PASSTHROUGH_CAMERA_STATE_H_ 1

/*
** Copyright 2017-2025 The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0 OR MIT
*/

/*
** This header is generated from the Khronos OpenXR XML API Registry.
**
*/


#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_ANDROID_passthrough_camera_state

// XR_ANDROID_passthrough_camera_state is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_passthrough_camera_state 1
#define XR_ANDROID_passthrough_camera_state_SPEC_VERSION 1
#define XR_ANDROID_PASSTHROUGH_CAMERA_STATE_EXTENSION_NAME "XR_ANDROID_passthrough_camera_state"
#define XR_TYPE_PASSTHROUGH_CAMERA_STATE_GET_INFO_ANDROID ((XrStructureType) 1000460000U)
#define XR_TYPE_SYSTEM_PASSTHROUGH_CAMERA_STATE_PROPERTIES_ANDROID ((XrStructureType) 1000460001U)

typedef enum XrPassthroughCameraStateANDROID {
    // The camera has been disabled by an app, the system or the user.
    XR_PASSTHROUGH_CAMERA_STATE_DISABLED_ANDROID = 0,
    // The camera is still coming online and not yet ready to use.
    XR_PASSTHROUGH_CAMERA_STATE_INITIALIZING_ANDROID = 1,
    // The camera is ready to use.
    XR_PASSTHROUGH_CAMERA_STATE_READY_ANDROID = 2,
    // The camera is in an unrecoverable error state.
    XR_PASSTHROUGH_CAMERA_STATE_ERROR_ANDROID = 3,
    XR_PASSTHROUGH_CAMERA_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrPassthroughCameraStateANDROID;
typedef struct XrSystemPassthroughCameraStatePropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsPassthroughCameraState;
} XrSystemPassthroughCameraStatePropertiesANDROID;

typedef struct XrPassthroughCameraStateGetInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrPassthroughCameraStateGetInfoANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrGetPassthroughCameraStateANDROID)(XrSession session, const XrPassthroughCameraStateGetInfoANDROID* getInfo, XrPassthroughCameraStateANDROID* cameraStateOutput);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrGetPassthroughCameraStateANDROID(
    XrSession                                   session,
    const XrPassthroughCameraStateGetInfoANDROID* getInfo,
    XrPassthroughCameraStateANDROID*            cameraStateOutput);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_passthrough_camera_state */

#ifdef __cplusplus
}
#endif

#endif
