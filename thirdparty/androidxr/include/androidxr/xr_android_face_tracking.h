#ifndef XR_ANDROID_FACE_TRACKING_H_
#define XR_ANDROID_FACE_TRACKING_H_ 1

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


#ifndef XR_ANDROID_face_tracking

// XR_ANDROID_face_tracking is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_face_tracking 1
XR_DEFINE_HANDLE(XrFaceTrackerANDROID)
#define XR_ANDROID_face_tracking_SPEC_VERSION 1
#define XR_ANDROID_FACE_TRACKING_EXTENSION_NAME "XR_ANDROID_face_tracking"
#define XR_TYPE_FACE_TRACKER_CREATE_INFO_ANDROID ((XrStructureType) 1000458000U)
#define XR_TYPE_FACE_STATE_GET_INFO_ANDROID ((XrStructureType) 1000458001U)
#define XR_TYPE_FACE_STATE_ANDROID        ((XrStructureType) 1000458002U)
#define XR_TYPE_SYSTEM_FACE_TRACKING_PROPERTIES_ANDROID ((XrStructureType) 1000458003U)
// The underlying tracking service is not yet ready.
#define XR_ERROR_SERVICE_NOT_READY_ANDROID ((XrResult) -1000458000U)
// XrFaceTrackerANDROID
#define XR_OBJECT_TYPE_FACE_TRACKER_ANDROID ((XrObjectType) 1000458000U)
#define XR_FACE_PARAMETER_COUNT_ANDROID   68
#define XR_FACE_REGION_CONFIDENCE_COUNT_ANDROID 3

