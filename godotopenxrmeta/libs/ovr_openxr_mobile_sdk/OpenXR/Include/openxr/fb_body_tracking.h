#ifndef FB_BODY_TRACKING_H_
#define FB_BODY_TRACKING_H_ 1

/**********************
This file is @generated from the OpenXR XML API registry.
Language    :   C99
Copyright   :   (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
***********************/

#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_FB_body_tracking

#define XR_FB_body_tracking 1
XR_DEFINE_HANDLE(XrBodyTrackerFB)
#define XR_FB_body_tracking_SPEC_VERSION  1
#define XR_FB_BODY_TRACKING_EXTENSION_NAME "XR_FB_body_tracking"
// XrBodyTrackerFB
static const XrObjectType XR_OBJECT_TYPE_BODY_TRACKER_FB = (XrObjectType) 1000076000;
static const XrStructureType XR_TYPE_BODY_TRACKER_CREATE_INFO_FB = (XrStructureType) 1000076001;
static const XrStructureType XR_TYPE_BODY_JOINTS_LOCATE_INFO_FB = (XrStructureType) 1000076002;
static const XrStructureType XR_TYPE_SYSTEM_BODY_TRACKING_PROPERTIES_FB = (XrStructureType) 1000076004;
static const XrStructureType XR_TYPE_BODY_JOINT_LOCATIONS_FB = (XrStructureType) 1000076005;
static const XrStructureType XR_TYPE_BODY_SKELETON_FB = (XrStructureType) 1000076006;

