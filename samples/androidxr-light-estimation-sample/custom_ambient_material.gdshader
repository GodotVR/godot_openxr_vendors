shader_type spatial;

uniform vec3 albedo : source_color = vec3(0.0, 0.0, 0.0);
uniform vec3 coefficients[9];
uniform mat3 rotation;

// Copied from ARCore's hello_ar_kotlin sample.
// See: https://github.com/google-ar/arcore-android-sdk/blob/52c722e43cd8ce546eea5dc4587e70e0c7f2c006/samples/hello_ar_kotlin/app/src/main/assets/shaders/environmental_hdr.frag#L132
vec3 applySH(vec3 dir, vec3 sh[9]) {
	vec3 radiance = sh[0] +
					sh[1] * (dir.y) +
					sh[2] * (dir.z) +
					sh[3] * (dir.x) +
					sh[4] * (dir.y * dir.x) +
					sh[5] * (dir.y * dir.z) +
					sh[6] * (3.0 * dir.z * dir.z - 1.0) +
					sh[7] * (dir.z * dir.x) +
					sh[8] * (dir.x * dir.x - dir.y * dir.y);
	return max(radiance, 0.0);
}

void fragment() {
	ALBEDO = albedo;

	vec3 dir = -(INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	dir = rotation * dir;

	vec3 e = applySH(dir, coefficients);
	e = max(e, vec3(0.0));
	IRRADIANCE = vec4(e, 1.0);
}