typedef enum XrFaceParameterIndicesANDROID {
    // The left brow lowerer blendshape parameter.
    XR_FACE_PARAMETER_INDICES_BROW_LOWERER_L_ANDROID = 0,
    // The right brow lowerer blendshape parameter.
    XR_FACE_PARAMETER_INDICES_BROW_LOWERER_R_ANDROID = 1,
    // The left cheek puff blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHEEK_PUFF_L_ANDROID = 2,
    // The right cheek puff blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHEEK_PUFF_R_ANDROID = 3,
    // The left cheek raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHEEK_RAISER_L_ANDROID = 4,
    // The right cheek raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHEEK_RAISER_R_ANDROID = 5,
    // The left cheek suck blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHEEK_SUCK_L_ANDROID = 6,
    // The right cheek suck blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHEEK_SUCK_R_ANDROID = 7,
    // The bottom chin raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHIN_RAISER_B_ANDROID = 8,
    // The top chin raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_CHIN_RAISER_T_ANDROID = 9,
    // The left dimpler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_DIMPLER_L_ANDROID = 10,
    // The right dimpler lowerer blendshape parameter.
    XR_FACE_PARAMETER_INDICES_DIMPLER_R_ANDROID = 11,
    // The left eyes closed blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_CLOSED_L_ANDROID = 12,
    // The right eyes closed blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_CLOSED_R_ANDROID = 13,
    // The left eyes look down blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_DOWN_L_ANDROID = 14,
    // The right eyes look down blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_DOWN_R_ANDROID = 15,
    // The left look left blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_LEFT_L_ANDROID = 16,
    // The left look right blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_LEFT_R_ANDROID = 17,
    // The right look left blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_RIGHT_L_ANDROID = 18,
    // The right look right blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_RIGHT_R_ANDROID = 19,
    // The left eyes look up blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_UP_L_ANDROID = 20,
    // The right eyes look up blendshape parameter.
    XR_FACE_PARAMETER_INDICES_EYES_LOOK_UP_R_ANDROID = 21,
    // The left inner brow raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_INNER_BROW_RAISER_L_ANDROID = 22,
    // The right inner brow raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_INNER_BROW_RAISER_R_ANDROID = 23,
    // The jaw drop blendshape parameter.
    XR_FACE_PARAMETER_INDICES_JAW_DROP_ANDROID = 24,
    // The left jaw sideways blendshape parameter.
    XR_FACE_PARAMETER_INDICES_JAW_SIDEWAYS_LEFT_ANDROID = 25,
    // The right jaw sideways blendshape parameter.
    XR_FACE_PARAMETER_INDICES_JAW_SIDEWAYS_RIGHT_ANDROID = 26,
    // The jaw thrust blendshape parameter.
    XR_FACE_PARAMETER_INDICES_JAW_THRUST_ANDROID = 27,
    // The left lid tightener blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LID_TIGHTENER_L_ANDROID = 28,
    // The right lid tightener blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LID_TIGHTENER_R_ANDROID = 29,
    // The left corner lip depressor blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_CORNER_DEPRESSOR_L_ANDROID = 30,
    // The right corner lip depressor blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_CORNER_DEPRESSOR_R_ANDROID = 31,
    // The left corner lip puller blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_CORNER_PULLER_L_ANDROID = 32,
    // The right corner lip puller blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_CORNER_PULLER_R_ANDROID = 33,
    // The left bottom lip funneler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_LB_ANDROID = 34,
    // The left top lip funneler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_LT_ANDROID = 35,
    // The right bottom lip funneler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_RB_ANDROID = 36,
    // The right top lip funneler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_FUNNELER_RT_ANDROID = 37,
    // The left lip pressor blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_PRESSOR_L_ANDROID = 38,
    // The right lip pressor blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_PRESSOR_R_ANDROID = 39,
    // The left lip pucker blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_PUCKER_L_ANDROID = 40,
    // The right lip pucker blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_PUCKER_R_ANDROID = 41,
    // The left lip stretcher blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_STRETCHER_L_ANDROID = 42,
    // The right lip stretcher blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_STRETCHER_R_ANDROID = 43,
    // The left bottom lip suck blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_SUCK_LB_ANDROID = 44,
    // The left top lip suck blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_SUCK_LT_ANDROID = 45,
    // The right bottom lip suck blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_SUCK_RB_ANDROID = 46,
    // The right top lip suck blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_SUCK_RT_ANDROID = 47,
    // The left lip tightener blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_TIGHTENER_L_ANDROID = 48,
    // The right lip tightener blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIP_TIGHTENER_R_ANDROID = 49,
    // The lips toward blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LIPS_TOWARD_ANDROID = 50,
    // The left lower lip depressor blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LOWER_LIP_DEPRESSOR_L_ANDROID = 51,
    // The right lower lip depressor blendshape parameter.
    XR_FACE_PARAMETER_INDICES_LOWER_LIP_DEPRESSOR_R_ANDROID = 52,
    // The mouth move left blendshape parameter.
    XR_FACE_PARAMETER_INDICES_MOUTH_LEFT_ANDROID = 53,
    // The mouth move right blendshape parameter.
    XR_FACE_PARAMETER_INDICES_MOUTH_RIGHT_ANDROID = 54,
    // The left nose wrinkler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_NOSE_WRINKLER_L_ANDROID = 55,
    // The right nose wrinkler blendshape parameter.
    XR_FACE_PARAMETER_INDICES_NOSE_WRINKLER_R_ANDROID = 56,
    // The left outer brow raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_OUTER_BROW_RAISER_L_ANDROID = 57,
    // The right outer brow raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_OUTER_BROW_RAISER_R_ANDROID = 58,
    // The left lid raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_UPPER_LID_RAISER_L_ANDROID = 59,
    // The right lid raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_UPPER_LID_RAISER_R_ANDROID = 60,
    // The left lip raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_UPPER_LIP_RAISER_L_ANDROID = 61,
    // The right lip raiser blendshape parameter.
    XR_FACE_PARAMETER_INDICES_UPPER_LIP_RAISER_R_ANDROID = 62,
    // The tongue out blendshape parameter.
    XR_FACE_PARAMETER_INDICES_TONGUE_OUT_ANDROID = 63,
    // The tongue left puller blendshape parameter.
    XR_FACE_PARAMETER_INDICES_TONGUE_LEFT_ANDROID = 64,
    // The right right puller blendshape parameter.
    XR_FACE_PARAMETER_INDICES_TONGUE_RIGHT_ANDROID = 65,
    // The right up puller blendshape parameter.
    XR_FACE_PARAMETER_INDICES_TONGUE_UP_ANDROID = 66,
    // The right down puller blendshape parameter.
    XR_FACE_PARAMETER_INDICES_TONGUE_DOWN_ANDROID = 67,
    XR_FACE_PARAMETER_INDICES_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrFaceParameterIndicesANDROID;

typedef enum XrFaceTrackingStateANDROID {
    // Indicates that face tracking is paused but may be resumed in the future.
    XR_FACE_TRACKING_STATE_PAUSED_ANDROID = 0,
    // Tracking has stopped but the client still has an active face tracker.
    XR_FACE_TRACKING_STATE_STOPPED_ANDROID = 1,
    // The face is currently tracked and its pose is current.
    XR_FACE_TRACKING_STATE_TRACKING_ANDROID = 2,
    XR_FACE_TRACKING_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrFaceTrackingStateANDROID;

typedef enum XrFaceConfidenceRegionsANDROID {
    // Confidence corresponding to the lower region.
    XR_FACE_CONFIDENCE_REGIONS_LOWER_ANDROID = 0,
    // Confidence corresponding to the left upper region.
    XR_FACE_CONFIDENCE_REGIONS_LEFT_UPPER_ANDROID = 1,
    // Confidence corresponding to the right upper region.
    XR_FACE_CONFIDENCE_REGIONS_RIGHT_UPPER_ANDROID = 2,
    XR_FACE_CONFIDENCE_REGIONS_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrFaceConfidenceRegionsANDROID;
typedef struct XrFaceTrackerCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrFaceTrackerCreateInfoANDROID;

typedef struct XrFaceStateGetInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrTime                      time;
} XrFaceStateGetInfoANDROID;

typedef struct XrFaceStateANDROID {
    XrStructureType               type;
    void* XR_MAY_ALIAS            next;
    uint32_t                      parametersCapacityInput;
    uint32_t                      parametersCountOutput;
    float*                        parameters;
    XrFaceTrackingStateANDROID    faceTrackingState;
    XrTime                        sampleTime;
    XrBool32                      isValid;
    uint32_t                      regionConfidencesCapacityInput;
    uint32_t                      regionConfidencesCountOutput;
    float*                        regionConfidences;
} XrFaceStateANDROID;

typedef struct XrSystemFaceTrackingPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsFaceTracking;
} XrSystemFaceTrackingPropertiesANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreateFaceTrackerANDROID)(XrSession session, const XrFaceTrackerCreateInfoANDROID* createInfo, XrFaceTrackerANDROID* faceTracker);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyFaceTrackerANDROID)(XrFaceTrackerANDROID faceTracker);
typedef XrResult (XRAPI_PTR *PFN_xrGetFaceStateANDROID)(XrFaceTrackerANDROID faceTracker, const XrFaceStateGetInfoANDROID* getInfo, XrFaceStateANDROID* faceStateOutput);
typedef XrResult (XRAPI_PTR *PFN_xrGetFaceCalibrationStateANDROID)(XrFaceTrackerANDROID faceTracker, XrBool32* faceIsCalibratedOutput);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateFaceTrackerANDROID(
    XrSession                                   session,
    const XrFaceTrackerCreateInfoANDROID*       createInfo,
    XrFaceTrackerANDROID*                       faceTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyFaceTrackerANDROID(
    XrFaceTrackerANDROID                        faceTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrGetFaceStateANDROID(
    XrFaceTrackerANDROID                        faceTracker,
    const XrFaceStateGetInfoANDROID*            getInfo,
    XrFaceStateANDROID*                         faceStateOutput);

XRAPI_ATTR XrResult XRAPI_CALL xrGetFaceCalibrationStateANDROID(
    XrFaceTrackerANDROID                        faceTracker,
    XrBool32*                                   faceIsCalibratedOutput);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_face_tracking */

#ifdef __cplusplus
}
#endif

#endif
