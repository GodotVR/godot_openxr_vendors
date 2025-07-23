# Change history for the Godot OpenXR loaders asset

## 4.1.1

- Update the export plugin version to match the maven central release

## 4.1.0

- Implement `XR_META_boundary_visibility` extension
- Add HorizonOS camera permissions when the Android CAMERA permission is enabled
- Implement `XR_FB_space_warp` extension (only with Godot 4.5+)
- Implement `XR_META_environment_depth` extension (only with Godot 4.5+)
- Implement `XR_FB_color_space` extension
- Update OpenXR to 1.1.49 release
- Implement `XR_META_body_tracking_full_body`, `XR_META_body_tracking_fidelity` and `XR_META_body_tracking_callibration`
- Clean-up editor plugins and class registration
- `OpenXRFbSceneManager`: Clarify how to check if scene capture is possible

## 4.0.0

- Support making hybrid apps for Meta headsets
- Add support for `XR_FB_android_surface_swapchain_create`
- Implement `XR_META_recommended_layer_resolution`
- Remove CMake from the build process
- Implement instant splash screen for Meta headsets
- Avoid casting errors when building with `precision=double`
- Add missing Pico store manifest
- Add support for `XR_FB_composition_layer_image_layout`
- Update demo and samples for Godot 4.4
- Switch Meta and Lynx to the Khronos loader
- Fix `OpenXRFbSpatialEntityStorageExtensionWrapper` typos
- Add support for `XR_FB_composition_layer_depth_test`
- Use project settings to avoid enabling unneeded OpenXR extensions
- Passthrough extensions should override real alpha blend mode, if enabled
- Update the main manifest with the latest from the Khronos OpenXR loader AAR
- Improve hand tracking related code in demo project

## 3.1.2
- Fix passthrough sample color map display bug
- Fix the issue preventing overridden vendor options from being updated

## 3.1.1
- Emit correct signal from `OpenXRFbSpatialEntity.erase_from_storage()`
- Only add the `LAUNCHER` category if "Show in App Library" is enabled

## 3.1.0
- Fix wall, floor and ceiling collision shapes with Jolt physics
- Add option to enable permissions necessary for sharing of Meta's spatial anchors
- Fix crash when attempting to create a spatial anchor too early
- Reworked geometric algebra used by Meta body tracking extension
- Remove deprecated "Contextual" boundary mode on Meta Quest
- Add two modes of Pico hand tracking to selectable export features
- Fix `OpenXRFbPassthroughExtensionWrapper` from wiping out the next pointer chain for system properties
- Update OpenXR to Khronos 1.1.41 release
- Add a project setting to enable/disable automatic requests for runtime permissions
- Add export profile for Magic Leap 2

## 3.0.1
- Fixed hand tracking issue

## 3.0.0
- Merge GDExtension into a single implementation
- Upgrade Android, Gradle, Godot and Kotlin dependencies
- Add XR_FB_face_tracking support
- Update to OpenXR 1.0.34 headers
- Add XR_FB_render_model extension wrapper and OpenXRFBRenderModel node
- Add XR_FB_passthrough extension wrapper
- Add XR_FB_hand_tracking_mesh extension wrapper and OpenXRFbHandTrackingMesh node
- Add XR_FB_hand_tracking_aim support
- Update Meta OpenXR mobile SDK to version 62
- Add a developer-facing API for interacting with scene anchors
- Add support for developer-created spatial anchors via XR_FB_spatial_entity
- Add XR_FB_hand_tracking_capsules extension wrapper
- Add OpenXRFbPassthroughGeometry node
- Add OpenXRMetaPassthroughColorLut
- Add feature flags to Khronos loader for HTC
- Add XR_HTC_passthrough extension wrapper
- Add manifest entries to Pico and switch Pico to using the Khronos Loader
- Add Meta Passthrough tutorial doc

## 2.0.4
- Fix misc crash when reloading project on Godot 4.3
- Fix issue with only the first permission being requested

## 2.0.3
- Migrate the export scripts from gdscript to C++ via gdextension
- Manually request eye tracking permission if it's included in the app manifest
- Change how singletons are accessed
- Fix the plugin version for the export plugins
- Add OpenXR extension wrappers for fb_scene, fb_spatial_entity, fb_spatial_entity_query, fb_spatial_entity_container

## 2.0.0
- Update to the new Godot 4.2 Android plugin packaging format
- Update the plugin to Godot v2 Android plugin
- Update to the Godot 4.2 Android library
- Add warning when multiple loaders are selected
- Add configs for the OpenXR Eye gaze interaction extension
- Add the ability to customize supported Meta devices
- Add support for Quest 3 devices
- Update the directory structure for the v2 plugin
- Update Meta OpenXR mobile SDK to version 57
- Update the java version to 17
- Rename the plugin to 'Godot OpenXR Vendors'
- Add godot-cpp dependency
- Add OpenXR 1.0.30 headers
- Add support for the Meta scene capture API (Donated by [Migeran](https://migeran.com))

## 1.1.0
- Update Meta OpenXR loader to version 54
- Update PICO OpenXR loader to version 2.2.0
- Bump dependencies versions to match the latest Godot 4.x stable version (v4.0.3)

## 1.0.0
- First version
- Added support for Meta Quest loader
- Added support for Pico loader
- Added support for Khronos loader (Magic Leap 2, HTC, etc.)
- Added support for Lynx loader
- Add logic to automatically publish the Godot OpenXR loaders libraries to mavencentral on release
