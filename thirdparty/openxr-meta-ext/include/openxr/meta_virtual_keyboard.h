#ifndef META_VIRTUAL_KEYBOARD_H_
#define META_VIRTUAL_KEYBOARD_H_ 1

/**********************
This file is @generated from the OpenXR XML API registry.
Language    :   C99
Copyright   :   (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
***********************/

#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_META_virtual_keyboard

#define XR_META_virtual_keyboard 1
XR_DEFINE_HANDLE(XrVirtualKeyboardMETA)
#define XR_MAX_VIRTUAL_KEYBOARD_COMMIT_TEXT_SIZE_META 3992
#define XR_META_virtual_keyboard_SPEC_VERSION 1
#define XR_META_VIRTUAL_KEYBOARD_EXTENSION_NAME "XR_META_virtual_keyboard"
// XrVirtualKeyboardMETA
static const XrObjectType XR_OBJECT_TYPE_VIRTUAL_KEYBOARD_META = (XrObjectType) 1000219000;
static const XrStructureType XR_TYPE_SYSTEM_VIRTUAL_KEYBOARD_PROPERTIES_META = (XrStructureType) 1000219001;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_CREATE_INFO_META = (XrStructureType) 1000219002;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_SPACE_CREATE_INFO_META = (XrStructureType) 1000219003;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_LOCATION_INFO_META = (XrStructureType) 1000219004;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_MODEL_VISIBILITY_SET_INFO_META = (XrStructureType) 1000219005;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_ANIMATION_STATE_META = (XrStructureType) 1000219006;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_MODEL_ANIMATION_STATES_META = (XrStructureType) 1000219007;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_TEXTURE_DATA_META = (XrStructureType) 1000219009;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_INPUT_INFO_META = (XrStructureType) 1000219010;
static const XrStructureType XR_TYPE_VIRTUAL_KEYBOARD_TEXT_CONTEXT_CHANGE_INFO_META = (XrStructureType) 1000219011;
static const XrStructureType XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_COMMIT_TEXT_META = (XrStructureType) 1000219014;
static const XrStructureType XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_BACKSPACE_META = (XrStructureType) 1000219015;
static const XrStructureType XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_ENTER_META = (XrStructureType) 1000219016;
static const XrStructureType XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_SHOWN_META = (XrStructureType) 1000219017;
static const XrStructureType XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_HIDDEN_META = (XrStructureType) 1000219018;

typedef enum XrVirtualKeyboardLocationTypeMETA {
    // Indicates that the application will provide the position and scale of the keyboard.
    XR_VIRTUAL_KEYBOARD_LOCATION_TYPE_CUSTOM_META = 0,
    // Indicates that the runtime will set the position and scale for far field keyboard.
    XR_VIRTUAL_KEYBOARD_LOCATION_TYPE_FAR_META = 1,
    // Indicates that the runtime will set the position and scale for direct interaction keyboard.
    XR_VIRTUAL_KEYBOARD_LOCATION_TYPE_DIRECT_META = 2,
    XR_VIRTUAL_KEYBOARD_LOCATION_TYPE_MAX_ENUM_META = 0x7FFFFFFF
} XrVirtualKeyboardLocationTypeMETA;

typedef enum XrVirtualKeyboardInputSourceMETA {
    // Left controller ray.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_CONTROLLER_RAY_LEFT_META = 1,
    // Right controller ray.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_CONTROLLER_RAY_RIGHT_META = 2,
    // Left hand ray.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_HAND_RAY_LEFT_META = 3,
    // Right hand ray.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_HAND_RAY_RIGHT_META = 4,
    // Left controller direct touch.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_CONTROLLER_DIRECT_LEFT_META = 5,
    // Right controller direct touch.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_CONTROLLER_DIRECT_RIGHT_META = 6,
    // Left hand direct touch.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_HAND_DIRECT_INDEX_TIP_LEFT_META = 7,
    // Right hand direct touch.
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_HAND_DIRECT_INDEX_TIP_RIGHT_META = 8,
    XR_VIRTUAL_KEYBOARD_INPUT_SOURCE_MAX_ENUM_META = 0x7FFFFFFF
} XrVirtualKeyboardInputSourceMETA;
typedef XrFlags64 XrVirtualKeyboardInputStateFlagsMETA;

// Flag bits for XrVirtualKeyboardInputStateFlagsMETA
// If the input source is considered 'pressed' at all. Pinch for hands, Primary button for controllers.
static const XrVirtualKeyboardInputStateFlagsMETA XR_VIRTUAL_KEYBOARD_INPUT_STATE_PRESSED_BIT_META = 0x00000001;

