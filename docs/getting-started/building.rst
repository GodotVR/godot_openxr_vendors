Building this asset
===================

.. note::

   * Version ``5.x`` and higher of this plugin requires **Godot 4.6 or newer**
   * Version ``4.x`` and higher of this plugin requires **Godot 4.4 or newer**
   * Version ``3.x`` and higher of this plugin requires **Godot 4.3 or newer**
   * Version ``2.x`` and higher of this plugin requires **Godot 4.2 or newer**

Check the `1.x branch <https://github.com/GodotVR/godot_openxr_vendors/tree/1.x>`_ and releases for support on prior versions of Godot 4.

After cloning this project, run the following command in the project root directory to initialize
the ``godot-cpp`` submodule:

.. code-block:: shell

   git submodule update --init

Short build instructions
------------------------

The following build command run all the build commands mentioned in the 'Expanded build instructions' section below.

Linux / MacOS
^^^^^^^^^^^^^

Run the following command from the root directory to build the plugin artifacts:

.. code-block:: shell

   ./gradlew buildPlugin

Windows
^^^^^^^

Run the following command from the root directory to build the plugin artifacts:

.. code-block:: shell

   gradlew.bat buildPlugin

Expanded build instructions
---------------------------

These are all the build commands that are being run by the ``gradlew buildPlugin`` command.
They are detailed here for those needing to customize / troubleshoot their build process.

Building the Godot-CPP bindings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the Android C++ bindings using the following commands.

.. code-block:: shell

   cd thirdparty/godot-cpp
   scons platform=android target=template_debug arch=arm64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json
   scons platform=android target=template_release arch=arm64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json
   scons platform=android target=template_debug arch=x86_64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json
   scons platform=android target=template_release arch=x86_64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json

When the command is completed, you should have static libraries stored in ``thirdparty/godot-cpp/bin``
that will be used for compilation by the plugin.

Building the Plugin
^^^^^^^^^^^^^^^^^^^

Run the following command from the root directory to generate the editor gdextension plugin:

.. code-block:: shell

   scons target=template_debug custom_api_file=thirdparty/godot_cpp_gdextension_api/extension_api.json
   scons target=template_release custom_api_file=thirdparty/godot_cpp_gdextension_api/extension_api.json

Linux / MacOS
~~~~~~~~~~~~~

Run the following command from the root directory to build the vendors ``AAR`` Android binaries:

.. code-block:: shell

   ./gradlew build

Windows
~~~~~~~

Run the following command from the root directory to build the vendors ``AAR`` Android binaries:

.. code-block:: shell

   gradlew.bat build

Creating a new OpenXR extension wrapper
=======================================

If you want to create a new OpenXR extension wrapper for an extension that we don't support yet, you can generate the boilerplate header and source file using this script:

.. code-block:: bash

   # Interactive mode: it'll prompt you for the extension name.
   ./scripts/create_new_extension.py

   # CLI mode: provide the extension name as an argument.
   # For example:
   ./scripts/create_new_extension.py XR_ANDROID_light_estimation
   ./scripts/create_new_extension.py XR_FB_color_space

Then follow its instructions for the next steps. You can look at other extension wrappers as a reference.

Hooks
=====

When contributing to the source code for the plugin, we highly recommend you install clang-format and copy the contents of the ``hooks`` folder into the folder ``.git/hooks/``.
This will ensure clang-format is run on any changed files before committing the changes to github and prevent disappointment when formatting issues prevent changes from being merged.
