#ifndef XR_ANDROID_RECOMMENDED_RESOLUTION_H_
#define XR_ANDROID_RECOMMENDED_RESOLUTION_H_ 1

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


#ifndef XR_ANDROID_recommended_resolution

// XR_ANDROID_recommended_resolution is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_recommended_resolution 1
#define XR_ANDROID_recommended_resolution_SPEC_VERSION 1
#define XR_ANDROID_RECOMMENDED_RESOLUTION_EXTENSION_NAME "XR_ANDROID_recommended_resolution"
#define XR_TYPE_EVENT_DATA_RECOMMENDED_RESOLUTION_CHANGED_ANDROID ((XrStructureType) 1000461000U)
typedef struct XrEventDataRecommendedResolutionChangedANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrEventDataRecommendedResolutionChangedANDROID;

#endif /* XR_ANDROID_recommended_resolution */

#ifdef __cplusplus
}
#endif

#endif
