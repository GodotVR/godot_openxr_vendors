# GDExtension API

This directory contains the API JSON for
[**Godot Engine**](https://github.com/godotengine/godot)'s *GDExtensions* API.

## Current API version
- [commit fbbf9ec4efd8f1055d00edb8d926eef8ba4c2cce](https://github.com/godotengine/godot/commit/fbbf9ec4efd8f1055d00edb8d926eef8ba4c2cce)

## Updating API

The API JSON is synced with the latest version of Godot used when developing the plugin. Here is the
update procedure:

- Compile [Godot Engine](https://github.com/godotengine/godot) at the specific
  version/commit which you are using.
  * Or if you use an official release, download that version of the Godot editor.
- Use the compiled or downloaded executable to generate the `extension_api.json` file with:

```
godot --dump-extension-api
```
- Copy the generated `extension_api.json` file into this directory
