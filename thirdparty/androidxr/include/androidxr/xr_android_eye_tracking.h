#ifndef XR_ANDROID_EYE_TRACKING_H_
#define XR_ANDROID_EYE_TRACKING_H_ 1

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


#ifndef XR_ANDROID_eye_tracking

// XR_ANDROID_eye_tracking is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_eye_tracking 1
XR_DEFINE_HANDLE(XrEyeTrackerANDROID)
#define XR_EYE_MAX_ANDROID                2
#define XR_ANDROID_eye_tracking_SPEC_VERSION 1
#define XR_ANDROID_EYE_TRACKING_EXTENSION_NAME "XR_ANDROID_eye_tracking"
#define XR_TYPE_EYES_ANDROID              ((XrStructureType) 1000456000U)
#define XR_TYPE_EYE_TRACKER_CREATE_INFO_ANDROID ((XrStructureType) 1000456001U)
#define XR_TYPE_EYES_GET_INFO_ANDROID     ((XrStructureType) 1000456002U)
#define XR_TYPE_SYSTEM_EYE_TRACKING_PROPERTIES_ANDROID ((XrStructureType) 1000456004U)
// XrEyeTrackerANDROID
#define XR_OBJECT_TYPE_EYE_TRACKER_ANDROID ((XrObjectType) 1000456000U)

typedef enum XrEyeIndexANDROID {
    // Left eye.
    XR_EYE_INDEX_LEFT_ANDROID = 0,
    // Right eye.
    XR_EYE_INDEX_RIGHT_ANDROID = 1,
    XR_EYE_INDEX_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrEyeIndexANDROID;

typedef enum XrEyeStateANDROID {
    // Indicates that the eye is in an error state or not present.
    XR_EYE_STATE_INVALID_ANDROID = 0,
    // Indicates that the eye is currently gazing.
    XR_EYE_STATE_GAZING_ANDROID = 1,
    // Indicates that the eye is currently shut due to a wink or a blink.
    XR_EYE_STATE_SHUT_ANDROID = 2,
    XR_EYE_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrEyeStateANDROID;

typedef enum XrEyeTrackingModeANDROID {
    // Indicates that eye tracking is not currently active.
    XR_EYE_TRACKING_MODE_NOT_TRACKING_ANDROID = 0,
    // Indicates that only the right eye is tracking.
    XR_EYE_TRACKING_MODE_RIGHT_ANDROID = 1,
    // Indicates that only the left eye is tracking.
    XR_EYE_TRACKING_MODE_LEFT_ANDROID = 2,
    // Indicates that both the left and right eyes are tracking.
    XR_EYE_TRACKING_MODE_BOTH_ANDROID = 3,
    XR_EYE_TRACKING_MODE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrEyeTrackingModeANDROID;
typedef struct XrSystemEyeTrackingPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsEyeTracking;
} XrSystemEyeTrackingPropertiesANDROID;

typedef struct XrEyeANDROID {
    XrEyeStateANDROID    eyeState;
    XrPosef              eyePose;
} XrEyeANDROID;

typedef struct XrEyesANDROID {
    XrStructureType             type;
    void* XR_MAY_ALIAS          next;
    XrEyeANDROID                eyes[XR_EYE_MAX_ANDROID];
    XrEyeTrackingModeANDROID    mode;
} XrEyesANDROID;

typedef struct XrEyesGetInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrTime                      time;
    XrSpace                     baseSpace;
} XrEyesGetInfoANDROID;

typedef struct XrEyeTrackerCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrEyeTrackerCreateInfoANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreateEyeTrackerANDROID)(XrSession session, const XrEyeTrackerCreateInfoANDROID* createInfo, XrEyeTrackerANDROID* eyeTracker);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyEyeTrackerANDROID)(XrEyeTrackerANDROID eyeTracker);
typedef XrResult (XRAPI_PTR *PFN_xrGetFineTrackingEyesInfoANDROID)(XrEyeTrackerANDROID eyeTracker, const XrEyesGetInfoANDROID* getInfo, XrEyesANDROID* eyesOutput);
typedef XrResult (XRAPI_PTR *PFN_xrGetCoarseTrackingEyesInfoANDROID)(XrEyeTrackerANDROID eyeTracker, const XrEyesGetInfoANDROID* getInfo, XrEyesANDROID* eyesOutput);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateEyeTrackerANDROID(
    XrSession                                   session,
    const XrEyeTrackerCreateInfoANDROID*        createInfo,
    XrEyeTrackerANDROID*                        eyeTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyEyeTrackerANDROID(
    XrEyeTrackerANDROID                         eyeTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrGetFineTrackingEyesInfoANDROID(
    XrEyeTrackerANDROID                         eyeTracker,
    const XrEyesGetInfoANDROID*                 getInfo,
    XrEyesANDROID*                              eyesOutput);

XRAPI_ATTR XrResult XRAPI_CALL xrGetCoarseTrackingEyesInfoANDROID(
    XrEyeTrackerANDROID                         eyeTracker,
    const XrEyesGetInfoANDROID*                 getInfo,
    XrEyesANDROID*                              eyesOutput);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_eye_tracking */

#ifdef __cplusplus
}
#endif

#endif
