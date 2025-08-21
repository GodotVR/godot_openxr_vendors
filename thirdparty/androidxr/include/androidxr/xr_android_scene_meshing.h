#ifndef XR_ANDROID_SCENE_MESHING_H_
#define XR_ANDROID_SCENE_MESHING_H_ 1

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


#ifndef XR_ANDROID_scene_meshing

// XR_ANDROID_scene_meshing is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_scene_meshing 1
XR_DEFINE_HANDLE(XrSceneMeshingTrackerANDROID)
XR_DEFINE_HANDLE(XrSceneMeshSnapshotANDROID)
#define XR_ANDROID_scene_meshing_SPEC_VERSION 3
#define XR_ANDROID_SCENE_MESHING_EXTENSION_NAME "XR_ANDROID_scene_meshing"
#define XR_TYPE_SYSTEM_SCENE_MESHING_PROPERTIES_ANDROID ((XrStructureType) 1000718000U)
#define XR_TYPE_SCENE_MESHING_TRACKER_CREATE_INFO_ANDROID ((XrStructureType) 1000718001U)
#define XR_TYPE_SCENE_MESH_SNAPSHOT_CREATE_INFO_ANDROID ((XrStructureType) 1000718002U)
#define XR_TYPE_SCENE_MESH_SNAPSHOT_CREATION_RESULT_ANDROID ((XrStructureType) 1000718003U)
#define XR_TYPE_SCENE_SUBMESH_STATE_ANDROID ((XrStructureType) 1000718004U)
#define XR_TYPE_SCENE_SUBMESH_DATA_ANDROID ((XrStructureType) 1000718005U)
// XrSceneMeshingTrackerANDROID
#define XR_OBJECT_TYPE_SCENE_MESHING_TRACKER_ANDROID ((XrObjectType) 1000718000U)
// XrSceneMeshSnapshotANDROID
#define XR_OBJECT_TYPE_SCENE_MESH_SNAPSHOT_ANDROID ((XrObjectType) 1000718001U)

typedef enum XrSceneMeshSemanticLabelSetANDROID {
    // This semantic label set represents no semantic labels.
    XR_SCENE_MESH_SEMANTIC_LABEL_SET_NONE_ANDROID = 0,
    // This semantic label set represents XrSceneMeshSemanticLabelANDROID.
    XR_SCENE_MESH_SEMANTIC_LABEL_SET_DEFAULT_ANDROID = 1,
    XR_SCENE_MESH_SEMANTIC_LABEL_SET_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrSceneMeshSemanticLabelSetANDROID;

typedef enum XrSceneMeshSemanticLabelANDROID {
    // This semantic indicates that the corresponding mesh element represents an unknown object.
    XR_SCENE_MESH_SEMANTIC_LABEL_OTHER_ANDROID = 0,
    // This semantic indicates that the corresponding mesh element represents a floor.
    XR_SCENE_MESH_SEMANTIC_LABEL_FLOOR_ANDROID = 1,
    // This semantic indicates that the corresponding mesh element represents a ceiling.
    XR_SCENE_MESH_SEMANTIC_LABEL_CEILING_ANDROID = 2,
    // This semantic indicates that the corresponding mesh element represents a wall.
    XR_SCENE_MESH_SEMANTIC_LABEL_WALL_ANDROID = 3,
    // This semantic indicates that the corresponding mesh element represents a table.
    XR_SCENE_MESH_SEMANTIC_LABEL_TABLE_ANDROID = 4,
    XR_SCENE_MESH_SEMANTIC_LABEL_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrSceneMeshSemanticLabelANDROID;

typedef enum XrSceneMeshTrackingStateANDROID {
    // The internal tracker is not yet ready to provide mesh data.
    XR_SCENE_MESH_TRACKING_STATE_INITIALIZING_ANDROID = 0,
    // The internal tracker is actively tracking.
    XR_SCENE_MESH_TRACKING_STATE_TRACKING_ANDROID = 1,
    // The internal tracker is waiting for valid measurements to integrate since the last mesh update.
    XR_SCENE_MESH_TRACKING_STATE_WAITING_ANDROID = 2,
    // The internal tracker has not received valid measurements for multiple cycles and is in an error state.
    XR_SCENE_MESH_TRACKING_STATE_ERROR_ANDROID = 3,
    XR_SCENE_MESH_TRACKING_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrSceneMeshTrackingStateANDROID;
typedef struct XrSystemSceneMeshingPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsSceneMeshing;
} XrSystemSceneMeshingPropertiesANDROID;

typedef struct XrSceneMeshingTrackerCreateInfoANDROID {
    XrStructureType                       type;
    const void* XR_MAY_ALIAS              next;
    XrSceneMeshSemanticLabelSetANDROID    semanticLabelSet;
    XrBool32                              enableNormals;
} XrSceneMeshingTrackerCreateInfoANDROID;

typedef struct XrSceneMeshSnapshotCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     baseSpace;
    XrTime                      time;
    XrBoxf                      boundingBox;
} XrSceneMeshSnapshotCreateInfoANDROID;

typedef struct XrSceneMeshSnapshotCreationResultANDROID {
    XrStructureType                    type;
    const void* XR_MAY_ALIAS           next;
    XrSceneMeshSnapshotANDROID         snapshot;
    XrSceneMeshTrackingStateANDROID    trackingState;
} XrSceneMeshSnapshotCreationResultANDROID;

typedef struct XrSceneSubmeshStateANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrUuid                submeshId;
    XrTime                lastUpdatedTime;
    XrPosef               submeshPoseInBaseSpace;
    XrExtent3Df           bounds;
} XrSceneSubmeshStateANDROID;

typedef struct XrSceneSubmeshDataANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrUuid                      submeshId;
    uint32_t                    vertexCapacityInput;
    uint32_t                    vertexCountOutput;
    XrVector3f*                 vertexPositions;
    XrVector3f*                 vertexNormals;
    uint8_t*                    vertexSemantics;
    uint32_t                    indexCapacityInput;
    uint32_t                    indexCountOutput;
    uint32_t*                   indices;
} XrSceneSubmeshDataANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateSupportedSemanticLabelSetsANDROID)(XrInstance instance, XrSystemId systemId, uint32_t supportedSemanticLabelSetsInputCapacity, uint32_t* supportedSemanticLabelSetsOutputCount, XrSceneMeshSemanticLabelSetANDROID* supportedSemanticLabelSets);
typedef XrResult (XRAPI_PTR *PFN_xrCreateSceneMeshingTrackerANDROID)(XrSession session, const XrSceneMeshingTrackerCreateInfoANDROID* createInfo, XrSceneMeshingTrackerANDROID* tracker);
typedef XrResult (XRAPI_PTR *PFN_xrDestroySceneMeshingTrackerANDROID)(XrSceneMeshingTrackerANDROID tracker);
typedef XrResult (XRAPI_PTR *PFN_xrCreateSceneMeshSnapshotANDROID)(XrSceneMeshingTrackerANDROID tracker, const XrSceneMeshSnapshotCreateInfoANDROID* createInfo, XrSceneMeshSnapshotCreationResultANDROID* outSnapshotCreationResult);
typedef XrResult (XRAPI_PTR *PFN_xrDestroySceneMeshSnapshotANDROID)(XrSceneMeshSnapshotANDROID snapshot);
typedef XrResult (XRAPI_PTR *PFN_xrGetAllSubmeshStatesANDROID)(XrSceneMeshSnapshotANDROID snapshot, uint32_t submeshStateCapacityInput, uint32_t* submeshStateCountOutput, XrSceneSubmeshStateANDROID* submeshStates);
typedef XrResult (XRAPI_PTR *PFN_xrGetSubmeshDataANDROID)(XrSceneMeshSnapshotANDROID snapshot, uint32_t submeshDataCount, XrSceneSubmeshDataANDROID* inoutSubmeshData);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSupportedSemanticLabelSetsANDROID(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    supportedSemanticLabelSetsInputCapacity,
    uint32_t*                                   supportedSemanticLabelSetsOutputCount,
    XrSceneMeshSemanticLabelSetANDROID*         supportedSemanticLabelSets);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateSceneMeshingTrackerANDROID(
    XrSession                                   session,
    const XrSceneMeshingTrackerCreateInfoANDROID* createInfo,
    XrSceneMeshingTrackerANDROID*               tracker);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroySceneMeshingTrackerANDROID(
    XrSceneMeshingTrackerANDROID                tracker);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateSceneMeshSnapshotANDROID(
    XrSceneMeshingTrackerANDROID                tracker,
    const XrSceneMeshSnapshotCreateInfoANDROID* createInfo,
    XrSceneMeshSnapshotCreationResultANDROID*   outSnapshotCreationResult);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroySceneMeshSnapshotANDROID(
    XrSceneMeshSnapshotANDROID                  snapshot);

XRAPI_ATTR XrResult XRAPI_CALL xrGetAllSubmeshStatesANDROID(
    XrSceneMeshSnapshotANDROID                  snapshot,
    uint32_t                                    submeshStateCapacityInput,
    uint32_t*                                   submeshStateCountOutput,
    XrSceneSubmeshStateANDROID*                 submeshStates);

XRAPI_ATTR XrResult XRAPI_CALL xrGetSubmeshDataANDROID(
    XrSceneMeshSnapshotANDROID                  snapshot,
    uint32_t                                    submeshDataCount,
    XrSceneSubmeshDataANDROID*                  inoutSubmeshData);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_scene_meshing */

#ifdef __cplusplus
}
#endif

#endif
