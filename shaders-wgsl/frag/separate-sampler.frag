#version 450
#extension GL_EXT_samplerless_texture_functions : require

layout(set = 0, binding = 0) uniform texture2D uTexture;
layout(set = 0, binding = 1) uniform sampler uSampler;
layout(set = 1, binding = 0) uniform texture2D uDepth;
layout(set = 1, binding = 1) uniform samplerShadow uShadowSampler;

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

vec4 sample_texture(texture2D t, sampler s, vec2 uv)
{
	return texture(sampler2D(t, s), uv);
}

float sample_shadow(texture2D t, samplerShadow s, vec3 uvz)
{
	return texture(sampler2DShadow(t, s), uvz);
}

void main()
{
	FragColor = sample_texture(uTexture, uSampler, vUV) * sample_shadow(uDepth, uShadowSampler, vec3(vUV, 0.5));
	FragColor += texelFetch(uTexture, ivec2(vUV), 0);
}
