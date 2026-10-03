#version 450

layout(binding = 0) uniform sampler2D uTex;
layout(binding = 1) uniform sampler2DShadow uShadow;

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

vec4 blur(sampler2D tex, vec2 uv)
{
	vec4 c = vec4(0.0);
	for (int i = -2; i <= 2; i++)
		c += texture(tex, uv + vec2(float(i) * 0.01, 0.0));
	return c / 5.0;
}

float pcf(sampler2DShadow tex, vec3 uvz)
{
	return texture(tex, uvz);
}

void main()
{
	FragColor = blur(uTex, vUV) * pcf(uShadow, vec3(vUV, 0.25));
}
