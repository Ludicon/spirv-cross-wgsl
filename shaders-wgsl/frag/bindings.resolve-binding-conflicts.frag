#version 450

// Combined image samplers, separate textures and samplers which share bindings, buffers,
// and an unused combined image sampler, matching tint's SPIR-V reader binding assignment.
layout(set = 0, binding = 0) uniform sampler2D uAlbedo;
layout(set = 0, binding = 1) uniform sampler2D uUnused;
layout(set = 0, binding = 2) uniform sampler2D uNormal;
layout(set = 0, binding = 3, std140) uniform UBO
{
	vec4 tint;
};
layout(set = 1, binding = 0) uniform texture2D uSeparate;
layout(set = 1, binding = 0) uniform sampler uSeparateSampler;
layout(set = 1, binding = 1, std430) readonly buffer SSBO
{
	vec4 values[];
};

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(uAlbedo, vUV) * texture(uNormal, vUV) * tint;
	FragColor += texture(sampler2D(uSeparate, uSeparateSampler), vUV) + values[0];
}
