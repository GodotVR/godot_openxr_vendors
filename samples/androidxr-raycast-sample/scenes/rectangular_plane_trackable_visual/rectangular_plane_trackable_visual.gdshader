shader_type spatial;
render_mode unshaded;

uniform vec3 uColor : source_color = vec3(1.0);
uniform vec2 uExtents = vec2(1.0);

const float BORDER_WIDTH_IN_METERS = 0.003;

void fragment() {
	float notLeftEdge = step(BORDER_WIDTH_IN_METERS/uExtents.x, UV.x);
	float notTopEdge = step(BORDER_WIDTH_IN_METERS/uExtents.y, UV.y);
	float notRightEdge = step(BORDER_WIDTH_IN_METERS/uExtents.x, 1.0-UV.x);
	float notBottomEdge = step(BORDER_WIDTH_IN_METERS/uExtents.y, 1.0-UV.y);
	ALBEDO = uColor * notLeftEdge * notTopEdge * notRightEdge * notBottomEdge;
}
