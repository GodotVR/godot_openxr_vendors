Obtaining the plugin
====================

There are four methods of obtaining a copy of this plugin.

Installing from the asset store (Godot 4.7 & newer)
---------------------------------------------------

For users on Godot 4.7 & newer, this is the easiest and preferred way of deploying the plugin from
the Godot editor.
After creating your project in the Godot editor, open the ``Asset Store`` and search for ``OpenXR vendors``.
You can now find the latest releases of the plugin and the Godot editor will install it.

Installing from the asset library (Godot 4.6 & older)
-----------------------------------------------------

For users on Godot 4.6 & older, this is the easiest and preferred way of deploying the plugin from
the Godot editor.
After creating your project in the Godot editor, open the asset library and search for ``OpenXR vendors``.
You can now find the latest releases of this plugin and Godot will install the plugin for you.

Downloading from the releases page
----------------------------------

All releases can be found on the `plugins releases page <https://github.com/GodotVR/godot_openxr_vendors/releases>`_.
Once you find the correct release, unfold the ``Assets`` section and download the ``godotopenxrvendorsaddon.zip`` file.

In Godot you can open the asset library and use the ``Import...`` button to properly install the plugin.

Alternatively you can extract the zip file and copy the files into place.

.. note::

    Note that the zip file contains a root folder as as required by the asset library.
    This root folder should not be recreated in your project.
    The plugin must be installed into ``res://addons/godotopenxrvendors``!

Building from source
--------------------

If you wish to use the latest changes, or add your own modifications you need to build the plugin from source.
You should clone the repository in a folder outside of your Godot project.

You can clone the repository by running the following command:

.. code-block::

    git clone --recursive https://github.com/godotvr/godot_openxr_vendors

Follow the instructions in :doc:`building` to build the plugin.

You can now copy the folder ``demo\addons\godotopenxrvendors`` into your project.

Updating the vendors plugin
---------------------------

Godot currently does not have a method of updating the plugin.
You need to manually delete the ``godotopenxrvendors`` folder and repeat the installation process.
