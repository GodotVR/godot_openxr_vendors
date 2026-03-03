#ifndef XR_ANDROID_TRACKABLES_IMAGE_H_
#define XR_ANDROID_TRACKABLES_IMAGE_H_ 1

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


#ifndef XR_ANDROID_trackables_image

// XR_ANDROID_trackables_image is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_trackables_image 1
XR_DEFINE_HANDLE(XrTrackableImageDatabaseANDROID)
#define XR_ANDROID_trackables_image_SPEC_VERSION 1
#define XR_ANDROID_TRACKABLES_IMAGE_EXTENSION_NAME "XR_ANDROID_trackables_image"
#define XR_TYPE_SYSTEM_IMAGE_TRACKING_PROPERTIES_ANDROID ((XrStructureType) 1000709000U)
#define XR_TYPE_TRACKABLE_IMAGE_DATABASE_ENTRY_ANDROID ((XrStructureType) 1000709001U)
#define XR_TYPE_TRACKABLE_IMAGE_DATABASE_CREATE_INFO_ANDROID ((XrStructureType) 1000709002U)
#define XR_TYPE_CREATE_TRACKABLE_IMAGE_DATABASE_COMPLETION_ANDROID ((XrStructureType) 1000709003U)
#define XR_TYPE_TRACKABLE_IMAGE_CONFIGURATION_ANDROID ((XrStructureType) 1000709004U)
#define XR_TYPE_TRACKABLE_IMAGE_ANDROID   ((XrStructureType) 1000709005U)
#define XR_TYPE_EVENT_DATA_IMAGE_TRACKING_LOST_ANDROID ((XrStructureType) 1000709006U)
// XrTrackableImageDatabaseANDROID
#define XR_OBJECT_TYPE_TRACKABLE_IMAGE_DATABASE_ANDROID ((XrObjectType) 1000709000U)
// Indicates that the input reference image format is not supported.
#define XR_ERROR_IMAGE_FORMAT_UNSUPPORTED_ANDROID ((XrResult) -1000709000U)
// Indicates that the trackable is an image.
#define XR_TRACKABLE_TYPE_IMAGE_ANDROID   ((XrTrackableTypeANDROID) 1000709000U)