typedef enum XrBodyJointFB {
    XR_BODY_JOINT_ROOT_FB = 0,
    XR_BODY_JOINT_HIPS_FB = 1,
    XR_BODY_JOINT_SPINE_LOWER_FB = 2,
    XR_BODY_JOINT_SPINE_MIDDLE_FB = 3,
    XR_BODY_JOINT_SPINE_UPPER_FB = 4,
    XR_BODY_JOINT_CHEST_FB = 5,
    XR_BODY_JOINT_NECK_FB = 6,
    XR_BODY_JOINT_HEAD_FB = 7,
    XR_BODY_JOINT_LEFT_SHOULDER_FB = 8,
    XR_BODY_JOINT_LEFT_SCAPULA_FB = 9,
    XR_BODY_JOINT_LEFT_ARM_UPPER_FB = 10,
    XR_BODY_JOINT_LEFT_ARM_LOWER_FB = 11,
    XR_BODY_JOINT_LEFT_HAND_WRIST_TWIST_FB = 12,
    XR_BODY_JOINT_RIGHT_SHOULDER_FB = 13,
    XR_BODY_JOINT_RIGHT_SCAPULA_FB = 14,
    XR_BODY_JOINT_RIGHT_ARM_UPPER_FB = 15,
    XR_BODY_JOINT_RIGHT_ARM_LOWER_FB = 16,
    XR_BODY_JOINT_RIGHT_HAND_WRIST_TWIST_FB = 17,
    XR_BODY_JOINT_LEFT_HAND_PALM_FB = 18,
    XR_BODY_JOINT_LEFT_HAND_WRIST_FB = 19,
    XR_BODY_JOINT_LEFT_HAND_THUMB_METACARPAL_FB = 20,
    XR_BODY_JOINT_LEFT_HAND_THUMB_PROXIMAL_FB = 21,
    XR_BODY_JOINT_LEFT_HAND_THUMB_DISTAL_FB = 22,
    XR_BODY_JOINT_LEFT_HAND_THUMB_TIP_FB = 23,
    XR_BODY_JOINT_LEFT_HAND_INDEX_METACARPAL_FB = 24,
    XR_BODY_JOINT_LEFT_HAND_INDEX_PROXIMAL_FB = 25,
    XR_BODY_JOINT_LEFT_HAND_INDEX_INTERMEDIATE_FB = 26,
    XR_BODY_JOINT_LEFT_HAND_INDEX_DISTAL_FB = 27,
    XR_BODY_JOINT_LEFT_HAND_INDEX_TIP_FB = 28,
    XR_BODY_JOINT_LEFT_HAND_MIDDLE_METACARPAL_FB = 29,
    XR_BODY_JOINT_LEFT_HAND_MIDDLE_PROXIMAL_FB = 30,
    XR_BODY_JOINT_LEFT_HAND_MIDDLE_INTERMEDIATE_FB = 31,
    XR_BODY_JOINT_LEFT_HAND_MIDDLE_DISTAL_FB = 32,
    XR_BODY_JOINT_LEFT_HAND_MIDDLE_TIP_FB = 33,
    XR_BODY_JOINT_LEFT_HAND_RING_METACARPAL_FB = 34,
    XR_BODY_JOINT_LEFT_HAND_RING_PROXIMAL_FB = 35,
    XR_BODY_JOINT_LEFT_HAND_RING_INTERMEDIATE_FB = 36,
    XR_BODY_JOINT_LEFT_HAND_RING_DISTAL_FB = 37,
    XR_BODY_JOINT_LEFT_HAND_RING_TIP_FB = 38,
    XR_BODY_JOINT_LEFT_HAND_LITTLE_METACARPAL_FB = 39,
    XR_BODY_JOINT_LEFT_HAND_LITTLE_PROXIMAL_FB = 40,
    XR_BODY_JOINT_LEFT_HAND_LITTLE_INTERMEDIATE_FB = 41,
    XR_BODY_JOINT_LEFT_HAND_LITTLE_DISTAL_FB = 42,
    XR_BODY_JOINT_LEFT_HAND_LITTLE_TIP_FB = 43,
    XR_BODY_JOINT_RIGHT_HAND_PALM_FB = 44,
    XR_BODY_JOINT_RIGHT_HAND_WRIST_FB = 45,
    XR_BODY_JOINT_RIGHT_HAND_THUMB_METACARPAL_FB = 46,
    XR_BODY_JOINT_RIGHT_HAND_THUMB_PROXIMAL_FB = 47,
    XR_BODY_JOINT_RIGHT_HAND_THUMB_DISTAL_FB = 48,
    XR_BODY_JOINT_RIGHT_HAND_THUMB_TIP_FB = 49,
    XR_BODY_JOINT_RIGHT_HAND_INDEX_METACARPAL_FB = 50,
    XR_BODY_JOINT_RIGHT_HAND_INDEX_PROXIMAL_FB = 51,
    XR_BODY_JOINT_RIGHT_HAND_INDEX_INTERMEDIATE_FB = 52,
    XR_BODY_JOINT_RIGHT_HAND_INDEX_DISTAL_FB = 53,
    XR_BODY_JOINT_RIGHT_HAND_INDEX_TIP_FB = 54,
    XR_BODY_JOINT_RIGHT_HAND_MIDDLE_METACARPAL_FB = 55,
    XR_BODY_JOINT_RIGHT_HAND_MIDDLE_PROXIMAL_FB = 56,
    XR_BODY_JOINT_RIGHT_HAND_MIDDLE_INTERMEDIATE_FB = 57,
    XR_BODY_JOINT_RIGHT_HAND_MIDDLE_DISTAL_FB = 58,
    XR_BODY_JOINT_RIGHT_HAND_MIDDLE_TIP_FB = 59,
    XR_BODY_JOINT_RIGHT_HAND_RING_METACARPAL_FB = 60,
    XR_BODY_JOINT_RIGHT_HAND_RING_PROXIMAL_FB = 61,
    XR_BODY_JOINT_RIGHT_HAND_RING_INTERMEDIATE_FB = 62,
    XR_BODY_JOINT_RIGHT_HAND_RING_DISTAL_FB = 63,
    XR_BODY_JOINT_RIGHT_HAND_RING_TIP_FB = 64,
    XR_BODY_JOINT_RIGHT_HAND_LITTLE_METACARPAL_FB = 65,
    XR_BODY_JOINT_RIGHT_HAND_LITTLE_PROXIMAL_FB = 66,
    XR_BODY_JOINT_RIGHT_HAND_LITTLE_INTERMEDIATE_FB = 67,
    XR_BODY_JOINT_RIGHT_HAND_LITTLE_DISTAL_FB = 68,
    XR_BODY_JOINT_RIGHT_HAND_LITTLE_TIP_FB = 69,
    XR_BODY_JOINT_COUNT_FB = 70,
    XR_BODY_JOINT_NONE_FB = -1,
    XR_BODY_JOINT_MAX_ENUM_FB = 0x7FFFFFFF
} XrBodyJointFB;