typedef struct XrSystemVirtualKeyboardPropertiesMETA {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsVirtualKeyboard;
} XrSystemVirtualKeyboardPropertiesMETA;

typedef struct XrVirtualKeyboardCreateInfoMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrVirtualKeyboardCreateInfoMETA;

typedef struct XrVirtualKeyboardSpaceCreateInfoMETA {
    XrStructureType                      type;
    const void* XR_MAY_ALIAS             next;
    XrVirtualKeyboardLocationTypeMETA    locationType;
    XrSpace                              space;
    // only set if locationType == XR_VIRTUAL_KEYBOARD_LOCATION_TYPE_CUSTOM_META
    XrPosef                              poseInSpace;
} XrVirtualKeyboardSpaceCreateInfoMETA;

typedef struct XrVirtualKeyboardLocationInfoMETA {
    XrStructureType                      type;
    const void* XR_MAY_ALIAS             next;
    XrVirtualKeyboardLocationTypeMETA    locationType;
    XrSpace                              space;
    // only set if locationType == XR_VIRTUAL_KEYBOARD_LOCATION_TYPE_CUSTOM_META
    XrPosef                              poseInSpace;
    float                                scale;
} XrVirtualKeyboardLocationInfoMETA;

typedef struct XrVirtualKeyboardModelVisibilitySetInfoMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrBool32                    visible;
} XrVirtualKeyboardModelVisibilitySetInfoMETA;

typedef struct XrVirtualKeyboardAnimationStateMETA {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    int32_t               animationIndex;
    float                 fraction;
} XrVirtualKeyboardAnimationStateMETA;

typedef struct XrVirtualKeyboardModelAnimationStatesMETA {
    XrStructureType                         type;
    void* XR_MAY_ALIAS                      next;
    uint32_t                                stateCapacityInput;
    uint32_t                                stateCountOutput;
    XrVirtualKeyboardAnimationStateMETA*    states;
} XrVirtualKeyboardModelAnimationStatesMETA;

typedef struct XrVirtualKeyboardTextureDataMETA {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    uint32_t              textureWidth;
    uint32_t              textureHeight;
    uint32_t              bufferCapacityInput;
    uint32_t              bufferCountOutput;
    uint8_t*              buffer;
} XrVirtualKeyboardTextureDataMETA;

typedef struct XrVirtualKeyboardInputInfoMETA {
    XrStructureType                         type;
    const void* XR_MAY_ALIAS                next;
    XrVirtualKeyboardInputSourceMETA        inputSource;
    XrSpace                                 inputSpace;
    XrPosef                                 inputPoseInSpace;
    XrVirtualKeyboardInputStateFlagsMETA    inputState;
} XrVirtualKeyboardInputInfoMETA;

typedef struct XrVirtualKeyboardTextContextChangeInfoMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    const char*                 textContext;
} XrVirtualKeyboardTextContextChangeInfoMETA;

typedef struct XrEventDataVirtualKeyboardCommitTextMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrVirtualKeyboardMETA       keyboard;
    char                        text[XR_MAX_VIRTUAL_KEYBOARD_COMMIT_TEXT_SIZE_META];
} XrEventDataVirtualKeyboardCommitTextMETA;

typedef struct XrEventDataVirtualKeyboardBackspaceMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrVirtualKeyboardMETA       keyboard;
} XrEventDataVirtualKeyboardBackspaceMETA;

typedef struct XrEventDataVirtualKeyboardEnterMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrVirtualKeyboardMETA       keyboard;
} XrEventDataVirtualKeyboardEnterMETA;

typedef struct XrEventDataVirtualKeyboardShownMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrVirtualKeyboardMETA       keyboard;
} XrEventDataVirtualKeyboardShownMETA;

typedef struct XrEventDataVirtualKeyboardHiddenMETA {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrVirtualKeyboardMETA       keyboard;
} XrEventDataVirtualKeyboardHiddenMETA;