typedef enum XrTrackableImageTrackingModeANDROID {
    // This mode has the highest accuracy and allows low latency tracking of moving images. It has also has the highest power consumption.
    XR_TRACKABLE_IMAGE_TRACKING_MODE_DYNAMIC_ANDROID = 1,
    // This mode should be used for images that are known to be static or semi-static. This mode leads to less power consumption in comparison to the dynamic mode. If a static image is being moved, it will be updated with a much higher latency than using the dynamic mode.
    XR_TRACKABLE_IMAGE_TRACKING_MODE_STATIC_ANDROID = 2,
    XR_TRACKABLE_IMAGE_TRACKING_MODE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrTrackableImageTrackingModeANDROID;

typedef enum XrTrackableImageFormatANDROID {
    // RGBA image format with 8 bits per channel color and transparency data.
    XR_TRACKABLE_IMAGE_FORMAT_R8G8B8A8_ANDROID = 1,
    XR_TRACKABLE_IMAGE_FORMAT_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrTrackableImageFormatANDROID;
typedef struct XrSystemImageTrackingPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsImageTracking;
    XrBool32              supportsPhysicalSizeEstimation;
    uint32_t              maxTrackedImageCount;
    uint32_t              maxLoadedImageCount;
} XrSystemImageTrackingPropertiesANDROID;

typedef struct XrTrackableImageDatabaseEntryANDROID {
    XrStructureType                        type;
    const void* XR_MAY_ALIAS               next;
    XrTrackableImageTrackingModeANDROID    trackingMode;
    float                                  physicalWidth;
    uint32_t                               imageWidth;
    uint32_t                               imageHeight;
    XrTrackableImageFormatANDROID          format;
    uint32_t                               bufferSize;
    const uint8_t*                         buffer;
} XrTrackableImageDatabaseEntryANDROID;

typedef struct XrTrackableImageDatabaseCreateInfoANDROID {
    XrStructureType                                type;
    const void* XR_MAY_ALIAS                       next;
    uint32_t                                       entryCount;
    const XrTrackableImageDatabaseEntryANDROID*    entries;
} XrTrackableImageDatabaseCreateInfoANDROID;

typedef struct XrCreateTrackableImageDatabaseCompletionANDROID {
    XrStructureType                    type;
    void* XR_MAY_ALIAS                 next;
    XrResult                           futureResult;
    XrTrackableImageDatabaseANDROID    database;
} XrCreateTrackableImageDatabaseCompletionANDROID;

typedef struct XrTrackableImageConfigurationANDROID {
    XrStructureType                           type;
    const void* XR_MAY_ALIAS                  next;
    uint32_t                                  databaseCount;
    const XrTrackableImageDatabaseANDROID*    databases;
} XrTrackableImageConfigurationANDROID;

typedef struct XrTrackableImageANDROID {
    XrStructureType                    type;
    const void* XR_MAY_ALIAS           next;
    XrTrackingStateANDROID             trackingState;
    XrTime                             lastUpdatedTime;
    XrTrackableImageDatabaseANDROID    database;
    uint32_t                           databaseEntryIndex;
    XrPosef                            centerPose;
    XrExtent2Df                        extents;
} XrTrackableImageANDROID;

typedef struct XrEventDataImageTrackingLostANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrTime                      time;
} XrEventDataImageTrackingLostANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreateTrackableImageDatabaseAsyncANDROID)(XrSession session, const XrTrackableImageDatabaseCreateInfoANDROID* createInfo, XrFutureEXT* future);
typedef XrResult (XRAPI_PTR *PFN_xrCreateTrackableImageDatabaseCompleteANDROID)(XrSession session, XrFutureEXT future, XrCreateTrackableImageDatabaseCompletionANDROID* completion);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyTrackableImageDatabaseANDROID)(XrTrackableImageDatabaseANDROID database);
typedef XrResult (XRAPI_PTR *PFN_xrAddTrackableImageDatabaseANDROID)(XrTrackableTrackerANDROID tracker, XrTrackableImageDatabaseANDROID database);
typedef XrResult (XRAPI_PTR *PFN_xrRemoveTrackableImageDatabaseANDROID)(XrTrackableTrackerANDROID tracker, XrTrackableImageDatabaseANDROID database);
typedef XrResult (XRAPI_PTR *PFN_xrGetTrackableImageANDROID)(XrTrackableTrackerANDROID tracker, const XrTrackableGetInfoANDROID* getInfo, XrTrackableImageANDROID* trackable);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateTrackableImageDatabaseAsyncANDROID(
    XrSession                                   session,
    const XrTrackableImageDatabaseCreateInfoANDROID* createInfo,
    XrFutureEXT*                                future);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateTrackableImageDatabaseCompleteANDROID(
    XrSession                                   session,
    XrFutureEXT                                 future,
    XrCreateTrackableImageDatabaseCompletionANDROID* completion);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyTrackableImageDatabaseANDROID(
    XrTrackableImageDatabaseANDROID             database);

XRAPI_ATTR XrResult XRAPI_CALL xrAddTrackableImageDatabaseANDROID(
    XrTrackableTrackerANDROID                   tracker,
    XrTrackableImageDatabaseANDROID             database);

XRAPI_ATTR XrResult XRAPI_CALL xrRemoveTrackableImageDatabaseANDROID(
    XrTrackableTrackerANDROID                   tracker,
    XrTrackableImageDatabaseANDROID             database);

XRAPI_ATTR XrResult XRAPI_CALL xrGetTrackableImageANDROID(
    XrTrackableTrackerANDROID                   tracker,
    const XrTrackableGetInfoANDROID*            getInfo,
    XrTrackableImageANDROID*                    trackable);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_trackables_image */

#ifdef __cplusplus
}
#endif

#endif
