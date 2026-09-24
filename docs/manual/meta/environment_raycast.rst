Meta Environment Raycast
========================

.. note::

    Check out the `Meta Environment Raycast Sample Project <https://github.com/GodotVR/godot_openxr_vendors/tree/master/samples/meta-raycast-sample>`_
    for a working demo of this feature.

The Meta Environment Raycast extension (``XR_META_environment_raycast``) casts a ray against the physical environment and returns the point where it hits a real world surface, together with the estimated surface normal.

It uses the same live depth data as :doc:`Meta Environment Depth <environment_depth>`, so it reacts immediately to objects that move, and it works without a scene capture. Typical uses are placing virtual objects on real tables, walls and floors, or pointing at the environment with a controller.

Project Settings
----------------

To use Meta Environment Raycast, the OpenXR extension must be enabled in project settings. The extension setting can be found in **Project Settings** under the **OpenXR** section. The **Environment Raycast** setting should be listed under **Extensions** in the **Meta** subcategory. Its project setting path is ``xr/openxr/extensions/meta/environment_raycast``.

When the setting is enabled, the ``com.oculus.permission.USE_SCENE`` permission is added to the Android manifest on export, and it is requested at startup (unless automatic permission requests are disabled in the project settings).

You will usually also want to enable **Passthrough**, so the player can see the environment they are pointing at.

Usage
-----

The raycaster is created automatically when the OpenXR session starts. Creation is asynchronous, so wait for the ``raycaster_ready`` signal (or check ``is_raycaster_ready()``) before casting rays:

.. code::

	func _ready() -> void:
		if not OpenXRMetaEnvironmentRaycastExtension.is_environment_raycast_supported():
			print("Environment raycast is not supported on this device")
			return

		OpenXRMetaEnvironmentRaycastExtension.raycaster_ready.connect(_on_raycaster_ready)
		OpenXRMetaEnvironmentRaycastExtension.raycaster_error.connect(_on_raycaster_error)


	func _on_raycaster_ready() -> void:
		print("Environment raycaster is ready")


	func _on_raycaster_error(error_code: int) -> void:
		push_error("Environment raycaster error: %d" % error_code)

Once the raycaster is ready, call ``raycast()`` with an origin, a direction and an optional maximum distance. A maximum distance of ``0.0`` or less disables the distance filter. The result is a dictionary:

.. code::

	func _process(_delta: float) -> void:
		if not OpenXRMetaEnvironmentRaycastExtension.is_raycaster_ready():
			return

		var origin: Vector3 = $XROrigin3D/RightController.position
		var direction: Vector3 = -$XROrigin3D/RightController.transform.basis.z

		var hit: Dictionary = OpenXRMetaEnvironmentRaycastExtension.raycast(origin, direction, 10.0)
		if hit.status == OpenXRMetaEnvironmentRaycastExtension.HIT_STATUS_HIT:
			print("Hit surface at ", hit.transform.origin, " with normal ", hit.transform.basis.z)

The dictionary contains the following keys:

.. table::
   :widths: auto

   +-------------------+-----------------+---------------------------------------------------------------------------------------+
   | ``status``        | ``int``         | One of the ``HitStatus`` values listed below. Only ``HIT_STATUS_HIT`` comes with a    |
   |                   |                 | usable pose.                                                                          |
   +-------------------+-----------------+---------------------------------------------------------------------------------------+
   | ``transform``     | ``Transform3D`` | The pose of the hit point, with its Z axis pointing along the surface normal. Only    |
   |                   |                 | valid when ``status`` is ``HIT_STATUS_HIT``.                                          |
   +-------------------+-----------------+---------------------------------------------------------------------------------------+

Hit status
----------

Only ``HIT_STATUS_HIT`` is a usable hit. The other statuses explain why no reliable hit point is available:

.. table::
   :widths: auto

   +--------------------------------------------+-------+-----------------------------------------------------------------------+
   | ``HIT_STATUS_HIT``                         | ``1`` | The ray intersected a physical surface.                               |
   +--------------------------------------------+-------+-----------------------------------------------------------------------+
   | ``HIT_STATUS_NO_HIT``                      | ``2`` | The ray did not intersect any surface within range.                   |
   +--------------------------------------------+-------+-----------------------------------------------------------------------+
   | ``HIT_STATUS_HIT_POINT_OCCLUDED``          | ``3`` | The hit point is occluded from the headset's point of view.           |
   +--------------------------------------------+-------+-----------------------------------------------------------------------+
   | ``HIT_STATUS_HIT_POINT_OUTSIDE_OF_FOV``    | ``4`` | The hit point is outside the field of view of the depth sensor.       |
   +--------------------------------------------+-------+-----------------------------------------------------------------------+
   | ``HIT_STATUS_RAY_OCCLUDED``                | ``5`` | The ray is occluded before reaching a surface.                        |
   +--------------------------------------------+-------+-----------------------------------------------------------------------+
   | ``HIT_STATUS_HIT_INVALID_ORIENTATION``     | ``6`` | A surface was hit, but its orientation could not be determined.       |
   +--------------------------------------------+-------+-----------------------------------------------------------------------+

Because the raycast relies on the depth sensor, rays pointing outside of the headset's view or at surfaces the sensor cannot see will not produce a hit, even if a surface is physically there.

Coordinate space
----------------

``raycast()`` takes and returns positions in the OpenXR play space, not in Godot world space. Play space is the coordinate space of the ``XROrigin3D`` node: it is the space the poses of ``XRCamera3D`` and ``XRController3D`` nodes are reported in before ``XROrigin3D`` (and any recentering via ``XRServer.center_on_hmd()``) is applied.

As long as ``XROrigin3D`` stays at the world origin and the view is never recentered, play space and world space are identical, and you can pass global positions directly. Otherwise, convert explicitly:

.. code::

	var play_to_world: Transform3D = $XROrigin3D.global_transform * XRServer.get_reference_frame()
	var world_to_play: Transform3D = play_to_world.affine_inverse()

	var origin: Vector3 = world_to_play * controller.global_position
	var direction: Vector3 = world_to_play.basis * -controller.global_transform.basis.z

	var hit: Dictionary = OpenXRMetaEnvironmentRaycastExtension.raycast(origin, direction, 10.0)
	if hit.status == OpenXRMetaEnvironmentRaycastExtension.HIT_STATUS_HIT:
		var hit_transform: Transform3D = play_to_world * hit.transform

This assumes the default world scale of ``1.0``.

Device support
--------------

Environment raycasting requires a depth sensor and is available on Meta Quest 3 and Meta Quest 3S. On other devices ``is_environment_raycast_supported()`` returns ``false``.

The Meta XR Simulator advertises the extension too, but it only provides the underlying depth data on Windows. On macOS every ray reports ``HIT_STATUS_NO_HIT``.
