shader_type spatial;
render_mode unshaded, blend_mix, fog_disabled;

// Shader code copied from:
//   samples/meta-scene-sample/assets/wireframe-material.gdshader
// Then modified to include semantic_color to set ALBEDO

uniform float wire_width : hint_range(0.0, 40.0) = 1.0;
uniform float wire_smoothness : hint_range(0.0, 0.1) = 0.01;

varying vec3 semantic_color;
varying vec3 barys;

void vertex() {
	int index = VERTEX_ID % 3;
	switch (index) {
		case 0:
			barys = vec3(1.0, 0.0, 0.0);
			break;
		case 1:
			barys = vec3(0.0, 1.0, 0.0);
			break;
		case 2:
			barys = vec3(0.0, 0.0, 1.0);
			break;
	}

	uint semantic_value = uint(CUSTOM0.x * 255.0);

	// Floor (red), ceiling (aqua), wall (blue), table (magenta), other (white)
	if (semantic_value == uint(1)) {
		semantic_color = vec3(0.94, 0.29, 0.29);
	} else if (semantic_value == uint(2)) {
		semantic_color = vec3(0.24, 0.94, 0.94);
	} else if (semantic_value == uint(3)) {
		semantic_color = vec3(0.0, 0.5, 0.78);
	} else if (semantic_value == uint(4)) {
		semantic_color = vec3(1.0, 0.0, 1.0);
	} else {
		semantic_color = vec3(1.0, 1.0, 1.0);
	}
}

void fragment() {
	vec3 deltas = fwidth(barys);
	vec3 barys_s = smoothstep(deltas * wire_width - wire_smoothness, deltas * wire_width + wire_smoothness, barys);
	float wires = min(barys_s.x, min(barys_s.y, barys_s.z));
	ALPHA = 1.0 - wires;

	ALBEDO = semantic_color;
}
