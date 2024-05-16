shader_type spatial;
render_mode unshaded;

const vec3 RED = vec3(1.0, 0.0, 0.0);
const vec3 GREEN = vec3(0.0, 1.0, 0.0);

uniform float value: hint_range(0.0, 1.0) = 0.0;

void fragment() {
	ALBEDO = mix(GREEN, RED, step(value, UV.y));
}
