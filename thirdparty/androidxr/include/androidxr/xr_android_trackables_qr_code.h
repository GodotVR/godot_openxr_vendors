#ifndef XR_ANDROID_TRACKABLES_QR_CODE_H_
#define XR_ANDROID_TRACKABLES_QR_CODE_H_ 1

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


#ifndef XR_ANDROID_trackables_qr_code

// XR_ANDROID_trackables_qr_code is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_trackables_qr_code 1
#define XR_ANDROID_trackables_qr_code_SPEC_VERSION 1
#define XR_ANDROID_TRACKABLES_QR_CODE_EXTENSION_NAME "XR_ANDROID_trackables_qr_code"
#define XR_TYPE_SYSTEM_QR_CODE_TRACKING_PROPERTIES_ANDROID ((XrStructureType) 1000708000U)
#define XR_TYPE_TRACKABLE_QR_CODE_CONFIGURATION_ANDROID ((XrStructureType) 1000708001U)
#define XR_TYPE_TRACKABLE_QR_CODE_ANDROID ((XrStructureType) 1000708002U)
// Indicates that the trackable is a QR code.
#define XR_TRACKABLE_TYPE_QR_CODE_ANDROID ((XrTrackableTypeANDROID) 1000708000U)

typedef enum XrQrCodeTrackingModeANDROID {
    // Tracking dynamic QR codes. This mode has the highest accuracy and works on moving and static QR codes, but also has the highest power consumption.
    XR_QR_CODE_TRACKING_MODE_DYNAMIC_ANDROID = 0,
    // Tracking static QR codes. This mode is primarily useful for QR codes that are known to be static, which leads to less power consumption in comparison to the dynamic mode.
    XR_QR_CODE_TRACKING_MODE_STATIC_ANDROID = 1,
    XR_QR_CODE_TRACKING_MODE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrQrCodeTrackingModeANDROID;
typedef struct XrSystemQrCodeTrackingPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsQrCodeTracking;
    XrBool32              supportsQrCodeSizeEstimation;
    uint16_t              maxQrCodeCount;
} XrSystemQrCodeTrackingPropertiesANDROID;

// XrTrackableQrCodeConfigurationANDROID extends XrTrackableTrackerCreateInfoANDROID
typedef struct XrTrackableQrCodeConfigurationANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrQrCodeTrackingModeANDROID    trackingMode;
    float                          qrCodeEdgeSize;
} XrTrackableQrCodeConfigurationANDROID;

typedef struct XrTrackableQrCodeANDROID {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    XrTrackingStateANDROID    trackingState;
    XrTime                    lastUpdatedTime;
    XrPosef                   centerPose;
    XrExtent2Df               extents;
    uint32_t                  bufferCapacityInput;
    uint32_t                  bufferCountOutput;
    char*                     buffer;
} XrTrackableQrCodeANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrGetTrackableQrCodeANDROID)(XrTrackableTrackerANDROID tracker, const XrTrackableGetInfoANDROID* getInfo, XrTrackableQrCodeANDROID* qrCodeOutput);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrGetTrackableQrCodeANDROID(
    XrTrackableTrackerANDROID                   tracker,
    const XrTrackableGetInfoANDROID*            getInfo,
    XrTrackableQrCodeANDROID*                   qrCodeOutput);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_trackables_qr_code */

#ifdef __cplusplus
}
#endif

#endif
