extends Node3D

const MESH_INSTANCE_3D = preload("res://meshes/mesh/mesh.tscn")

var _scene_meshing: OpenXRAndroidSceneMeshing

var _thread: Thread
var _semaphore: Semaphore
var _mutex: Mutex
var _query_bounds_pose: Transform3D
var _query_bounds_extents: Vector3

# map uuid (StringName) -> MeshInfos
var _mesh_infos: Dictionary


class MeshInfo:
	var array_mesh: ArrayMesh
	var xform: Transform3D
	var mesh_instance: MeshInstance3D


func _ready():
	# TODO when get_supported_semantic_label_sets() returns valid data, query
	# get_supported_semantic_label_sets() and choose SEMANTIC_LABEL_SET_NONE when
	# SEMANTIC_LABEL_SET_DEFAULT is not found

	_scene_meshing = OpenXRAndroidSceneMeshing.new()
	if _scene_meshing.initialize(OpenXRAndroidSceneMeshing.SEMANTIC_LABEL_SET_DEFAULT, true):
		_semaphore = Semaphore.new()
		_thread = Thread.new()
		_mutex = Mutex.new()
		_thread.start(_update_mesh_thread)
	else:
		printerr("Failed to initialize OpenXRAndroidSceneMeshing")


func _notification(what):
	if what == NOTIFICATION_PREDELETE:
		_semaphore.post()

		# wait until _update_mesh_thread is done working
		_mutex.lock()

		# we have the lock; _update_mesh_thread will fail its next try_lock() and should exit
		_thread.wait_to_finish()

		# not necessary
		_mutex.unlock()


func _update_mesh_thread():
	while _mutex.try_lock():
		_update_mesh.call_deferred()
		_semaphore.wait()
		_mutex.unlock()

		var current_submesh_datas: Dictionary = _scene_meshing.get_submesh_data(_query_bounds_pose, _query_bounds_extents)

		for uuid in current_submesh_datas.keys():
			var submesh_data: OpenXRAndroidSceneSubmeshData = current_submesh_datas[uuid]
			var update_state: OpenXRAndroidSceneSubmeshData.UpdateState = submesh_data.get_update_state()
			if update_state == OpenXRAndroidSceneSubmeshData.UPDATE_STATE_DELETED:
				# sanity; we should have seen this mesh before
				if !_mesh_infos.has(uuid):
					printerr("This is the first time we've seen uuid [%s]; but its state is 'DELETED'" % uuid)
					continue

				# use 'null' as the indication that the mesh should be deleted in the main thread
				_mesh_infos[uuid].array_mesh = null
				continue

			if update_state == OpenXRAndroidSceneSubmeshData.UPDATE_STATE_CREATED:
				# sanity; we should have not seen this mesh before
				if _mesh_infos.has(uuid):
					printerr("We've already seen uuid [%s]; but its state is 'CREATED'" % uuid)
					# still set its data though
				else:
					_mesh_infos[uuid] = MeshInfo.new()

			_mesh_infos[uuid].xform = submesh_data.get_transform()

			if update_state != OpenXRAndroidSceneSubmeshData.UPDATE_STATE_UNCHANGED:
				var array_mesh := ArrayMesh.new()
				array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, submesh_data.get_arrays())
				_mesh_infos[uuid].array_mesh = array_mesh


func _update_mesh():
	for uuid in _mesh_infos.keys():
		var mesh_info: MeshInfo = _mesh_infos[uuid]
		if mesh_info.array_mesh == null:
			# sanity
			if mesh_info.mesh_instance == null:
				printerr("Deleting a MeshInfo which doesn't have a MeshInstance3D; how was it created?")
			else:
				mesh_info.mesh_instance.queue_free()

			# it's okay to modify the dictionary as we iterate over its keys
			# https://docs.godotengine.org/en/stable/classes/class_dictionary.html#class-dictionary-method-erase
			_mesh_infos.erase(uuid)
			continue

		# handle newly created meshes
		if mesh_info.mesh_instance == null:
			mesh_info.mesh_instance = MESH_INSTANCE_3D.instantiate()
			add_child(mesh_info.mesh_instance)

		mesh_info.mesh_instance.global_transform = mesh_info.xform

		if mesh_info.mesh_instance.mesh != mesh_info.array_mesh:
			mesh_info.mesh_instance.mesh = mesh_info.array_mesh

	# update pose and extents for next meshes
	_query_bounds_pose = get_tree().root.get_camera_3d().get_camera_transform().translated_local(Vector3(0, 0, -1))
	_query_bounds_extents = Vector3(1.0, 1.0, 1.0)

	_semaphore.post()
