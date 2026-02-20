extends Object


## Call [method uprighted] on [param transform_3d], then offset by [param offset].[br]
## [param offset] is local to [param transform_3d].
static func upright_then_offset(transform_3d: Transform3D, offset: Vector3) -> Transform3D:
	return uprighted(transform_3d).translated_local(offset)


## Apply the offset to [param transform_3d], then make it upright.
static func offset_then_uprighted(transform_3d: Transform3D, offset: Vector3) -> Transform3D:
	return uprighted(transform_3d.translated_local(offset))


## Returns a copy of [param transform_3d], except its UP vector is equal to Vector3.UP
static func uprighted(transform_3d: Transform3D) -> Transform3D:
	# right = Vector3.UP.cross(transform_3d.basis.z).normalized()
	var right := Vector3(transform_3d.basis.z.z, 0.0, -transform_3d.basis.z.x).normalized()

	# forward = right.cross(Vector3.UP)
	var forward := Vector3(-right.z, 0.0, right.x)

	return Transform3D(right, Vector3.UP, forward, transform_3d.origin)
