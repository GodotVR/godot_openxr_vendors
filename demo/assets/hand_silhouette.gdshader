shader_type spatial;

render_mode unshaded, cull_disabled;
uniform vec3 silhouette_color : source_color = vec3(0.071, 0.068, 0.104);
uniform float max_opacity : hint_range(0.0, 1.0, 0.01) = 0.95;
uniform float fade_offset : hint_range(0.0, 0.1, 0.001) = 0.055;
uniform float fade_sharpness : hint_range(0.0, 35.0, 0.01) = 28.00;

varying vec3 vertex_pos;

void vertex() {
	vertex_pos = VERTEX;
}

void fragment() {
	ALBEDO = silhouette_color;
	ALPHA = (clamp((vertex_pos.y + fade_offset) * fade_sharpness, 0.00, max_opacity));
}
