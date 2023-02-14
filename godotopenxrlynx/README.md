# Godot OpenXR Lynx Android loader

Binaries taken from Lynx at:
https://portal.lynx-r.com/downloads/item/1/download/latest

# AndroidManifest changes

Add this to your manifest to enable OpenXR and Ultraleap's hand tracking:

```
<uses-feature android:name="android.hardware.vr.headtracking" android:required="true" android:version="1"/>
<uses-permission android:name="org.khronos.openxr.permission.OPENXR"/>

<queries>
    <intent>
        <action android:name="org.khronos.openxr.OpenXRRuntimeService"/>
    </intent>
    <provider android:authorities="org.khronos.openxr.runtime_broker;org.khronos.openxr.system_runtime_broker" />
    <package android:name="com.ultraleap.tracking.service"/>
    <package android:name="com.ultraleap.openxr.api_layer"/>
</queries>
```

And add this to the `application` block in your manifest to enable hand tracking:

```
<meta-data android:name="handtracking" android:value="1" />
```
