# Meta Environment Raycast Sample

> Note: this project requires Godot 4.6 or later

This is a sample project demonstrating the `XR_META_environment_raycast` OpenXR extension supported by the Godot OpenXR Vendors plugin.
See the [Meta Environment Raycast](https://godotvr.github.io/godot_openxr_vendors/manual/meta/environment_raycast.html) tutorial doc for
a detailed walkthrough of this feature.

Environment raycasting casts a ray against the live depth data of the physical environment and returns the point where it hits a real world surface.
It is supported on Meta Quest 3 and Quest 3S.

## What the sample does

- Casts a ray every frame out of the tip of the right index finger (hand tracking), or from the right controller or the headset when no hand is tracked.
- Shows a ring reticle on the physical surface the ray hits.
- Places a small box on the surface on an index finger pinch or when the trigger or A button is pressed. A middle finger pinch or the B button removes them all.

## Setup

1. Enable `xr/openxr/extensions/meta/environment_raycast` and `xr/openxr/extensions/meta/passthrough` in **Project Settings**. For hand input, also enable `xr/openxr/extensions/hand_tracking` and `xr/openxr/extensions/meta/hand_tracking_aim` (all are already enabled in this project).
2. Export with the Meta plugin enabled. The `com.oculus.permission.USE_SCENE` permission is added to the Android manifest and requested at startup automatically.
