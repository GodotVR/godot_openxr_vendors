#ifndef XR_ANDROID_RAYCAST_H_
#define XR_ANDROID_RAYCAST_H_ 1

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


#ifndef XR_ANDROID_raycast

// XR_ANDROID_raycast is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_raycast 1
#define XR_ANDROID_raycast_SPEC_VERSION   1
#define XR_ANDROID_RAYCAST_EXTENSION_NAME "XR_ANDROID_raycast"
#define XR_TYPE_RAYCAST_INFO_ANDROID      ((XrStructureType) 1000463000U)
#define XR_TYPE_RAYCAST_HIT_RESULTS_ANDROID ((XrStructureType) 1000463001U)
// Indicates that the trackable is the perception depth buffer.
#define XR_TRACKABLE_TYPE_DEPTH_ANDROID   ((XrTrackableTypeANDROID) 1000463000U)
typedef struct XrRaycastInfoANDROID {
    XrStructureType                     type;
    const void* XR_MAY_ALIAS            next;
    uint32_t                            maxResults;
    uint32_t                            trackerCount;
    const XrTrackableTrackerANDROID*    trackers;
    XrVector3f                          origin;
    XrVector3f                          trajectory;
    XrSpace                             space;
    XrTime                              time;
} XrRaycastInfoANDROID;

typedef struct XrRaycastHitResultANDROID {
    XrTrackableTypeANDROID    type;
    XrTrackableANDROID        trackable;
    XrPosef                   pose;
} XrRaycastHitResultANDROID;

typedef struct XrRaycastHitResultsANDROID {
    XrStructureType               type;
    void* XR_MAY_ALIAS            next;
    uint32_t                      resultsCapacityInput;
    uint32_t                      resultsCountOutput;
    XrRaycastHitResultANDROID*    results;
} XrRaycastHitResultsANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateRaycastSupportedTrackableTypesANDROID)(XrInstance instance, XrSystemId systemId, uint32_t trackableTypeCapacityInput, uint32_t* trackableTypeCountOutput, XrTrackableTypeANDROID* trackableTypes);
typedef XrResult (XRAPI_PTR *PFN_xrRaycastANDROID)(XrSession session, const XrRaycastInfoANDROID* rayInfo, XrRaycastHitResultsANDROID* results);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateRaycastSupportedTrackableTypesANDROID(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    trackableTypeCapacityInput,
    uint32_t*                                   trackableTypeCountOutput,
    XrTrackableTypeANDROID*                     trackableTypes);

XRAPI_ATTR XrResult XRAPI_CALL xrRaycastANDROID(
    XrSession                                   session,
    const XrRaycastInfoANDROID*                 rayInfo,
    XrRaycastHitResultsANDROID*                 results);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_raycast */

#ifdef __cplusplus
}
#endif

#endif
