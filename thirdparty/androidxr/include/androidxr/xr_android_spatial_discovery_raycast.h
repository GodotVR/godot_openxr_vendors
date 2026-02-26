#ifndef XR_ANDROID_SPATIAL_DISCOVERY_RAYCAST_H_
#define XR_ANDROID_SPATIAL_DISCOVERY_RAYCAST_H_ 1

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


#ifndef XR_ANDROID_spatial_discovery_raycast

// XR_ANDROID_spatial_discovery_raycast is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_spatial_discovery_raycast 1
#define XR_ANDROID_spatial_discovery_raycast_SPEC_VERSION 1
#define XR_ANDROID_SPATIAL_DISCOVERY_RAYCAST_EXTENSION_NAME "XR_ANDROID_spatial_discovery_raycast"
// Raycast against depth buffer
#define XR_SPATIAL_CAPABILITY_DEPTH_RAYCAST_ANDROID ((XrSpatialCapabilityEXT) 1000786000U)
// Component that provides the pose of a raycast hit on an entity. Corresponding list structure is slink:XrSpatialComponentRaycastResultListANDROID; Corresponding data structure is slink:XrSpatialRaycastResultDataANDROID
#define XR_SPATIAL_COMPONENT_TYPE_RAYCAST_RESULT_ANDROID ((XrSpatialComponentTypeEXT) 1000786000U)
#define XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_DEPTH_RAYCAST_ANDROID ((XrStructureType) 1000786000U)
#define XR_TYPE_SPATIAL_RAYCAST_INFO_ANDROID ((XrStructureType) 1000786001U)
#define XR_TYPE_SPATIAL_COMPONENT_RAYCAST_RESULT_LIST_ANDROID ((XrStructureType) 1000786002U)
#define XR_TYPE_SPATIAL_RAYCAST_SNAPSHOT_CREATE_INFO_ANDROID ((XrStructureType) 1000786003U)
typedef struct XrSpatialRaycastResultDataANDROID {
    XrPosef    hitPose;
    float      distanceSquared;
} XrSpatialRaycastResultDataANDROID;

typedef struct XrSpatialCapabilityConfigurationDepthRaycastANDROID {
    XrStructureType                     type;
    const void* XR_MAY_ALIAS            next;
    XrSpatialCapabilityEXT              capability;
    uint32_t                            enabledComponentCount;
    const XrSpatialComponentTypeEXT*    enabledComponents;
} XrSpatialCapabilityConfigurationDepthRaycastANDROID;

// XrSpatialRaycastInfoANDROID extends XrSpatialDiscoverySnapshotCreateInfoEXT
typedef struct XrSpatialRaycastInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
    XrVector3f                  origin;
    XrVector3f                  direction;
    float                       maxDistance;
} XrSpatialRaycastInfoANDROID;

// XrSpatialComponentRaycastResultListANDROID extends XrSpatialComponentDataQueryResultEXT
typedef struct XrSpatialComponentRaycastResultListANDROID {
    XrStructureType                       type;
    void* XR_MAY_ALIAS                    next;
    uint32_t                              raycastResultCount;
    XrSpatialRaycastResultDataANDROID*    raycastResults;
} XrSpatialComponentRaycastResultListANDROID;

typedef struct XrSpatialRaycastSnapshotCreateInfoANDROID {
    XrStructureType                       type;
    const void* XR_MAY_ALIAS              next;
    uint32_t                              componentTypeCount;
    const XrSpatialComponentTypeEXT*      componentTypes;
    const XrSpatialRaycastInfoANDROID*    raycastInfo;
} XrSpatialRaycastSnapshotCreateInfoANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreateSpatialRaycastSnapshotANDROID)(XrSpatialContextEXT spatialContext, const XrSpatialRaycastSnapshotCreateInfoANDROID* createInfo, XrSpatialSnapshotEXT* snapshot);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpatialRaycastSnapshotANDROID(
    XrSpatialContextEXT                         spatialContext,
    const XrSpatialRaycastSnapshotCreateInfoANDROID* createInfo,
    XrSpatialSnapshotEXT*                       snapshot);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_spatial_discovery_raycast */

#ifdef __cplusplus
}
#endif

#endif
