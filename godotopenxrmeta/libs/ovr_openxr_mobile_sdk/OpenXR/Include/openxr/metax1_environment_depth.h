#ifndef METAX1_ENVIRONMENT_DEPTH_H_
#define METAX1_ENVIRONMENT_DEPTH_H_ 1

/**********************
This file is @generated from the OpenXR XML API registry.
Language    :   C99
Copyright   :   (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
***********************/

#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_METAX1_environment_depth

#define XR_METAX1_environment_depth 1
XR_DEFINE_HANDLE(XrEnvironmentDepthProviderMETAX1)
#define XR_METAX1_environment_depth_SPEC_VERSION 1
#define XR_METAX1_ENVIRONMENT_DEPTH_EXTENSION_NAME "XR_METAX1_environment_depth"
// XrEnvironmentDepthProviderMETAX1
static const XrObjectType XR_OBJECT_TYPE_ENVIRONMENT_DEPTH_PROVIDER_METAX1 = (XrObjectType) 1000262000;
static const XrStructureType XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_METAX1 = (XrStructureType) 1000262002;
static const XrStructureType XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_METAX1 = (XrStructureType) 1000262003;
static const XrStructureType XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_PROPERTIES_METAX1 = (XrStructureType) 1000262004;
static const XrStructureType XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_METAX1 = (XrStructureType) 1000262005;
static const XrStructureType XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_METAX1 = (XrStructureType) 1000262006;
static const XrStructureType XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_METAX1 = (XrStructureType) 1000262007;
// Warning: The requested depth image is not yet available.
static const XrResult XR_ENVIRONMENT_DEPTH_NOT_AVAILABLE_METAX1 = (XrResult) 1000262000;
typedef XrFlags64 XrEnvironmentDepthProviderCreateFlagsMETAX1;

// Flag bits for XrEnvironmentDepthProviderCreateFlagsMETAX1
// Remove hands from depth maps.
static const XrEnvironmentDepthProviderCreateFlagsMETAX1 XR_ENVIRONMENT_DEPTH_PROVIDER_CREATE_FLAGS_REMOVE_HANDS_BIT_METAX1 = 0x00000001;

typedef XrFlags64 XrEnvironmentDepthSwapchainCreateFlagsMETAX1;

// Flag bits for XrEnvironmentDepthSwapchainCreateFlagsMETAX1

typedef struct XrEnvironmentDepthProviderCreateInfoMETAX1 {
    XrStructureType                                type;
    const void* XR_MAY_ALIAS                       next;
    XrEnvironmentDepthProviderCreateFlagsMETAX1    createFlags;
} XrEnvironmentDepthProviderCreateInfoMETAX1;

typedef struct XrEnvironmentDepthSwapchainCreateInfoMETAX1 {
    XrStructureType                                 type;
    const void* XR_MAY_ALIAS                        next;
    XrEnvironmentDepthSwapchainCreateFlagsMETAX1    createFlags;
} XrEnvironmentDepthSwapchainCreateInfoMETAX1;

typedef struct XrEnvironmentDepthSwapchainPropertiesMETAX1 {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    XrSwapchain               swapchain;
    XrSwapchainCreateFlags    createFlags;
    XrSwapchainUsageFlags     usageFlags;
    int64_t                   format;
    uint32_t                  sampleCount;
    uint32_t                  width;
    uint32_t                  height;
    uint32_t                  faceCount;
    uint32_t                  arraySize;
    uint32_t                  mipCount;
} XrEnvironmentDepthSwapchainPropertiesMETAX1;

typedef struct XrEnvironmentDepthImageAcquireInfoMETAX1 {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      displayTime;
} XrEnvironmentDepthImageAcquireInfoMETAX1;

typedef struct XrEnvironmentDepthImageViewMETAX1 {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrPosef                     pose;
    uint32_t                    swapchainIndex;
    uint32_t                    imageArrayIndex;
    XrFovf                      fov;
    float                       minDepth;
    float                       maxDepth;
    float                       nearZ;
    float                       farZ;
} XrEnvironmentDepthImageViewMETAX1;

typedef struct XrEnvironmentDepthImageMETAX1 {
    XrStructureType                       type;
    const void* XR_MAY_ALIAS              next;
    XrTime                                generationTime;
    XrTime                                displayTime;
    uint32_t                              viewCount;
    XrEnvironmentDepthImageViewMETAX1*    views;
} XrEnvironmentDepthImageMETAX1;

