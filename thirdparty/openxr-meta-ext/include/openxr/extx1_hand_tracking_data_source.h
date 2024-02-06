#ifndef EXTX1_HAND_TRACKING_DATA_SOURCE_H_
#define EXTX1_HAND_TRACKING_DATA_SOURCE_H_ 1

/**********************
This file is @generated from the OpenXR XML API registry.
Language    :   C99
Copyright   :   (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
***********************/

#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_EXTX1_hand_tracking_data_source

#define XR_EXTX1_hand_tracking_data_source 1
#define XR_EXTX1_hand_tracking_data_source_SPEC_VERSION 1
#define XR_EXTX1_HAND_TRACKING_DATA_SOURCE_EXTENSION_NAME "XR_EXTX1_hand_tracking_data_source"
static const XrStructureType XR_TYPE_HAND_TRACKING_DATA_SOURCE_INFO_EXTX1 = (XrStructureType) 1000294000;
static const XrStructureType XR_TYPE_HAND_TRACKING_DATA_SOURCE_STATE_EXTX1 = (XrStructureType) 1000294001;

typedef enum XrHandTrackingDataSourceEXTX1 {
    // This data source value indicates individual fingers and joints are tracked from unobstructed data source such as optical hand tracking, digital gloves or motion capture devices.
    XR_HAND_TRACKING_DATA_SOURCE_UNOBSTRUCTED_EXTX1 = 1,
    // This data source value indicates hand joints are inferred based on motion controller state.
    XR_HAND_TRACKING_DATA_SOURCE_CONTROLLER_EXTX1 = 2,
    XR_HAND_TRACKING_DATA_SOURCE_EXTX1_MAX_ENUM = 0x7FFFFFFF
} XrHandTrackingDataSourceEXTX1;
// XrHandTrackingDataSourceInfoEXTX1 extends XrHandTrackerCreateInfoEXT
typedef struct XrHandTrackingDataSourceInfoEXTX1 {
    XrStructureType                   type;
    const void* XR_MAY_ALIAS          next;
    uint32_t                          requestedDataSourceCount;
    XrHandTrackingDataSourceEXTX1*    requestedDataSources;
} XrHandTrackingDataSourceInfoEXTX1;

// XrHandTrackingDataSourceStateEXTX1 extends XrHandJointLocationsEXT
typedef struct XrHandTrackingDataSourceStateEXTX1 {
    XrStructureType                  type;
    void* XR_MAY_ALIAS               next;
    XrBool32                         isActive;
    XrHandTrackingDataSourceEXTX1    dataSource;
} XrHandTrackingDataSourceStateEXTX1;

#endif /* XR_EXTX1_hand_tracking_data_source */

#ifdef  XR_EXTX1_HAND_TRACKING_DATA_SOURCE_TAG_ALIAS
typedef XrHandTrackingDataSourceEXTX1 XrHandTrackingDataSourceEXT;
#define XR_HAND_TRACKING_DATA_SOURCE_UNOBSTRUCTED_EXT XR_HAND_TRACKING_DATA_SOURCE_UNOBSTRUCTED_EXTX1
#define XR_HAND_TRACKING_DATA_SOURCE_CONTROLLER_EXT XR_HAND_TRACKING_DATA_SOURCE_CONTROLLER_EXTX1
typedef XrHandTrackingDataSourceInfoEXTX1 XrHandTrackingDataSourceInfoEXT;
typedef XrHandTrackingDataSourceStateEXTX1 XrHandTrackingDataSourceStateEXT;
#define XR_EXT_hand_tracking_data_source_SPEC_VERSION XR_EXTX1_hand_tracking_data_source_SPEC_VERSION
#define XR_EXT_HAND_TRACKING_DATA_SOURCE_EXTENSION_NAME XR_EXTX1_HAND_TRACKING_DATA_SOURCE_EXTENSION_NAME
#define XR_TYPE_HAND_TRACKING_DATA_SOURCE_INFO_EXT XR_TYPE_HAND_TRACKING_DATA_SOURCE_INFO_EXTX1
#define XR_TYPE_HAND_TRACKING_DATA_SOURCE_STATE_EXT XR_TYPE_HAND_TRACKING_DATA_SOURCE_STATE_EXTX1
#endif /* XR_EXTX1_HAND_TRACKING_DATA_SOURCE_TAG_ALIAS */


#ifdef __cplusplus
}
#endif

#endif
