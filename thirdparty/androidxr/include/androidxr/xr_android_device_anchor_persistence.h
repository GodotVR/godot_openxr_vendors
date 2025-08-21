#ifndef XR_ANDROID_DEVICE_ANCHOR_PERSISTENCE_H_
#define XR_ANDROID_DEVICE_ANCHOR_PERSISTENCE_H_ 1

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


#ifndef XR_ANDROID_device_anchor_persistence

// XR_ANDROID_device_anchor_persistence is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_device_anchor_persistence 1
XR_DEFINE_HANDLE(XrDeviceAnchorPersistenceANDROID)
#define XR_ANDROID_device_anchor_persistence_SPEC_VERSION 1
#define XR_ANDROID_DEVICE_ANCHOR_PERSISTENCE_EXTENSION_NAME "XR_ANDROID_device_anchor_persistence"
#define XR_TYPE_PERSISTED_ANCHOR_SPACE_CREATE_INFO_ANDROID ((XrStructureType) 1000457001U)
#define XR_TYPE_PERSISTED_ANCHOR_SPACE_INFO_ANDROID ((XrStructureType) 1000457002U)
#define XR_TYPE_DEVICE_ANCHOR_PERSISTENCE_CREATE_INFO_ANDROID ((XrStructureType) 1000457003U)
#define XR_TYPE_SYSTEM_DEVICE_ANCHOR_PERSISTENCE_PROPERTIES_ANDROID ((XrStructureType) 1000457004U)
// XrUuidExt passed to the function was not found to be a persisted anchor.
#define XR_ERROR_ANCHOR_ID_NOT_FOUND_ANDROID ((XrResult) -1000457000U)
// XrUuidExt passed to the function was already marked to be persisted.
#define XR_ERROR_ANCHOR_ALREADY_PERSISTED_ANDROID ((XrResult) -1000457001U)
// ANchor cannot be persisted because it is not tracking.
#define XR_ERROR_ANCHOR_NOT_TRACKING_ANDROID ((XrResult) -1000457002U)
// Persisted data stored by this app has not been loaded yet.
#define XR_ERROR_PERSISTED_DATA_NOT_READY_ANDROID ((XrResult) -1000457003U)
// XrDeviceAnchorPersistenceANDROID
#define XR_OBJECT_TYPE_DEVICE_ANCHOR_PERSISTENCE_ANDROID ((XrObjectType) 1000457000U)

typedef enum XrAnchorPersistStateANDROID {
    // Anchor has not been requested to be persisted by the app.
    XR_ANCHOR_PERSIST_STATE_PERSIST_NOT_REQUESTED_ANDROID = 0,
    // Anchor has been requested to be persisted but not persisted yet.
    XR_ANCHOR_PERSIST_STATE_PERSIST_PENDING_ANDROID = 1,
    // Anchor has been successfully persisted by the runtime.
    XR_ANCHOR_PERSIST_STATE_PERSISTED_ANDROID = 2,
    XR_ANCHOR_PERSIST_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrAnchorPersistStateANDROID;
typedef struct XrDeviceAnchorPersistenceCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrDeviceAnchorPersistenceCreateInfoANDROID;

typedef struct XrPersistedAnchorSpaceCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrUuidEXT                   anchorId;
} XrPersistedAnchorSpaceCreateInfoANDROID;

typedef struct XrPersistedAnchorSpaceInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     anchor;
} XrPersistedAnchorSpaceInfoANDROID;

typedef struct XrSystemDeviceAnchorPersistencePropertiesANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrBool32                    supportsAnchorPersistence;
} XrSystemDeviceAnchorPersistencePropertiesANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateSupportedPersistenceAnchorTypesANDROID)(XrInstance instance, XrSystemId systemId, uint32_t trackableTypeCapacityInput, uint32_t* trackableTypeCountOutput, XrTrackableTypeANDROID* trackableTypes);
typedef XrResult (XRAPI_PTR *PFN_xrCreateDeviceAnchorPersistenceANDROID)(XrSession session, const XrDeviceAnchorPersistenceCreateInfoANDROID* createInfo, XrDeviceAnchorPersistenceANDROID* outHandle);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyDeviceAnchorPersistenceANDROID)(XrDeviceAnchorPersistenceANDROID handle);
typedef XrResult (XRAPI_PTR *PFN_xrPersistAnchorANDROID)(XrDeviceAnchorPersistenceANDROID handle, const XrPersistedAnchorSpaceInfoANDROID* persistedInfo, XrUuidEXT* anchorIdOutput);
typedef XrResult (XRAPI_PTR *PFN_xrGetAnchorPersistStateANDROID)(XrDeviceAnchorPersistenceANDROID handle, const XrUuidEXT* anchorId, XrAnchorPersistStateANDROID* persistState);
typedef XrResult (XRAPI_PTR *PFN_xrCreatePersistedAnchorSpaceANDROID)(XrDeviceAnchorPersistenceANDROID handle, const XrPersistedAnchorSpaceCreateInfoANDROID* createInfo, XrSpace* anchorOutput);
typedef XrResult (XRAPI_PTR *PFN_xrEnumeratePersistedAnchorsANDROID)(XrDeviceAnchorPersistenceANDROID handle, uint32_t anchorIdsCapacityInput, uint32_t* anchorIdsCountOutput, XrUuidEXT* anchorIds);
typedef XrResult (XRAPI_PTR *PFN_xrUnpersistAnchorANDROID)(XrDeviceAnchorPersistenceANDROID handle, const XrUuidEXT* anchorId);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSupportedPersistenceAnchorTypesANDROID(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    trackableTypeCapacityInput,
    uint32_t*                                   trackableTypeCountOutput,
    XrTrackableTypeANDROID*                     trackableTypes);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateDeviceAnchorPersistenceANDROID(
    XrSession                                   session,
    const XrDeviceAnchorPersistenceCreateInfoANDROID* createInfo,
    XrDeviceAnchorPersistenceANDROID*           outHandle);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyDeviceAnchorPersistenceANDROID(
    XrDeviceAnchorPersistenceANDROID            handle);

XRAPI_ATTR XrResult XRAPI_CALL xrPersistAnchorANDROID(
    XrDeviceAnchorPersistenceANDROID            handle,
    const XrPersistedAnchorSpaceInfoANDROID*    persistedInfo,
    XrUuidEXT*                                  anchorIdOutput);

XRAPI_ATTR XrResult XRAPI_CALL xrGetAnchorPersistStateANDROID(
    XrDeviceAnchorPersistenceANDROID            handle,
    const XrUuidEXT*                            anchorId,
    XrAnchorPersistStateANDROID*                persistState);

XRAPI_ATTR XrResult XRAPI_CALL xrCreatePersistedAnchorSpaceANDROID(
    XrDeviceAnchorPersistenceANDROID            handle,
    const XrPersistedAnchorSpaceCreateInfoANDROID* createInfo,
    XrSpace*                                    anchorOutput);

XRAPI_ATTR XrResult XRAPI_CALL xrEnumeratePersistedAnchorsANDROID(
    XrDeviceAnchorPersistenceANDROID            handle,
    uint32_t                                    anchorIdsCapacityInput,
    uint32_t*                                   anchorIdsCountOutput,
    XrUuidEXT*                                  anchorIds);

XRAPI_ATTR XrResult XRAPI_CALL xrUnpersistAnchorANDROID(
    XrDeviceAnchorPersistenceANDROID            handle,
    const XrUuidEXT*                            anchorId);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_device_anchor_persistence */

#ifdef __cplusplus
}
#endif

#endif