typedef XrResult (XRAPI_PTR *PFN_xrCreateEnvironmentDepthProviderMETAX1)(XrSession session, const XrEnvironmentDepthProviderCreateInfoMETAX1* createInfo, XrEnvironmentDepthProviderMETAX1* environmentDepthProvider);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyEnvironmentDepthProviderMETAX1)(XrEnvironmentDepthProviderMETAX1 environmentDepthProvider);
typedef XrResult (XRAPI_PTR *PFN_xrStartEnvironmentDepthProviderMETAX1)(XrEnvironmentDepthProviderMETAX1 environmentDepthProvider);
typedef XrResult (XRAPI_PTR *PFN_xrStopEnvironmentDepthProviderMETAX1)(XrEnvironmentDepthProviderMETAX1 environmentDepthProvider);
typedef XrResult (XRAPI_PTR *PFN_xrCreateEnvironmentDepthSwapchainMETAX1)(XrEnvironmentDepthProviderMETAX1 environmentDepthProvider, const XrEnvironmentDepthSwapchainCreateInfoMETAX1* createInfo, XrEnvironmentDepthSwapchainPropertiesMETAX1* swapchainProperties);
typedef XrResult (XRAPI_PTR *PFN_xrAcquireEnvironmentDepthImageMETAX1)(XrEnvironmentDepthProviderMETAX1 environmentDepthProvider, const XrEnvironmentDepthImageAcquireInfoMETAX1* acquireInfo, XrEnvironmentDepthImageMETAX1* environmentDepthImage);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateEnvironmentDepthProviderMETAX1(
    XrSession                                   session,
    const XrEnvironmentDepthProviderCreateInfoMETAX1* createInfo,
    XrEnvironmentDepthProviderMETAX1*           environmentDepthProvider);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyEnvironmentDepthProviderMETAX1(
    XrEnvironmentDepthProviderMETAX1            environmentDepthProvider);

XRAPI_ATTR XrResult XRAPI_CALL xrStartEnvironmentDepthProviderMETAX1(
    XrEnvironmentDepthProviderMETAX1            environmentDepthProvider);

XRAPI_ATTR XrResult XRAPI_CALL xrStopEnvironmentDepthProviderMETAX1(
    XrEnvironmentDepthProviderMETAX1            environmentDepthProvider);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateEnvironmentDepthSwapchainMETAX1(
    XrEnvironmentDepthProviderMETAX1            environmentDepthProvider,
    const XrEnvironmentDepthSwapchainCreateInfoMETAX1* createInfo,
    XrEnvironmentDepthSwapchainPropertiesMETAX1* swapchainProperties);

XRAPI_ATTR XrResult XRAPI_CALL xrAcquireEnvironmentDepthImageMETAX1(
    XrEnvironmentDepthProviderMETAX1            environmentDepthProvider,
    const XrEnvironmentDepthImageAcquireInfoMETAX1* acquireInfo,
    XrEnvironmentDepthImageMETAX1*              environmentDepthImage);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_METAX1_environment_depth */

#ifdef  XR_METAX1_ENVIRONMENT_DEPTH_TAG_ALIAS
typedef XrEnvironmentDepthProviderMETAX1 XrEnvironmentDepthProviderMETA;
typedef XrEnvironmentDepthProviderCreateInfoMETAX1 XrEnvironmentDepthProviderCreateInfoMETA;
typedef XrEnvironmentDepthProviderCreateFlagsMETAX1 XrEnvironmentDepthProviderCreateFlagsMETA;

#define XR_ENVIRONMENT_DEPTH_PROVIDER_CREATE_FLAGS_REMOVE_HANDS_BIT_META XR_ENVIRONMENT_DEPTH_PROVIDER_CREATE_FLAGS_REMOVE_HANDS_BIT_METAX1
typedef XrEnvironmentDepthSwapchainCreateFlagsMETAX1 XrEnvironmentDepthSwapchainCreateFlagsMETA;

typedef XrEnvironmentDepthSwapchainCreateInfoMETAX1 XrEnvironmentDepthSwapchainCreateInfoMETA;
typedef XrEnvironmentDepthSwapchainPropertiesMETAX1 XrEnvironmentDepthSwapchainPropertiesMETA;
typedef XrEnvironmentDepthImageAcquireInfoMETAX1 XrEnvironmentDepthImageAcquireInfoMETA;
typedef XrEnvironmentDepthImageViewMETAX1 XrEnvironmentDepthImageViewMETA;
typedef XrEnvironmentDepthImageMETAX1 XrEnvironmentDepthImageMETA;
#define XR_META_environment_depth_SPEC_VERSION XR_METAX1_environment_depth_SPEC_VERSION
#define XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME XR_METAX1_ENVIRONMENT_DEPTH_EXTENSION_NAME
#define XR_OBJECT_TYPE_ENVIRONMENT_DEPTH_PROVIDER_META XR_OBJECT_TYPE_ENVIRONMENT_DEPTH_PROVIDER_METAX1
#define XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_METAX1
#define XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_METAX1
#define XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_PROPERTIES_META XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_PROPERTIES_METAX1
#define XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_METAX1
#define XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_METAX1
#define XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_METAX1
#define XR_ENVIRONMENT_DEPTH_NOT_AVAILABLE_META XR_ENVIRONMENT_DEPTH_NOT_AVAILABLE_METAX1
#endif /* XR_METAX1_ENVIRONMENT_DEPTH_TAG_ALIAS */


#ifdef __cplusplus
}
#endif

#endif
