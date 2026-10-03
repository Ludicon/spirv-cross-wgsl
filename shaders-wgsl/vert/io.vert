#version 450

struct Material
{
	vec4 color;
	float roughness;
};

layout(location = 0) in vec4 aPosition;
layout(location = 1) in mat4 aInstanceMatrix;
layout(location = 5) in ivec2 aIndices;
layout(location = 6) in uvec4 aBones;
layout(location = 7) in vec2 aUVs[2];

layout(location = 0) out vec4 vColor;
layout(location = 1) flat out int vIndex;
layout(location = 2) out vec2 vUVs[2];
layout(location = 4) noperspective out float vLinear;
layout(location = 5) centroid out vec3 vCentroid;
layout(location = 6) out Material vMaterial;
layout(location = 8) out VertexOut
{
	vec3 normal;
	flat uint bone;
} vOut;

layout(push_constant) uniform Push
{
	mat4 mvp;
	vec4 tint;
} push;

void main()
{
	gl_Position = push.mvp * aInstanceMatrix * aPosition;
	vColor = push.tint * float(gl_VertexIndex) + float(gl_InstanceIndex);
	vIndex = aIndices.x + aIndices.y;
	vUVs = aUVs;
	vLinear = aPosition.w;
	vCentroid = aPosition.xyz;
	vMaterial.color = push.tint;
	vMaterial.roughness = 0.5;
	vOut.normal = aInstanceMatrix[2].xyz;
	vOut.bone = aBones.x + aBones.w;
}