typedef XrResult (XRAPI_PTR *PFN_xrCreateVirtualKeyboardMETA)(XrSession session, const XrVirtualKeyboardCreateInfoMETA* createInfo, XrVirtualKeyboardMETA* keyboard);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyVirtualKeyboardMETA)(XrVirtualKeyboardMETA keyboard);
typedef XrResult (XRAPI_PTR *PFN_xrCreateVirtualKeyboardSpaceMETA)(XrSession session, XrVirtualKeyboardMETA keyboard, const XrVirtualKeyboardSpaceCreateInfoMETA* createInfo, XrSpace* keyboardSpace);
typedef XrResult (XRAPI_PTR *PFN_xrSuggestVirtualKeyboardLocationMETA)(XrVirtualKeyboardMETA keyboard, const XrVirtualKeyboardLocationInfoMETA* locationInfo);
typedef XrResult (XRAPI_PTR *PFN_xrGetVirtualKeyboardScaleMETA)(XrVirtualKeyboardMETA keyboard, float* scale);
typedef XrResult (XRAPI_PTR *PFN_xrSetVirtualKeyboardModelVisibilityMETA)(XrVirtualKeyboardMETA keyboard, const XrVirtualKeyboardModelVisibilitySetInfoMETA* modelVisibility);
typedef XrResult (XRAPI_PTR *PFN_xrGetVirtualKeyboardModelAnimationStatesMETA)(XrVirtualKeyboardMETA keyboard, XrVirtualKeyboardModelAnimationStatesMETA* animationStates);
typedef XrResult (XRAPI_PTR *PFN_xrGetVirtualKeyboardDirtyTexturesMETA)(XrVirtualKeyboardMETA keyboard, uint32_t textureIdCapacityInput, uint32_t* textureIdCountOutput, uint64_t* textureIds);
typedef XrResult (XRAPI_PTR *PFN_xrGetVirtualKeyboardTextureDataMETA)(XrVirtualKeyboardMETA keyboard, uint64_t textureId, XrVirtualKeyboardTextureDataMETA* textureData);
typedef XrResult (XRAPI_PTR *PFN_xrSendVirtualKeyboardInputMETA)(XrVirtualKeyboardMETA keyboard, const XrVirtualKeyboardInputInfoMETA* info, XrPosef* interactorRootPose);
typedef XrResult (XRAPI_PTR *PFN_xrChangeVirtualKeyboardTextContextMETA)(XrVirtualKeyboardMETA keyboard, const XrVirtualKeyboardTextContextChangeInfoMETA* changeInfo);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateVirtualKeyboardMETA(
    XrSession                                   session,
    const XrVirtualKeyboardCreateInfoMETA*      createInfo,
    XrVirtualKeyboardMETA*                      keyboard);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyVirtualKeyboardMETA(
    XrVirtualKeyboardMETA                       keyboard);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateVirtualKeyboardSpaceMETA(
    XrSession                                   session,
    XrVirtualKeyboardMETA                       keyboard,
    const XrVirtualKeyboardSpaceCreateInfoMETA* createInfo,
    XrSpace*                                    keyboardSpace);

XRAPI_ATTR XrResult XRAPI_CALL xrSuggestVirtualKeyboardLocationMETA(
    XrVirtualKeyboardMETA                       keyboard,
    const XrVirtualKeyboardLocationInfoMETA*    locationInfo);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVirtualKeyboardScaleMETA(
    XrVirtualKeyboardMETA                       keyboard,
    float*                                      scale);

XRAPI_ATTR XrResult XRAPI_CALL xrSetVirtualKeyboardModelVisibilityMETA(
    XrVirtualKeyboardMETA                       keyboard,
    const XrVirtualKeyboardModelVisibilitySetInfoMETA* modelVisibility);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVirtualKeyboardModelAnimationStatesMETA(
    XrVirtualKeyboardMETA                       keyboard,
    XrVirtualKeyboardModelAnimationStatesMETA*  animationStates);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVirtualKeyboardDirtyTexturesMETA(
    XrVirtualKeyboardMETA                       keyboard,
    uint32_t                                    textureIdCapacityInput,
    uint32_t*                                   textureIdCountOutput,
    uint64_t*                                   textureIds);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVirtualKeyboardTextureDataMETA(
    XrVirtualKeyboardMETA                       keyboard,
    uint64_t                                    textureId,
    XrVirtualKeyboardTextureDataMETA*           textureData);

XRAPI_ATTR XrResult XRAPI_CALL xrSendVirtualKeyboardInputMETA(
    XrVirtualKeyboardMETA                       keyboard,
    const XrVirtualKeyboardInputInfoMETA*       info,
    XrPosef*                                    interactorRootPose);

XRAPI_ATTR XrResult XRAPI_CALL xrChangeVirtualKeyboardTextContextMETA(
    XrVirtualKeyboardMETA                       keyboard,
    const XrVirtualKeyboardTextContextChangeInfoMETA* changeInfo);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_META_virtual_keyboard */

#ifdef __cplusplus
}
#endif

#endif
