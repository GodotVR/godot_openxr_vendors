Welcome to the Godot OpenXR Vendors plugin documentation!
=========================================================

The **Godot OpenXR Vendors plugin** is a GDExtension for Godot 4, that adds support for
OpenXR vendor extensions, which can't be made part of Godot itself, because they are too
specific to a particular vendor.

As just a handful of examples, this plugin adds support for Meta's scene anchors and HTC's
face tracking extension.

It also includes support for various Android OpenXR loaders. Until they can standardize on
the Khronos loader, a number of vendors need to use a proprietary OpenXR loader on Android.

.. Below is the main table-of-content tree of the documentation website.
   It is hidden on the page itself, but it makes up the sidebar for navigation.

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: API Reference
   :name: sec-class-reference

   api/index
