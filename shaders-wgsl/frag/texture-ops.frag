#version 450

layout(binding = 0) uniform sampler2D uTex;
layout(binding = 1) uniform sampler2DArray uTexArray;
layout(binding = 2) uniform samplerCube uCube;
layout(binding = 3) uniform sampler3D uVolume;
layout(binding = 4) uniform sampler2DShadow uShadow;
layout(binding = 5) uniform sampler2DArrayShadow uShadowArray;
layout(binding = 6) uniform sampler2DMS uMSTex;
layout(binding = 7) uniform sampler1D uTex1D;

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vDir;
layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 c = texture(uTex, vUV);
	c += texture(uTex, vUV, 1.0);
	c += textureLod(uTex, vUV, 2.0);
	c += textureGrad(uTex, vUV, dFdx(vUV), dFdy(vUV));
	c += textureOffset(uTex, vUV, ivec2(1, -1));
	c += textureLodOffset(uTex, vUV, 1.0, ivec2(-2, 3));
	c += textureProj(uTex, vec3(vUV, 2.0));
	c += textureGather(uTex, vUV, 2);
	c += texelFetch(uTex, ivec2(vUV * 64.0), 1);
	c += texelFetchOffset(uTex, ivec2(vUV * 64.0), 0, ivec2(1, 1));
	c += texture(uTexArray, vec3(vUV, 2.0));
	c += textureLod(uTexArray, vec3(vUV, 1.0), 0.0);
	c += texture(uCube, vDir);
	c += textureLod(uCube, vDir, 3.0);
	c += texture(uVolume, vDir);
	c += vec4(texture(uShadow, vec3(vUV, 0.5)));
	c += vec4(textureLod(uShadow, vec3(vUV, 0.5), 0.0));
	c += vec4(texture(uShadowArray, vec4(vUV, 1.0, 0.5)));
	c += textureGather(uShadow, vUV, 0.5);
	c += texelFetch(uMSTex, ivec2(vUV * 64.0), 2);
	c += texture(uTex1D, vUV.x);
	ivec2 size = textureSize(uTex, 0) + textureSize(uTexArray, 1).xy + textureSize(uMSTex);
	int levels = textureQueryLevels(uTex) + textureSamples(uMSTex);
	FragColor = c + vec4(size, levels, 0.0);
}
