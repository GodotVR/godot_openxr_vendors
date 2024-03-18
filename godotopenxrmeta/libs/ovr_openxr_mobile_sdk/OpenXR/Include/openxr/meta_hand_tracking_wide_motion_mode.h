#ifndef META_HAND_TRACKING_WIDE_MOTION_MODE_H_
#define META_HAND_TRACKING_WIDE_MOTION_MODE_H_ 1

/**********************
This file is @generated from the OpenXR XML API registry.
Language    :   C99
Copyright   :   (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
***********************/

#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_META_hand_tracking_wide_motion_mode

#define XR_META_hand_tracking_wide_motion_mode 1
#define XR_META_hand_tracking_wide_motion_mode_SPEC_VERSION 1
#define XR_META_HAND_TRACKING_WIDE_MOTION_MODE_EXTENSION_NAME "XR_META_hand_tracking_wide_motion_mode"
static const XrStructureType XR_TYPE_HAND_TRACKING_WIDE_MOTION_MODE_INFO_META = (XrStructureType) 1000539000;

typedef enum XrHandTrackingWideMotionModeMETA {
    // The runtime will attempt to leverage high fidelity body tracking algorithms to calculate hand pose infomation outside of the range of usual tracking area.
    XR_HAND_TRACKING_WIDE_MOTION_MODE_HIGH_FIDELITY_BODY_TRACKING_META = 1,
    XR_HAND_TRACKING_WIDE_MOTION_MODE_MAX_ENUM_META = 0x7FFFFFFF
} XrHandTrackingWideMotionModeMETA;
// XrHandTrackingWideMotionModeInfoMETA extends XrHandTrackerCreateInfoEXT
typedef struct XrHandTrackingWideMotionModeInfoMETA {
    XrStructureType                     type;
    const void* XR_MAY_ALIAS            next;
    XrHandTrackingWideMotionModeMETA    requestedWideMotionMode;
} XrHandTrackingWideMotionModeInfoMETA;

#endif /* XR_META_hand_tracking_wide_motion_mode */

#ifdef __cplusplus
}
#endif

#endif
