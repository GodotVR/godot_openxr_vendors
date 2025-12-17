extends Node3D

const WireframeMesh = preload("uid://co0a00gndugbg")

var _scene_meshing: OpenXRAndroidSceneMeshing
var _mesh_infos: Dictionary[StringName, MeshInfo]


class MeshInfo:
	var array_mesh: ArrayMesh
	var xform: Transform3D
	var mesh_instance: MeshInstance3D


func _ready():
	var scene_meshing_ext = Engine.get_singleton("OpenXRAndroidSceneMeshingExtension")
	var supported_semantic_label_sets = scene_meshing_ext.get_supported_semantic_label_sets()

	var semantic_label_set = OpenXRAndroidSceneMeshing.SEMANTIC_LABEL_SET_NONE
	if supported_semantic_label_sets.has(OpenXRAndroidSceneMeshing.SEMANTIC_LABEL_SET_DEFAULT):
		semantic_label_set = OpenXRAndroidSceneMeshing.SEMANTIC_LABEL_SET_DEFAULT

	_scene_meshing = OpenXRAndroidSceneMeshing.new()
	if not _scene_meshing.initialize(semantic_label_set, true):
		printerr("Failed to initialize OpenXRAndroidSceneMeshing")
		_scene_meshing = null


func _on_mesh_update_timer_timeout() -> void:
	if _scene_meshing == null:
		return

	var query_bounds_pose = get_tree().root.get_camera_3d().get_camera_transform().translated_local(Vector3.FORWARD)
	var query_bounds_extents = Vector3.ONE
	var submesh_datas: Dictionary = _scene_meshing.get_submesh_data(query_bounds_pose, query_bounds_extents)

	for uuid in submesh_datas.keys():
		var submesh_data: OpenXRAndroidSceneSubmeshData = submesh_datas[uuid]
		var update_state: OpenXRAndroidSceneSubmeshData.UpdateState = submesh_data.get_update_state()
		if update_state == OpenXRAndroidSceneSubmeshData.UPDATE_STATE_DELETED:
			# Sanity check: we should have seen this mesh before.
			if !_mesh_infos.has(uuid):
				printerr("This is the first time we've seen uuid [%s]; but its state is 'DELETED'" % uuid)
				continue

			# Use 'null' as the indication that the mesh should be deleted.
			_mesh_infos[uuid].array_mesh = null
			continue

		if update_state == OpenXRAndroidSceneSubmeshData.UPDATE_STATE_CREATED:
			if _mesh_infos.has(uuid):
				printerr("We've already seen uuid [%s]; but its state is 'CREATED'" % uuid)
			else:
				_mesh_infos[uuid] = MeshInfo.new()

		_mesh_infos[uuid].xform = submesh_data.get_transform()

		if update_state != OpenXRAndroidSceneSubmeshData.UPDATE_STATE_UNCHANGED:
			var array_mesh := ArrayMesh.new()
			array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, submesh_data.get_arrays())
			_mesh_infos[uuid].array_mesh = array_mesh

	for uuid in _mesh_infos.keys():
		var mesh_info: MeshInfo = _mesh_infos[uuid]
		if mesh_info.array_mesh == null:
			if mesh_info.mesh_instance == null:
				printerr("Deleting a MeshInfo which doesn't have a MeshInstance3D; how was it created?")
			else:
				mesh_info.mesh_instance.queue_free()

			# It's okay to modify the dictionary as we iterate over its keys.
			# https://docs.godotengine.org/en/stable/classes/class_dictionary.html#class-dictionary-method-erase
			_mesh_infos.erase(uuid)
			continue

		# Handle newly created meshes.
		if mesh_info.mesh_instance == null:
			mesh_info.mesh_instance = WireframeMesh.instantiate()
			add_child(mesh_info.mesh_instance)

		mesh_info.mesh_instance.transform = mesh_info.xform

		if mesh_info.mesh_instance.mesh != mesh_info.array_mesh:
			mesh_info.mesh_instance.mesh = mesh_info.array_mesh
