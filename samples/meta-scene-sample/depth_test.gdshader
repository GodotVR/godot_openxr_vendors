shader_type spatial;

global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_VIEW_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_VIEW_RIGHT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT;

uniform vec3 albedo : source_color;

varying vec4 cube_world_position;

void vertex() {
	cube_world_position = MODEL_MATRIX * vec4(VERTEX, 1.0f);
}

void fragment() {
	highp vec4 cube_depth_camera_position;
	if (uint(VIEW_INDEX) == uint(0)) {
		cube_depth_camera_position = META_ENVIRONMENT_DEPTH_PROJECTION_LEFT * META_ENVIRONMENT_DEPTH_VIEW_LEFT * cube_world_position;
	} else {
		cube_depth_camera_position = META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT * META_ENVIRONMENT_DEPTH_VIEW_RIGHT * cube_world_position;
	}

	highp vec2 cube_depth_camera_position_hc = cube_depth_camera_position.xy / cube_depth_camera_position.w;
	cube_depth_camera_position_hc = cube_depth_camera_position_hc * 0.5f + 0.5f;

	highp vec3 depth_view_coord = vec3(cube_depth_camera_position_hc, float(VIEW_INDEX));
	highp float depth_view_eye_z = texture(META_ENVIRONMENT_DEPTH_TEXTURE, depth_view_coord).r;

	highp float cube_depth = cube_depth_camera_position.z / cube_depth_camera_position.w;
	cube_depth = cube_depth * 0.5f + 0.5f;

	DEPTH = cube_depth;

	if (cube_depth < depth_view_eye_z) {
		ALBEDO = albedo;
		ALPHA = 1.0;
	} else {
		ALBEDO = vec3(0.0, 0.0, 0.0);
		ALPHA = 0.0;
	}
}
