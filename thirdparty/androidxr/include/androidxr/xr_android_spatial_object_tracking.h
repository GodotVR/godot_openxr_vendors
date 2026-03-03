#ifndef XR_ANDROID_SPATIAL_OBJECT_TRACKING_H_
#define XR_ANDROID_SPATIAL_OBJECT_TRACKING_H_ 1

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


#ifndef XR_ANDROID_spatial_object_tracking

// XR_ANDROID_spatial_object_tracking is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_spatial_object_tracking 1
#define XR_ANDROID_spatial_object_tracking_SPEC_VERSION 1
#define XR_ANDROID_SPATIAL_OBJECT_TRACKING_EXTENSION_NAME "XR_ANDROID_spatial_object_tracking"
#define XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_OBJECT_TRACKING_ANDROID ((XrStructureType) 1000785000U)
#define XR_TYPE_SPATIAL_COMPONENT_OBJECT_SEMANTIC_LABEL_LIST_ANDROID ((XrStructureType) 1000785001U)
// Object tracking
#define XR_SPATIAL_CAPABILITY_OBJECT_TRACKING_ANDROID ((XrSpatialCapabilityEXT) 1000785000U)
// Component that provides a semantic label for a object; Corresponding list structure is slink:XrSpatialComponentObjectSemanticLabelListANDROID; Corresponding data structure is elink:XrSpatialObjectSemanticLabelANDROID
#define XR_SPATIAL_COMPONENT_TYPE_OBJECT_SEMANTIC_LABEL_ANDROID ((XrSpatialComponentTypeEXT) 1000785000U)

typedef enum XrSpatialObjectSemanticLabelANDROID {
    // The runtime was unable to classify this entity.
    XR_SPATIAL_OBJECT_SEMANTIC_LABEL_UNCATEGORIZED_ANDROID = 0,
    // The entity is a keyboard.
    XR_SPATIAL_OBJECT_SEMANTIC_LABEL_KEYBOARD_ANDROID = 1,
    // The entity is a mouse.
    XR_SPATIAL_OBJECT_SEMANTIC_LABEL_MOUSE_ANDROID = 2,
    // The entity is a laptop.
    XR_SPATIAL_OBJECT_SEMANTIC_LABEL_LAPTOP_BASE_ANDROID = 3,
    XR_SPATIAL_OBJECT_SEMANTIC_LABEL_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrSpatialObjectSemanticLabelANDROID;
typedef struct XrSpatialCapabilityConfigurationObjectTrackingANDROID {
    XrStructureType                               type;
    const void* XR_MAY_ALIAS                      next;
    XrSpatialCapabilityEXT                        capability;
    uint32_t                                      enabledComponentCount;
    const XrSpatialComponentTypeEXT*              enabledComponents;
    uint32_t                                      activeSemanticLabelCount;
    const XrSpatialObjectSemanticLabelANDROID*    activeSemanticLabels;
} XrSpatialCapabilityConfigurationObjectTrackingANDROID;

// XrSpatialComponentObjectSemanticLabelListANDROID extends XrSpatialComponentDataQueryResultEXT
typedef struct XrSpatialComponentObjectSemanticLabelListANDROID {
    XrStructureType                         type;
    void* XR_MAY_ALIAS                      next;
    uint32_t                                semanticLabelCount;
    XrSpatialObjectSemanticLabelANDROID*    semanticLabels;
} XrSpatialComponentObjectSemanticLabelListANDROID;

#endif /* XR_ANDROID_spatial_object_tracking */

#ifdef __cplusplus
}
#endif

#endif
