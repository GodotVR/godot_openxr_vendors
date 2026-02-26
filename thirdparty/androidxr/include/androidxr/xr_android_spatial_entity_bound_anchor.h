#ifndef XR_ANDROID_SPATIAL_ENTITY_BOUND_ANCHOR_H_
#define XR_ANDROID_SPATIAL_ENTITY_BOUND_ANCHOR_H_ 1

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


#ifndef XR_ANDROID_spatial_entity_bound_anchor

// XR_ANDROID_spatial_entity_bound_anchor is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_spatial_entity_bound_anchor 1
#define XR_ANDROID_spatial_entity_bound_anchor_SPEC_VERSION 1
#define XR_ANDROID_SPATIAL_ENTITY_BOUND_ANCHOR_EXTENSION_NAME "XR_ANDROID_spatial_entity_bound_anchor"
// Spatial entity with parent ID does not have at least one component enumerated by xrEnumerateSpatialAnchorAttachableComponentsANDROID
#define XR_ERROR_SPATIAL_ANCHOR_ATTACHABLE_COMPONENT_NOT_FOUND_ANDROID ((XrResult) -1000790001U)
#define XR_TYPE_SPATIAL_ANCHOR_PARENT_ANDROID ((XrStructureType) 1000790000U)
// XrSpatialAnchorParentANDROID extends XrSpatialAnchorCreateInfoEXT
typedef struct XrSpatialAnchorParentANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpatialEntityIdEXT        parentId;
} XrSpatialAnchorParentANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateSpatialAnchorAttachableComponentsANDROID)(XrInstance instance, XrSystemId systemId, uint32_t attachableComponentCapacityInput, uint32_t* attachableComponentCountOutput, XrSpatialComponentTypeEXT* attachableComponents);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSpatialAnchorAttachableComponentsANDROID(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    attachableComponentCapacityInput,
    uint32_t*                                   attachableComponentCountOutput,
    XrSpatialComponentTypeEXT*                  attachableComponents);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_spatial_entity_bound_anchor */

#ifdef __cplusplus
}
#endif

#endif
