# Khronos OpenXR Android SDK

Binaries taken from the Official OpenXR working group repo at:
https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/download/release-1.0.26/openxr_loader_for_android-1.0.26.aar

# AndroidManifest changes

Activities that use OpenXR should add the IMMERSIVE_HMD category tag to their intent-filter

```xml
<category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />
```

for example:

```xml
    <intent-filter>
        <action android:name="android.intent.action.MAIN" />
        <category android:name="android.intent.category.LAUNCHER" />
        <category android:name="org.khronos.openxr.intent.category.IMMERSIVE_HMD" />
    </intent-filter>
```

Details on this category tag can be found here:
https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#android-runtime-category
