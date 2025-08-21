#ifndef XR_ANDROID_TRACKABLES_OBJECT_H_
#define XR_ANDROID_TRACKABLES_OBJECT_H_ 1

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


#ifndef XR_ANDROID_trackables_object

// XR_ANDROID_trackables_object is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_trackables_object 1
#define XR_ANDROID_trackables_object_SPEC_VERSION 1
#define XR_ANDROID_TRACKABLES_OBJECT_EXTENSION_NAME "XR_ANDROID_trackables_object"
#define XR_TYPE_TRACKABLE_OBJECT_ANDROID  ((XrStructureType) 1000466000U)
#define XR_TYPE_TRACKABLE_OBJECT_CONFIGURATION_ANDROID ((XrStructureType) 1000466001U)
// Indicates that the trackable is the object.
#define XR_TRACKABLE_TYPE_OBJECT_ANDROID  ((XrTrackableTypeANDROID) 1000466000U)

typedef enum XrObjectLabelANDROID {
    // It was not possible to label the object.
    XR_OBJECT_LABEL_UNKNOWN_ANDROID = 0,
    // The object is a keyboard.
    XR_OBJECT_LABEL_KEYBOARD_ANDROID = 1,
    // The object is a mouse.
    XR_OBJECT_LABEL_MOUSE_ANDROID = 2,
    // The object is a laptop.
    XR_OBJECT_LABEL_LAPTOP_ANDROID = 3,
    XR_OBJECT_LABEL_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrObjectLabelANDROID;
typedef struct XrTrackableObjectANDROID {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    XrTrackingStateANDROID    trackingState;
    XrPosef                   centerPose;
    XrExtent3DfEXT            extents;
    XrObjectLabelANDROID      objectLabel;
    XrTime                    lastUpdatedTime;
} XrTrackableObjectANDROID;

// XrTrackableObjectConfigurationANDROID extends XrTrackableTrackerCreateInfoANDROID
typedef struct XrTrackableObjectConfigurationANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    uint32_t                       labelCount;
    const XrObjectLabelANDROID*    activeLabels;
} XrTrackableObjectConfigurationANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrGetTrackableObjectANDROID)(XrTrackableTrackerANDROID tracker, const XrTrackableGetInfoANDROID* getInfo, XrTrackableObjectANDROID* objectOutput);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrGetTrackableObjectANDROID(
    XrTrackableTrackerANDROID                   tracker,
    const XrTrackableGetInfoANDROID*            getInfo,
    XrTrackableObjectANDROID*                   objectOutput);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_trackables_object */

#ifdef __cplusplus
}
#endif

#endif
