# Godot OpenXR Loaders

This repository builds the AAR assets required for Godot 4 to export XR application to the Android devices.

## Building this asset

### Linux
You can build this asset after cloning by simply running:
```
./gradlew build
```

### Windows
You can build this asset after cloning by simply running:
```
gradlew.bat build
```

## Downloading this asset

It is possible to download compiled assets that have been released from [releases](https://github.com/GodotVR/godot_openxr_loaders/releases).
The latest official release will be made available in Godot's asset library.

Work in progress builds are available under [actions](https://github.com/GodotVR/godot_openxr_loaders/actions).

## Using this asset

### Installing android build templates

Due to the use of AARs you will need to use the gradle build option in Godot.
Open your Godot XR project in Godot and open the `Project` menu and select `Install Android Build templates`.
This should create an `android` folder in your project folder.

### Copying the plugin into place

When downloading this asset from Godot's asset library it should place the required files into `android/plugins`.
If you've downloaded the zip file from the releases page, copy the files in `aar/android/plugins` manually to `android/plugins` in your project. You may need to create the plugins folder.

If you have build the asset from source, you need to create the `android/plugins` folder and then copy the following files:
- `GodotopenXRMeta.gdap` into `android/plugins`
- `godotopenxrmeta\build\outputs\aar\godotopenxrmeta-release.aar` into `android/plugins/godotopenxrmeta`
- `godotopenxrmeta\build\outputs\aar\godotopenxrmeta-debug.aar` into `android/plugins/godotopenxrmeta`
- `GodotOpenXRPico.gdap` into `android/plugins`
- `godotopenxrpico\build\outputs\aar\godotopenxrpico-release.aar` into `android/plugins/godotopenxrpico`
- `GodotOpenXRKHR.gdap` into `android/plugins`
- `godotopenxrkhr\build\outputs\aar\godotopenxrkhr-release.aar` into `android/plugins/godotopenxrkhr`
- `godotopenxrkhr\build\outputs\aar\godotopenxrkhr-debug.aar` into `android/plugins/godotopenxrkhr`

### Configuring your export

Back in Godot open the `Project` menu and select `Export...`.
If it doesn't already exist create an Android export configuration by pressing `Add...` and selecting `Android`.
Tick the `Use Gradle Build` tickbox.
Now enable the loader you need to use. Do *not* select multiple loaders!

You can create multiple export templates, one for each loader.

## License

Unless specified otherwise, sources in this repository are licensed under MIT, see `LICENSE` for more information.

Note that some vendor-specific components are licensed under separate license terms, which are listed in their corresponding folders.

## About this repository

This repository was created and maintained by Bastiaan "Mux213" Olij. The original setup for this plugin was created by Gergely Kis and Gabor Pal Korom at [Migeran](https://migeran.com).

See CONTRIBUTORS.md for further contributions.
