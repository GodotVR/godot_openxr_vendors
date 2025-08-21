#ifndef XR_ANDROID_PERFORMANCE_METRICS_H_
#define XR_ANDROID_PERFORMANCE_METRICS_H_ 1

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


#ifndef XR_ANDROID_performance_metrics

// XR_ANDROID_performance_metrics is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_performance_metrics 1
#define XR_ANDROID_performance_metrics_SPEC_VERSION 1
#define XR_ANDROID_PERFORMANCE_METRICS_EXTENSION_NAME "XR_ANDROID_performance_metrics"
#define XR_TYPE_PERFORMANCE_METRICS_STATE_ANDROID ((XrStructureType) 1000465000U)
#define XR_TYPE_PERFORMANCE_METRICS_COUNTER_ANDROID ((XrStructureType) 1000465001U)

typedef enum XrPerformanceMetricsCounterUnitANDROID {
    // The performance counter unit is generic (unspecified).
    XR_PERFORMANCE_METRICS_COUNTER_UNIT_GENERIC_ANDROID = 0,
    // The performance counter unit is percentage (%).
    XR_PERFORMANCE_METRICS_COUNTER_UNIT_PERCENTAGE_ANDROID = 1,
    // The performance counter unit is millisecond.
    XR_PERFORMANCE_METRICS_COUNTER_UNIT_MILLISECONDS_ANDROID = 2,
    // The performance counter unit is byte.
    XR_PERFORMANCE_METRICS_COUNTER_UNIT_BYTES_ANDROID = 3,
    // The performance counter unit is hertz (Hz).
    XR_PERFORMANCE_METRICS_COUNTER_UNIT_HERTZ_ANDROID = 4,
    XR_PERFORMANCE_METRICS_COUNTER_UNIT_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrPerformanceMetricsCounterUnitANDROID;
typedef XrFlags64 XrPerformanceMetricsCounterFlagsANDROID;

// Flag bits for XrPerformanceMetricsCounterFlagsANDROID
// Indicates any of the values in XrPerformanceMetricsCounterANDROID is valid.
static const XrPerformanceMetricsCounterFlagsANDROID XR_PERFORMANCE_METRICS_COUNTER_ANY_VALUE_VALID_BIT_ANDROID = 0x00000001;
// Indicates the uintValue in XrPerformanceMetricsCounterANDROID is valid.
static const XrPerformanceMetricsCounterFlagsANDROID XR_PERFORMANCE_METRICS_COUNTER_UINT_VALUE_VALID_BIT_ANDROID = 0x00000002;
// Indicates the floatValue in XrPerformanceMetricsCounterANDROID is valid.
static const XrPerformanceMetricsCounterFlagsANDROID XR_PERFORMANCE_METRICS_COUNTER_FLOAT_VALUE_VALID_BIT_ANDROID = 0x00000004;

typedef struct XrPerformanceMetricsStateANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              enabled;
} XrPerformanceMetricsStateANDROID;

typedef struct XrPerformanceMetricsCounterANDROID {
    XrStructureType                            type;
    void* XR_MAY_ALIAS                         next;
    XrPerformanceMetricsCounterFlagsANDROID    counterFlags;
    XrPerformanceMetricsCounterUnitANDROID     counterUnit;
    uint32_t                                   uintValue;
    float                                      floatValue;
} XrPerformanceMetricsCounterANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumeratePerformanceMetricsCounterPathsANDROID)(XrInstance instance, uint32_t counterPathCapacityInput, uint32_t* counterPathCountOutput, XrPath* counterPaths);
typedef XrResult (XRAPI_PTR *PFN_xrSetPerformanceMetricsStateANDROID)(XrSession session, const XrPerformanceMetricsStateANDROID* state);
typedef XrResult (XRAPI_PTR *PFN_xrGetPerformanceMetricsStateANDROID)(XrSession session, XrPerformanceMetricsStateANDROID* state);
typedef XrResult (XRAPI_PTR *PFN_xrQueryPerformanceMetricsCounterANDROID)(XrSession session, XrPath counterPath, XrPerformanceMetricsCounterANDROID* counter);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumeratePerformanceMetricsCounterPathsANDROID(
    XrInstance                                  instance,
    uint32_t                                    counterPathCapacityInput,
    uint32_t*                                   counterPathCountOutput,
    XrPath*                                     counterPaths);

XRAPI_ATTR XrResult XRAPI_CALL xrSetPerformanceMetricsStateANDROID(
    XrSession                                   session,
    const XrPerformanceMetricsStateANDROID*     state);

XRAPI_ATTR XrResult XRAPI_CALL xrGetPerformanceMetricsStateANDROID(
    XrSession                                   session,
    XrPerformanceMetricsStateANDROID*           state);

XRAPI_ATTR XrResult XRAPI_CALL xrQueryPerformanceMetricsCounterANDROID(
    XrSession                                   session,
    XrPath                                      counterPath,
    XrPerformanceMetricsCounterANDROID*         counter);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_ANDROID_performance_metrics */

#ifdef __cplusplus
}
#endif

#endif
