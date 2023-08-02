# Godot OpenXR Loaders

This repository builds the Android plugin addon required for Godot 4.2 (and higher) to export XR 
applications to the Android devices.

**Note:** 

Version `2.x` of this plugin requires **Godot 4.2**.
Check the [`1.x` branch](https://github.com/GodotVR/godot_openxr_loaders/tree/1.x) and releases 
for support on prior versions of Godot 4.

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

The use of a Godot Android plugin requires the use of the gradle build option in Godot:
- Open your Godot XR project in Godot
- Open the `Project` menu
- Select `Install Android Build templates`

### Copying the plugin into place

When downloading this asset from Godot's asset library it should place the required files into 
the `addons`directory.
If you've downloaded the zip file from the releases page, copy the files in `asset/addons` 
manually to `addons` in your project. You may need to create the `addons` folder.

If you have build the asset from source, you need to create the `addons` folder in your project 
if it doesn't exist already, and then copy the content of the `demo/addons` folder to `addons` 
in your project.

### Configuring your export

Back in Godot open the `Project` menu and select `Export...`:
- If it doesn't already exist, create an Android export configuration by pressing `Add...` and 
selecting `Android`
- Tick the `Use Gradle Build` tickbox
- Select `OpenXR` as the **XR Mode** under the **XR Features** section
- Enable the loader you need to use
- Scroll down to the bottom to see additional features for specific platforms.

**Note:**
**Do not** select multiple loaders in the same export template!
You can instead create multiple export templates, one for each loader.

## License

Unless specified otherwise, sources in this repository are licensed under MIT, see `LICENSE` for more information.

Note that some vendor-specific components are licensed under separate license terms, which are listed in their corresponding folders.

## About this repository

This repository was created and maintained by Bastiaan "Mux213" Olij. The original setup for this plugin was created by Gergely Kis and Gabor Pal Korom at [Migeran](https://migeran.com).

See CONTRIBUTORS.md for further contributions.
