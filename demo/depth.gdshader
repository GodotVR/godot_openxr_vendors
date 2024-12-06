shader_type spatial;

global uniform sampler2DArray environment_depth_texture;
global uniform mat4 environment_depth_view_projection_left;
global uniform mat4 environment_depth_view_projection_right;
global uniform mat4 environment_depth_projection_projection_left;
global uniform mat4 environment_depth_projection_projection_right;

void vertex() {
	// Called for every vertex the material is visible on.
}

void fragment() {
	// Called for every pixel the material is visible on.
	float depth = texture(environment_depth_texture, vec3(UV, float(VIEW_INDEX))).r;
	ALBEDO = vec3(depth);
	//ALBEDO = vec3(1, 1, 1);
}

//void light() {
	// Called for every pixel for every light affecting the material.
	// Uncomment to replace the default light processing function with this one.
//}