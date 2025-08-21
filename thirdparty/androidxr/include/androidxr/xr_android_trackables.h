#ifndef XR_ANDROID_TRACKABLES_H_
#define XR_ANDROID_TRACKABLES_H_ 1

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


#ifndef XR_ANDROID_trackables

// XR_ANDROID_trackables is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_trackables 1

#define XR_NULL_TRACKABLE_ANDROID 0

XR_DEFINE_ATOM(XrTrackableANDROID)
XR_DEFINE_HANDLE(XrTrackableTrackerANDROID)
#define XR_ANDROID_trackables_SPEC_VERSION 1
#define XR_ANDROID_TRACKABLES_EXTENSION_NAME "XR_ANDROID_trackables"
#define XR_TYPE_TRACKABLE_GET_INFO_ANDROID ((XrStructureType) 1000455000U)
#define XR_TYPE_ANCHOR_SPACE_CREATE_INFO_ANDROID ((XrStructureType) 1000455001U)
#define XR_TYPE_TRACKABLE_PLANE_ANDROID   ((XrStructureType) 1000455003U)
#define XR_TYPE_TRACKABLE_TRACKER_CREATE_INFO_ANDROID ((XrStructureType) 1000455004U)
#define XR_TYPE_SYSTEM_TRACKABLES_PROPERTIES_ANDROID ((XrStructureType) 1000455005U)
// XrTrackableTrackerANDROID
#define XR_OBJECT_TYPE_TRACKABLE_TRACKER_ANDROID ((XrObjectType) 1000455001U)
// Indicates that the parameters contains multiple trackable types.
#define XR_ERROR_MISMATCHING_TRACKABLE_TYPE_ANDROID ((XrResult) -1000455000U)
// Indicates that the function is not supported by the given trackable type.
#define XR_ERROR_TRACKABLE_TYPE_NOT_SUPPORTED_ANDROID ((XrResult) -1000455001U)

