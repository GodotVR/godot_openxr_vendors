shader_type canvas_item;

uniform int ITERATIONS = 1;

// A classic PRNG function
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void fragment() {
    // Generate a random float between 0.0 and 1.0 based on the pixel's UV
	int iterations = ITERATIONS;
	vec2 uv = UV;
	while (iterations > 0) {
		uv = vec2(rand(uv), rand(uv.yx));
    	float random_value = rand(UV);
		iterations--;
	}
    // Output a static noise pattern
    COLOR = vec4(vec3(uv.x), 0.001);
}
