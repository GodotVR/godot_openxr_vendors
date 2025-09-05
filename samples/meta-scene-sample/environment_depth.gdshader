shader_type spatial;

global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest, repeat_disable, hint_default_black;
global uniform highp vec2 META_ENVIRONMENT_DEPTH_TEXEL_SIZE;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT;

uniform vec3 albedo : source_color;

varying vec4 world_position;

// 0 = Hard occlusion
// 1 = Soft occlusion via bilinear filtering
// 2 = Soft occlusion via PCF 3x3
#define OCCLUSION_MODE 2

#define PCF_TEXEL_RADIUS 1.0

void vertex() {
	world_position = MODEL_MATRIX * vec4(VERTEX, 1.0f);
}

float get_bilinear_environment_depth(vec2 uv, uint view_index) {
	vec2 p = uv / META_ENVIRONMENT_DEPTH_TEXEL_SIZE;
	vec2 f = fract(p);
	vec2 i = floor(p);

	vec2 uv00 = (i + vec2(0.5, 0.5)) * META_ENVIRONMENT_DEPTH_TEXEL_SIZE;
	vec2 uv10 = uv00 + vec2(META_ENVIRONMENT_DEPTH_TEXEL_SIZE.x, 0.0);
	vec2 uv01 = uv00 + vec2(0.0, META_ENVIRONMENT_DEPTH_TEXEL_SIZE.y);
	vec2 uv11 = uv00 + META_ENVIRONMENT_DEPTH_TEXEL_SIZE;

	float d00 = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(uv00, float(view_index))).r;
	float d10 = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(uv10, float(view_index))).r;
	float d01 = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(uv01, float(view_index))).r;
	float d11 = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(uv11, float(view_index))).r;

	return mix(mix(d00, d10, f.x), mix(d01, d11, f.x), f.y);
}

float get_hard_environment_alpha(vec2 uv, uint view_index, float camera_depth) {
	highp float depth_view_eye_z = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(uv, float(view_index))).r;
	return depth_view_eye_z < camera_depth ? 0.0 : 1.0;
}

float smooth_compare(float d, float camera_depth) {
	float delta = d - camera_depth;
	float w = max(fwidth(delta), 1e-5);

	return smoothstep(-w, w, delta);
}

float get_smooth_environment_alpha(vec2 uv, uint view_index, float camera_depth) {
	float d = get_bilinear_environment_depth(uv, view_index);
	return smooth_compare(d, camera_depth);
}

float get_smooth_pcf3x3_environment_alpha(vec2 uv, uint view_index, float camera_depth) {
	vec2 r = PCF_TEXEL_RADIUS * META_ENVIRONMENT_DEPTH_TEXEL_SIZE;

	float w[3] = float[3](1.0, 2.0, 1.0);
	float norm = 0.0;
	float a = 0.0;

	for (int j = -1; j <= 1; j++) {
		for (int i = -1; i <= 1; i++) {
			float w2 = w[i+1] * w[j+1];
			float d = get_bilinear_environment_depth(uv + vec2(float(i), float(j)) * r, view_index);
			float s = smooth_compare(d, camera_depth);
			a += w2 * s;
			norm += w2;
		}
	}
	return a / norm;
}

float calculate_environment_occlusion(vec4 wp, uint view_index) {
	highp vec4 depth_camera_position;
	if (view_index == uint(0)) {
		depth_camera_position = META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT * wp;
	} else {
		depth_camera_position = META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT * wp;
	}

	highp vec2 depth_camera_position_hc = depth_camera_position.xy / depth_camera_position.w;
	depth_camera_position_hc = depth_camera_position_hc * 0.5f + 0.5f;

	highp float camera_depth = depth_camera_position.z / depth_camera_position.w;
	camera_depth = camera_depth * 0.5f + 0.5f;

#if OCCLUSION_MODE == 1
	return get_smooth_environment_alpha(depth_camera_position_hc, uint(view_index), camera_depth);
#elif OCCLUSION_MODE == 2
	return get_smooth_pcf3x3_environment_alpha(depth_camera_position_hc, uint(view_index), camera_depth);
#else
	return get_hard_environment_alpha(depth_camera_position_hc, uint(view_index), camera_depth);
#endif
}

void fragment() {
	ALBEDO = albedo;
	ALPHA = calculate_environment_occlusion(world_position, uint(VIEW_INDEX));
}
