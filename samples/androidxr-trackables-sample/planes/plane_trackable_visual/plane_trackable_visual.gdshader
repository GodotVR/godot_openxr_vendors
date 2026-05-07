shader_type spatial;
render_mode depth_draw_opaque, cull_back, unshaded;

uniform vec3 color: source_color = vec3(0.0);

varying vec3 PSWorldPosition;

// The normalized percentage of the dot's edge we want to blur. 1.0 would be everything is
// blurred (linear gradient from center of dot to edge) while 0 would be nothing is blurred.
const float dotAntiAliasAmount = 0.15;

const float dotRadiusMeters = 0.0072;
const float dotSpacingMeters = 0.065;

void vertex() {
	PSWorldPosition = VERTEX;
}

void fragment() {
	// -- Creating the dot pattern --

	// In this context a "cell" is a rectangle that defines how much space one dot takes up.
	// This determines the spacing between dots (e.g. if the worldCellSize is 0.05 meters then
	// we would have
	//    [ · ][ · ][ · ][ · ]
	// [ · ][ · ][ · ][ · ][ · ])
	// With the width and height of each [ · ] being 0.05 meters which would make the spacing
	// between · horizontally 0.05 meters and the diagonal ones sqrt(2)*0.05 meters.
	// Never allow dots to cross over each other
	float worldCellSize = max(dotSpacingMeters, dotRadiusMeters * 2.0);

	vec2 worldPosGrid = PSWorldPosition.xz / worldCellSize;
	vec2 worldPosOffset = fract(worldPosGrid);

	// Calculate the row and column index of the current cell we are in.
	float rowIndex = floor(worldPosGrid.y);
	float colIndex = floor(worldPosGrid.x);

	// Offset every other row
	if (abs(int(rowIndex)) % 2 == 1) {
		worldPosOffset.x = fract(worldPosOffset.x + 0.5);
		colIndex = floor((PSWorldPosition.x + 0.5 * worldCellSize) / worldCellSize);
	}

	// This is the distance (in meters) from the fragment to the center of the dot in it's cell.
	float distFromDotCenterMeters = length(worldPosOffset - vec2(0.5)) * worldCellSize;
	// A signed distance with negative being inside the dot and positive outside the dot
	float fillDist = abs(distFromDotCenterMeters) - dotRadiusMeters;
	// The distance in meters we want to "feather" the outside edge a bit. This means we have a linear soft edge from -antialiasDist to +antialiastDist
	float antialiasDist = dotRadiusMeters * dotAntiAliasAmount;
	// The alpha of the fragment relative to the dot. Should be 0 outside of the dot, 1 anywhere
	// inside and a linear interpolation between the two for (-antialiasDist, antialiasDist) meters on the edge of the dot.
	float dotSDF = 1.0f - smoothstep(-antialiasDist, antialiasDist, fillDist);

	vec4 fullColor = mix(vec4(color.xyz + vec3(0.2, 0.2, 0.2), 0.1), vec4(color.xyz, 0.4), dotSDF);
	ALBEDO = fullColor.xyz;
	ALPHA = fullColor.w;
}