// Describes the set of body joints to track when creating an slink:XrBodyTrackerFB.
typedef enum XrBodyJointSetFB {
    // Indicates that the created slink:XrBodyTrackerFB tracks the set of body joints described by elink:XrBodyJointFB enum, i.e. the flink:xrLocateBodyJointsFB function returns an array of joint locations with the count of ename:XR_BODY_JOINT_COUNT_FB and can be indexed using elink:XrBodyJointFB.
    XR_BODY_JOINT_SET_DEFAULT_FB = 0,
    XR_BODY_JOINT_SET_MAX_ENUM_FB = 0x7FFFFFFF
} XrBodyJointSetFB;
typedef struct XrBodyJointLocationFB {
    XrSpaceLocationFlags    locationFlags;
    XrPosef                 pose;
} XrBodyJointLocationFB;

typedef struct XrSystemBodyTrackingPropertiesFB {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsBodyTracking;
} XrSystemBodyTrackingPropertiesFB;

typedef struct XrBodyTrackerCreateInfoFB {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrBodyJointSetFB            bodyJointSet;
} XrBodyTrackerCreateInfoFB;

typedef struct XrBodySkeletonJointFB {
    int32_t    joint;
    int32_t    parentJoint;
    XrPosef    pose;
} XrBodySkeletonJointFB;

typedef struct XrBodySkeletonFB {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    uint32_t                  jointCount;
    XrBodySkeletonJointFB*    joints;
} XrBodySkeletonFB;

typedef struct XrBodyJointsLocateInfoFB {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     baseSpace;
    XrTime                      time;
} XrBodyJointsLocateInfoFB;

typedef struct XrBodyJointLocationsFB {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    XrBool32                  isActive;
    float                     confidence;
    uint32_t                  jointCount;
    XrBodyJointLocationFB*    jointLocations;
    uint32_t                  skeletonChangedCount;
    XrTime                    time;
} XrBodyJointLocationsFB;

typedef XrResult (XRAPI_PTR *PFN_xrCreateBodyTrackerFB)(XrSession session, const XrBodyTrackerCreateInfoFB* createInfo, XrBodyTrackerFB* bodyTracker);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyBodyTrackerFB)(XrBodyTrackerFB bodyTracker);
typedef XrResult (XRAPI_PTR *PFN_xrLocateBodyJointsFB)(XrBodyTrackerFB bodyTracker, const XrBodyJointsLocateInfoFB* locateInfo, XrBodyJointLocationsFB* locations);
typedef XrResult (XRAPI_PTR *PFN_xrGetBodySkeletonFB)(XrBodyTrackerFB bodyTracker, XrBodySkeletonFB* skeleton);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateBodyTrackerFB(
    XrSession                                   session,
    const XrBodyTrackerCreateInfoFB*            createInfo,
    XrBodyTrackerFB*                            bodyTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyBodyTrackerFB(
    XrBodyTrackerFB                             bodyTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrLocateBodyJointsFB(
    XrBodyTrackerFB                             bodyTracker,
    const XrBodyJointsLocateInfoFB*             locateInfo,
    XrBodyJointLocationsFB*                     locations);

XRAPI_ATTR XrResult XRAPI_CALL xrGetBodySkeletonFB(
    XrBodyTrackerFB                             bodyTracker,
    XrBodySkeletonFB*                           skeleton);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_FB_body_tracking */

#ifdef __cplusplus
}
#endif

#endif
