#ifndef XR_ANDROID_COMPOSITION_LAYER_PASSTHROUGH_MESH_H_
#define XR_ANDROID_COMPOSITION_LAYER_PASSTHROUGH_MESH_H_ 1

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


#ifndef XR_ANDROID_composition_layer_passthrough_mesh

// XR_ANDROID_composition_layer_passthrough_mesh is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_composition_layer_passthrough_mesh 1
XR_DEFINE_HANDLE(XrPassthroughLayerANDROID)
#define XR_ANDROID_composition_layer_passthrough_mesh_SPEC_VERSION 1
#define XR_ANDROID_COMPOSITION_LAYER_PASSTHROUGH_MESH_EXTENSION_NAME "XR_ANDROID_composition_layer_passthrough_mesh"
#define XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_ANDROID ((XrStructureType) 1000462000U)
#define XR_TYPE_PASSTHROUGH_LAYER_MESH_ANDROID ((XrStructureType) 1000462001U)
#define XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_ANDROID ((XrStructureType) 1000462002U)
#define XR_TYPE_SYSTEM_PASSTHROUGH_LAYER_PROPERTIES_ANDROID ((XrStructureType) 1000462003U)
// Returned by passthrough layer functions to indicate the mesh data exceeds the supported limit.
#define XR_ERROR_MESH_DATA_LIMIT_EXCEEDED_ANDROID ((XrResult) -1000462000U)
// XrPassthroughLayerANDROID
#define XR_OBJECT_TYPE_PASSTHROUGH_LAYER_ANDROID ((XrObjectType) 1000462000U)

typedef enum XrWindingOrderANDROID {
    // Winding order of the mesh's triangles is not known.
    XR_WINDING_ORDER_UNKNOWN_ANDROID = 0,
    // Winding order of the mesh's triangles is clockwise.
    XR_WINDING_ORDER_CW_ANDROID = 1,
    // Winding order of the mesh's triangles is counter-clockwise.
    XR_WINDING_ORDER_CCW_ANDROID = 2,
    XR_WINDING_ORDER_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrWindingOrderANDROID;
typedef struct XrPassthroughLayerCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    uint32_t                    vertexCapacity;
    uint32_t                    indexCapacity;
} XrPassthroughLayerCreateInfoANDROID;

// XrPassthroughLayerMeshANDROID extends XrPassthroughLayerCreateInfoANDROID
typedef struct XrPassthroughLayerMeshANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrWindingOrderANDROID       windingOrder;
    uint32_t                    vertexCount;
    const XrVector3f*           vertices;
    uint32_t                    indexCount;
    const uint16_t*             indices;
} XrPassthroughLayerMeshANDROID;

// XrCompositionLayerPassthroughANDROID extends XrCompositionLayerBaseHeader
typedef struct XrCompositionLayerPassthroughANDROID {
    XrStructureType              type;
    const void* XR_MAY_ALIAS     next;
    XrCompositionLayerFlags      layerFlags;
    XrSpace                      space;
    XrPosef                      pose;
    XrVector3f                   scale;
    float                        opacity;
    XrPassthroughLayerANDROID    layer;
} XrCompositionLayerPassthroughANDROID;

typedef struct XrSystemPassthroughLayerPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsPassthroughLayer;
    uint32_t              maxMeshIndexCount;
    uint32_t              maxMeshVertexCount;
} XrSystemPassthroughLayerPropertiesANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreatePassthroughLayerANDROID)(XrSession session, const XrPassthroughLayerCreateInfoANDROID* createInfo, XrPassthroughLayerANDROID* layer);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyPassthroughLayerANDROID)(XrPassthroughLayerANDROID layer);
typedef XrResult (XRAPI_PTR *PFN_xrSetPassthroughLayerMeshANDROID)(XrPassthroughLayerANDROID layer, const XrPassthroughLayerMeshANDROID* mesh);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreatePassthroughLayerANDROID(
    XrSession                                   session,
    const XrPassthroughLayerCreateInfoANDROID*  createInfo,
    XrPassthroughLayerANDROID*                  layer);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyPassthroughLayerANDROID(
    XrPassthroughLayerANDROID                   layer);

XRAPI_ATTR XrResult XRAPI_CALL xrSetPassthroughLayerMeshANDROID(
    XrPassthroughLayerANDROID                   layer,
    const XrPassthroughLayerMeshANDROID*        mesh);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_composition_layer_passthrough_mesh */

#ifdef __cplusplus
}
#endif

#endif