typedef enum XrTrackingStateANDROID {
    // Indicates that the trackable or anchor tracking is paused but may be resumed in the future.
    XR_TRACKING_STATE_PAUSED_ANDROID = 0,
    // Tracking has stopped on this Trackable and will never be resumed.
    XR_TRACKING_STATE_STOPPED_ANDROID = 1,
    // The object is currently tracked and its pose is current.
    XR_TRACKING_STATE_TRACKING_ANDROID = 2,
    XR_TRACKING_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrTrackingStateANDROID;

typedef enum XrTrackableTypeANDROID {
    // Indicates that the trackable is not valid.
    XR_TRACKABLE_TYPE_NOT_VALID_ANDROID = 0,
    // Indicates that the trackable is a plane.
    XR_TRACKABLE_TYPE_PLANE_ANDROID = 1,
    XR_TRACKABLE_TYPE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrTrackableTypeANDROID;

typedef enum XrPlaneTypeANDROID {
    // A horizontal plane facing downward (for example a ceiling).
    XR_PLANE_TYPE_HORIZONTAL_DOWNWARD_FACING_ANDROID = 0,
    // A horizontal plane facing upward (for example a floor or tabletop).
    XR_PLANE_TYPE_HORIZONTAL_UPWARD_FACING_ANDROID = 1,
    // A vertical plane (for example a wall).
    XR_PLANE_TYPE_VERTICAL_ANDROID = 2,
    // A plane with an arbitrary orientation.
    XR_PLANE_TYPE_ARBITRARY_ANDROID = 3,
    XR_PLANE_TYPE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrPlaneTypeANDROID;

typedef enum XrPlaneLabelANDROID {
    // It was not possible to label the plane
    XR_PLANE_LABEL_UNKNOWN_ANDROID = 0,
    // The plane is a wall.
    XR_PLANE_LABEL_WALL_ANDROID = 1,
    // The plane is a floor.
    XR_PLANE_LABEL_FLOOR_ANDROID = 2,
    // The plane is a ceiling.
    XR_PLANE_LABEL_CEILING_ANDROID = 3,
    // The plane is a table.
    XR_PLANE_LABEL_TABLE_ANDROID = 4,
    XR_PLANE_LABEL_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrPlaneLabelANDROID;
typedef struct XrTrackableTrackerCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrTrackableTypeANDROID      trackableType;
} XrTrackableTrackerCreateInfoANDROID;

typedef struct XrTrackableGetInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrTrackableANDROID          trackable;
    XrSpace                     baseSpace;
    XrTime                      time;
} XrTrackableGetInfoANDROID;

typedef struct XrTrackablePlaneANDROID {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    XrTrackingStateANDROID    trackingState;
    XrPosef                   centerPose;
    XrExtent2Df               extents;
    XrPlaneTypeANDROID        planeType;
    XrPlaneLabelANDROID       planeLabel;
    XrTrackableANDROID        subsumedByPlane;
    XrTime                    lastUpdatedTime;
    uint32_t                  vertexCapacityInput;
    uint32_t*                 vertexCountOutput;
    XrVector2f*               vertices;
} XrTrackablePlaneANDROID;

typedef struct XrAnchorSpaceCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
    XrPosef                     pose;
    XrTrackableANDROID          trackable;
} XrAnchorSpaceCreateInfoANDROID;

typedef struct XrSystemTrackablesPropertiesANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrBool32                    supportsAnchor;
    uint32_t                    maxAnchors;
} XrSystemTrackablesPropertiesANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateSupportedTrackableTypesANDROID)(XrInstance instance, XrSystemId systemId, uint32_t trackableTypeCapacityInput, uint32_t* trackableTypeCountOutput, XrTrackableTypeANDROID* trackableTypes);
typedef XrResult (XRAPI_PTR *PFN_xrEnumerateSupportedAnchorTrackableTypesANDROID)(XrInstance instance, XrSystemId systemId, uint32_t trackableTypeCapacityInput, uint32_t* trackableTypeCountOutput, XrTrackableTypeANDROID* trackableTypes);
typedef XrResult (XRAPI_PTR *PFN_xrCreateTrackableTrackerANDROID)(XrSession session, const XrTrackableTrackerCreateInfoANDROID* createInfo, XrTrackableTrackerANDROID* trackableTracker);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyTrackableTrackerANDROID)(XrTrackableTrackerANDROID trackableTracker);
typedef XrResult (XRAPI_PTR *PFN_xrGetAllTrackablesANDROID)(XrTrackableTrackerANDROID trackableTracker, uint32_t trackableCapacityInput, uint32_t* trackableCountOutput, XrTrackableANDROID* trackables);
typedef XrResult (XRAPI_PTR *PFN_xrGetTrackablePlaneANDROID)(XrTrackableTrackerANDROID trackableTracker, const XrTrackableGetInfoANDROID* getInfo, XrTrackablePlaneANDROID* planeOutput);
typedef XrResult (XRAPI_PTR *PFN_xrCreateAnchorSpaceANDROID)(XrSession session, const XrAnchorSpaceCreateInfoANDROID* createInfo, XrSpace* anchorOutput);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSupportedTrackableTypesANDROID(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    trackableTypeCapacityInput,
    uint32_t*                                   trackableTypeCountOutput,
    XrTrackableTypeANDROID*                     trackableTypes);

XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSupportedAnchorTrackableTypesANDROID(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    trackableTypeCapacityInput,
    uint32_t*                                   trackableTypeCountOutput,
    XrTrackableTypeANDROID*                     trackableTypes);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateTrackableTrackerANDROID(
    XrSession                                   session,
    const XrTrackableTrackerCreateInfoANDROID*  createInfo,
    XrTrackableTrackerANDROID*                  trackableTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyTrackableTrackerANDROID(
    XrTrackableTrackerANDROID                   trackableTracker);

XRAPI_ATTR XrResult XRAPI_CALL xrGetAllTrackablesANDROID(
    XrTrackableTrackerANDROID                   trackableTracker,
    uint32_t                                    trackableCapacityInput,
    uint32_t*                                   trackableCountOutput,
    XrTrackableANDROID*                         trackables);

XRAPI_ATTR XrResult XRAPI_CALL xrGetTrackablePlaneANDROID(
    XrTrackableTrackerANDROID                   trackableTracker,
    const XrTrackableGetInfoANDROID*            getInfo,
    XrTrackablePlaneANDROID*                    planeOutput);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateAnchorSpaceANDROID(
    XrSession                                   session,
    const XrAnchorSpaceCreateInfoANDROID*       createInfo,
    XrSpace*                                    anchorOutput);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_trackables */

#ifdef __cplusplus
}
#endif

#endif
