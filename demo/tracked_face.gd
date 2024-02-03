extends Node3D


# Dictionary of expressions to blend-shapes available on the face model
const _expressions := {
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_BROW_LOWERER_L : "browDownLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_BROW_LOWERER_R : "browDownRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_CHEEK_PUFF_L : "Cheek_Puff_Left",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_CHEEK_PUFF_R : "Cheek_Puff_Right",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_CHEEK_RAISER_L : "cheekSquintLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_CHEEK_RAISER_R : "cheekSquintRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_DIMPLER_L : "mouthDimpleLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_DIMPLER_R : "mouthDimpleRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_CLOSED_L : "eyeBlinkLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_CLOSED_R : "eyeBlinkRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_DOWN_L : "eyeLookDownLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_DOWN_R : "eyeLookDownRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_LEFT_L : "eyeLookOutLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_LEFT_R : "eyeLookInRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_RIGHT_L : "eyeLookInLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_RIGHT_R : "eyeLookOutRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_UP_L : "eyeLookUpLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_EYES_LOOK_UP_R : "eyeLookUpRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_JAW_DROP : "jawOpen",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_JAW_SIDEWAYS_LEFT : "jawLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_JAW_SIDEWAYS_RIGHT : "jawRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_JAW_THRUST : "jawForward",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_CORNER_DEPRESSOR_L : "mouthFrownLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_CORNER_DEPRESSOR_R : "mouthFrownRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_CORNER_PULLER_L : "mouthSmileLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_CORNER_PULLER_R : "mouthSmileRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_PRESSOR_L : "mouthPressLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_PRESSOR_R : "mouthPressRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_STRETCHER_L : "mouthStretchLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIP_STRETCHER_R : "mouthStretchRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LIPS_TOWARD : "mouthClose",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LOWER_LIP_DEPRESSOR_L : "mouthLowerDownLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_LOWER_LIP_DEPRESSOR_R : "mouthLowerDownRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_MOUTH_LEFT : "mouthLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_MOUTH_RIGHT : "mouthRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_NOSE_WRINKLER_L : "noseSneerLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_NOSE_WRINKLER_R : "noseSneerRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_OUTER_BROW_RAISER_L : "browOuterUpLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_OUTER_BROW_RAISER_R : "browOuterUpRight",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_UPPER_LIP_RAISER_L : "mouthUpperUpLeft",
	OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_UPPER_LIP_RAISER_R : "mouthUpperUpRight",
}


# Face mesh
@onready var face : MeshInstance3D = $Face/Face

# Blend-shapes by expression
var _blend_shapes : Array[int] = []


func _ready() -> void:
	# Build a table of blend-shapes by expression
	_blend_shapes.resize(OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_COUNT)
	_blend_shapes.fill(-1)
	for expression in OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_COUNT:
		# Skip if there is no mapping to blend-shape name
		if not _expressions.has(expression):
			continue

		# Find and save the blend-shape
		var blend_shape_name : String = _expressions[expression]
		var blend_shape := face.find_blend_shape_by_name(blend_shape_name)
		_blend_shapes[expression] = blend_shape


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta : float) -> void:
	# Get the weights
	var weights := OpenXRFbFaceTrackingExtensionWrapper.get_weights()
	if weights.is_empty():
		return

	# Set the blend-shapes to use the weights
	for expression in OpenXRFbFaceTrackingExtensionWrapper.EXPRESSION_COUNT:
		# Find the blend-shape
		var blend_shape := _blend_shapes[expression]
		if blend_shape < 0:
			continue

		# Set the blend-shape value
		face.set_blend_shape_value(blend_shape, weights[expression])
