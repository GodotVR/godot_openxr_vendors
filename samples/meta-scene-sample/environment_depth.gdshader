shader_type spatial;

global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest, repeat_disable, hint_default_black;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT;

uniform vec3 albedo : source_color;

varying vec4 world_position;

void vertex() {
	world_position = MODEL_MATRIX * vec4(VERTEX, 1.0f);
}

bool is_occluded_by_environment(vec4 wp, int view_index) {
	highp vec4 depth_camera_position;
	if (view_index == int(0)) {
		depth_camera_position = META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT * wp;
	} else {
		depth_camera_position = META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT * wp;
	}

	highp vec2 depth_camera_position_hc = depth_camera_position.xy / depth_camera_position.w;
	depth_camera_position_hc = depth_camera_position_hc * 0.5f + 0.5f;

	highp vec3 depth_view_coord = vec3(depth_camera_position_hc, float(view_index));
	highp float depth_view_eye_z = texture(META_ENVIRONMENT_DEPTH_TEXTURE, depth_view_coord).r;

	highp float camera_depth = depth_camera_position.z / depth_camera_position.w;
	camera_depth = camera_depth * 0.5f + 0.5f;

	return depth_view_eye_z < camera_depth;
}

void fragment() {
	ALBEDO = albedo;
	ALPHA = is_occluded_by_environment(world_position, int(VIEW_INDEX)) ? 0.0 : 1.0;
}
