# Godot OpenXR Meta AAR

This repository builds the AAR asset required for Godot 4 to export XR application to the Quest.
It encapsulates Metas OpenXR loader in an asset that can be deployed into a Godot project.

This requires Godot with [PR 65798](https://github.com/godotengine/godot/pull/65798) included. See the Godot documentation on how to build this for Android.

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

It is possible to download compiled assets that have been released from [releases](https://github.com/BastiaanOlij/godot_openxr_meta/releases).
The latest official release will be made available in Godots asset library.

For work in progress builds you can check [actions](https://github.com/BastiaanOlij/godot_openxr_meta/actions) to download the 

## Using this asset

### Installing android build templates

Due to the use of AARs you will need to use the custom build option in Godot.
Open your Godot XR project in Godot and open the `Project` menu and select `Install Android Build templates`.
This should create an `android` folder in your project folder.

### Copying the plugin into place

When downloading this asset from Godots asset library it should place the required files into `android/plugins`.
If you've downloaded the zip file from the releases page, copy the files in `aar/android/plugins` manually to `android/plugins` in your project. You may need to create the plugins folder.

If you have build the asset from source, you need to create the `android/plugins` folder and then copy the following files into this folder:
- `godotopenxrmeta.gdap`
- `godotopenxrmeta\build\outputs\aar\godotopenxrmeta-release.aar`
- `godotopenxrmeta\build\outputs\aar\godotopenxrmeta-debug.aar`

### Configuring your export

Back in Godot open the `Project` menu and select `Export...`.
If it doesn't already exist create an Android export configuration by pressing `Add...` and selecting `Android`.
Tick the `Use Custom Build` tickbox.
Make sure the Godot OpenXR Meta plugin is enabled.

## License

The sources in this repository are licensed under MIT, see `LICENSE` for more information.
Note that the Meta OpenXR loader has its own license.

## About this repository

This repository was created and maintained by Bastiaan "Mux213" Olij. The original setup for this plugin was created by Gergely Kis. 
