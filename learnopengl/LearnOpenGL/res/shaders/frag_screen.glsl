#version 450 core

in vec2 texCoords;

uniform sampler2D colorBuffer;
uniform sampler2D depthStencilBuffer;
uniform bool grayscale;

out vec4 fragColor;

float toLinear(float sRGB) {
	if (sRGB < 0.04045) {
		return sRGB / 12.92;
	} else {
		return pow((sRGB + 0.055) / 1.055, 2.4);		
	}
}

vec3 colorToLinear(vec3 sRGB) {
	return vec3(toLinear(sRGB.r), toLinear(sRGB.g), toLinear(sRGB.b));
}

float toSRGB(float linear) {
	if (linear <= 0.0031308) {
		return 12.92 * linear;
	} else {
		return 1.055 * pow(linear, 1.0 / 2.4) - 0.055;
	}
}

float getLuminance(vec3 linear) {
	return	0.2126 * linear.r + 0.7152 * linear.g + 0.0722 * linear.b;
}

float getDepthVal() {
	const float near = 0.01f;
	const float far = 100.f;
	float depthVal = texture(depthStencilBuffer, texCoords).r;
	depthVal = depthVal * 2.0 - 1.0; // back to NDC 
    depthVal = ((2.0 * near * far) / (far + near - depthVal * (far - near))) / far;

	return depthVal;
}

const float offset = 1.0 / 300.0;

const vec2 offsets[9] = vec2[](
	vec2(-offset,  offset), // top-left
	vec2( 0.0f,    offset), // top-center
	vec2( offset,  offset), // top-right
	vec2(-offset,  0.0f),   // center-left
	vec2( 0.0f,    0.0f),   // center-center
	vec2( offset,  0.0f),   // center-right
	vec2(-offset, -offset), // bottom-left
	vec2( 0.0f,   -offset), // bottom-center
	vec2( offset, -offset)  // bottom-right
);

float blur[9] = float[](
	1 / 16.0,  2 / 16.0,  1 / 16.0,
	2 / 16.0,  4 / 16.0,  2 / 16.0,
	1 / 16.0,  2 / 16.0,  1 / 16.0
);

float edge[9] = float[](
	1, 1, 1,
	1, -8, 1,
	1, 1, 1
);

float emptyKernel[9] = float[](
	0, 0, 0,
	0, 1, 0,
	0, 0, 0
);

void main() {
	vec3 color;

//	if (depthVal > 0.01) {
//		color = vec3(texture(colorBuffer, texCoords));
//	} else {
//		for (int i = 0; i < 9; ++i) {
//			color += vec3(texture(colorBuffer, texCoords.st + offsets[i])) * 1.0/9.0;//blur[i];
//		}
//	}

	color = vec3(texture(colorBuffer, texCoords));

	if (grayscale) {
		color = colorToLinear(color);
		float luminance = getLuminance(color);
		luminance = toSRGB(luminance);
		color = vec3(luminance);
	}

	fragColor = vec4(color, 1.f);
}