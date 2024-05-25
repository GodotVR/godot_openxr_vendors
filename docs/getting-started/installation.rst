Installation
============

Installing Android build templates
-----------------------------------

Using a Godot Android plugin requires the use of the Gradle build option in Godot:

- Open your Godot XR project in Godot
- Open the `Project` menu
- Select `Install Android Build templates`

Copying the plugin into place
------------------------------

When downloading this asset from Godot's asset library, it should place the required files into
the `addons` directory.
If you've downloaded the zip file from the releases page, copy the files in `asset/addons`
manually to `addons` in your project. You may need to create the `addons` folder.

If you have built the asset from source, you need to create the `addons` folder in your project
if it doesn't exist already, and then copy the content of the `demo/addons` folder to `addons`
in your project.

Configuring your export
-----------------------

Back in Godot, open the `Project` menu, and select `Export...`:

- If it doesn't already exist, create an Android export configuration by pressing `Add...` and
selecting `Android`
- Tick the `Use Gradle Build` tickbox
- Select `OpenXR` as the **XR Mode** under the **XR Features** section
- Enable the XR vendor you need to use
- Scroll down to the bottom to see additional features for specific platforms.

**Note:**
**Do not** select multiple vendors in the same export template!
You can instead create multiple export templates, one for each vendor.