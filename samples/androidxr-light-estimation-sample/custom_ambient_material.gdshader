shader_type spatial;

uniform vec3 albedo : source_color = vec3(0.0, 0.0, 0.0);
uniform vec3 coefficients[9];
uniform mat3 rotation;
uniform int sh_l = 2;

vec3 applySH(vec3 dir, vec3 sh[9]) {
	float x = dir.x;
	float y = dir.y;
	float z = dir.z;

	// SH basis constants.
	const float c0 = 0.2820947918;  // 1/(2*sqrt(pi))
	const float c1 = 0.4886025119;  // sqrt(3)/(2*sqrt(pi))
	const float c2 = 1.0925484306;  // sqrt(15)/(2*sqrt(pi))
	const float c3 = 0.3153915653;  // sqrt(5)/(4*sqrt(pi))
	const float c4 = 0.5462742153;  // sqrt(15)/(4*sqrt(pi))

	vec3 result = vec3(0.0);

	// L0
	if (sh_l >= 0) {
		result += sh[0] * c0;
	}

	// L1
	if (sh_l >= 1) {
		result += sh[1] * (c1 * y);
		result += sh[2] * (c1 * z);
		result += sh[3] * (c1 * x);
	}

	// L2
	if (sh_l >= 2) {
		result += sh[4] * (c2 * x * y);
		result += sh[5] * (c2 * y * z);
		result += sh[6] * (c3 * (3.0 * z * z - 1.0));
		result += sh[7] * (c2 * x * z);
		result += sh[8] * (c4 * (x * x - y * y));
	}

	return result;
}

void fragment() {
	ALBEDO = albedo;

	vec3 dir = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	dir = rotation * dir;

	vec3 e = applySH(dir, coefficients);
	e = max(e, vec3(0.0));
	EMISSION = e;
}
