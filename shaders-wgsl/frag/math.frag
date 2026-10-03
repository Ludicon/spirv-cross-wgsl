#version 450

layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;
layout(location = 2) flat in ivec4 vI;
layout(location = 3) flat in uvec4 vU;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform UBO
{
	mat4 m;
	mat3 m3;
	mat2 m2;
};

void main()
{
	vec4 r = mod(vA, vB) + fract(vA) + inversesqrt(vA) + atan(vA, vB) + roundEven(vA);
	r += vec4(normalize(vA.x), reflect(vA.y, vB.y), refract(vA.z, vB.z, 0.5), faceforward(vA.w, vB.w, 1.0));
	r += vec4(lessThan(vA, vB)) + vec4(not(equal(vI, ivec4(vU)))) + vec4(isnan(vA)) + vec4(isinf(vB));
	r += vec4(mix(vA, vB, lessThan(vA, vB))) + mix(vA, vB, 0.25) + vec4(any(greaterThan(vA, vB)) ? 1.0 : 0.0);
	r += vec4(abs(vI) % 3) + vec4(vU / 2u) + vec4(sign(vI)) + vec4(min(vI, ivec4(2)) + max(vI, ivec4(-2)));
	r += vec4(clamp(vU, 1u, 4u)) + vec4(intBitsToFloat(vI) + uintBitsToFloat(vU));
	r += inverse(m) * vA + vec4(inverse(m3) * vA.xyz, 0.0) + vec4(inverse(m2) * vA.xy, 0.0, 0.0);
	r += vec4(determinant(m) + determinant(m3)) + transpose(m) * vB + outerProduct(vA, vB) * vA;
	vec4 whole;
	r += modf(vA, whole) + whole;
	ivec4 e;
	r += frexp(vB, e) + vec4(e) + ldexp(vA, ivec4(2));
	r += vec4(packSnorm4x8(vA) + packUnorm2x16(vB.xy)) + unpackUnorm4x8(vU.x) + vec4(unpackHalf2x16(vU.y), 0.0, 0.0);
	FragColor = r + fma(vA, vB, vA) + smoothstep(vA, vB, vec4(0.5)) + step(vA, vB) + pow(abs(vA), vB);
}
