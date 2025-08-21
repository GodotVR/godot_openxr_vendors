#ifndef XR_ANDROID_LIGHT_ESTIMATION_H_
#define XR_ANDROID_LIGHT_ESTIMATION_H_ 1

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


#ifndef XR_ANDROID_light_estimation

// XR_ANDROID_light_estimation is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_light_estimation 1
XR_DEFINE_HANDLE(XrLightEstimatorANDROID)
#define XR_ANDROID_light_estimation_SPEC_VERSION 1
#define XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME "XR_ANDROID_light_estimation"
#define XR_TYPE_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID ((XrStructureType) 1000700000U)
#define XR_TYPE_LIGHT_ESTIMATE_GET_INFO_ANDROID ((XrStructureType) 1000700001U)
#define XR_TYPE_LIGHT_ESTIMATE_ANDROID    ((XrStructureType) 1000700002U)
#define XR_TYPE_DIRECTIONAL_LIGHT_ANDROID ((XrStructureType) 1000700003U)
#define XR_TYPE_SPHERICAL_HARMONICS_ANDROID ((XrStructureType) 1000700004U)
#define XR_TYPE_AMBIENT_LIGHT_ANDROID     ((XrStructureType) 1000700005U)
#define XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_ANDROID ((XrStructureType) 1000700006U)
// XrLightEstimatorANDROID
#define XR_OBJECT_TYPE_LIGHT_ESTIMATOR_ANDROID ((XrObjectType) 1000700000U)

typedef enum XrLightEstimateStateANDROID {
    // The light estimate is valid
    XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID = 0,
    // The light estimate is invalid
    XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID = 1,
    XR_LIGHT_ESTIMATE_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrLightEstimateStateANDROID;

typedef enum XrSphericalHarmonicsKindANDROID {
    // The spherical harmonic coefficients represent the environment light radiance function, excluding the contribution from the main light.
    XR_SPHERICAL_HARMONICS_KIND_TOTAL_ANDROID = 0,
    // The spherical harmonic coefficients represent the environment light radiance function, including the contribution from the main light.
    XR_SPHERICAL_HARMONICS_KIND_AMBIENT_ANDROID = 1,
    XR_SPHERICAL_HARMONICS_KIND_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrSphericalHarmonicsKindANDROID;
typedef struct XrSystemLightEstimationPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsLightEstimation;
} XrSystemLightEstimationPropertiesANDROID;

typedef struct XrLightEstimatorCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrLightEstimatorCreateInfoANDROID;

typedef struct XrLightEstimateGetInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
} XrLightEstimateGetInfoANDROID;

typedef struct XrLightEstimateANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrLightEstimateStateANDROID    state;
    XrTime                         lastUpdatedTime;
} XrLightEstimateANDROID;

// XrDirectionalLightANDROID extends XrLightEstimateANDROID
typedef struct XrDirectionalLightANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrLightEstimateStateANDROID    state;
    XrVector3f                     intensity;
    XrVector3f                     direction;
} XrDirectionalLightANDROID;

// XrAmbientLightANDROID extends XrLightEstimateANDROID
typedef struct XrAmbientLightANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrLightEstimateStateANDROID    state;
    XrVector3f                     intensity;
    XrVector3f                     colorCorrection;
} XrAmbientLightANDROID;

// XrSphericalHarmonicsANDROID extends XrLightEstimateANDROID
typedef struct XrSphericalHarmonicsANDROID {
    XrStructureType                    type;
    void* XR_MAY_ALIAS                 next;
    XrLightEstimateStateANDROID        state;
    XrSphericalHarmonicsKindANDROID    kind;
    float                              coefficients[9][3];
} XrSphericalHarmonicsANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreateLightEstimatorANDROID)(XrSession session, XrLightEstimatorCreateInfoANDROID* createInfo, XrLightEstimatorANDROID* outHandle);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyLightEstimatorANDROID)(XrLightEstimatorANDROID estimator);
typedef XrResult (XRAPI_PTR *PFN_xrGetLightEstimateANDROID)(XrLightEstimatorANDROID estimator, const XrLightEstimateGetInfoANDROID* input, XrLightEstimateANDROID* output);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateLightEstimatorANDROID(
    XrSession                                   session,
    XrLightEstimatorCreateInfoANDROID*          createInfo,
    XrLightEstimatorANDROID*                    outHandle);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyLightEstimatorANDROID(
    XrLightEstimatorANDROID                     estimator);

XRAPI_ATTR XrResult XRAPI_CALL xrGetLightEstimateANDROID(
    XrLightEstimatorANDROID                     estimator,
    const XrLightEstimateGetInfoANDROID*        input,
    XrLightEstimateANDROID*                     output);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_light_estimation */

#ifdef __cplusplus
}
#endif

#endif
