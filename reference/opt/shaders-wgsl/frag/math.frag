fn spvInverse2x2(m : mat2x2f) -> mat2x2f
{
    let det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    return mat2x2f(m[1][1], -m[0][1], -m[1][0], m[0][0]) * (1.0f / det);
}

fn spvInverse3x3(m : mat3x3f) -> mat3x3f
{
    let c0 = cross(m[1], m[2]);
    let c1 = cross(m[2], m[0]);
    let c2 = cross(m[0], m[1]);
    let det = dot(m[0], c0);
    return transpose(mat3x3f(c0, c1, c2)) * (1.0f / det);
}

fn spvInverse4x4(m : mat4x4f) -> mat4x4f
{
    let a00 = m[0][0]; let a01 = m[0][1]; let a02 = m[0][2]; let a03 = m[0][3];
    let a10 = m[1][0]; let a11 = m[1][1]; let a12 = m[1][2]; let a13 = m[1][3];
    let a20 = m[2][0]; let a21 = m[2][1]; let a22 = m[2][2]; let a23 = m[2][3];
    let a30 = m[3][0]; let a31 = m[3][1]; let a32 = m[3][2]; let a33 = m[3][3];
    let b00 = a00 * a11 - a01 * a10;
    let b01 = a00 * a12 - a02 * a10;
    let b02 = a00 * a13 - a03 * a10;
    let b03 = a01 * a12 - a02 * a11;
    let b04 = a01 * a13 - a03 * a11;
    let b05 = a02 * a13 - a03 * a12;
    let b06 = a20 * a31 - a21 * a30;
    let b07 = a20 * a32 - a22 * a30;
    let b08 = a20 * a33 - a23 * a30;
    let b09 = a21 * a32 - a22 * a31;
    let b10 = a21 * a33 - a23 * a31;
    let b11 = a22 * a33 - a23 * a32;
    let det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    return mat4x4f(
        a11 * b11 - a12 * b10 + a13 * b09, a02 * b10 - a01 * b11 - a03 * b09, a31 * b05 - a32 * b04 + a33 * b03, a22 * b04 - a21 * b05 - a23 * b03,
        a12 * b08 - a10 * b11 - a13 * b07, a00 * b11 - a02 * b08 + a03 * b07, a32 * b02 - a30 * b05 - a33 * b01, a20 * b05 - a22 * b02 + a23 * b01,
        a10 * b10 - a11 * b08 + a13 * b06, a01 * b08 - a00 * b10 - a03 * b06, a30 * b04 - a31 * b02 + a33 * b00, a21 * b02 - a20 * b04 - a23 * b00,
        a11 * b07 - a10 * b09 - a12 * b06, a00 * b09 - a01 * b07 + a02 * b06, a31 * b01 - a30 * b03 - a32 * b00, a20 * b03 - a21 * b01 + a22 * b00) * (1.0f / det);
}

struct UBO
{
    m : mat4x4f,
    m3 : mat3x3f,
    m2 : mat2x4f,
}

struct ResType
{
    _m0 : vec4f,
    _m1 : vec4f,
}

struct ResType_1
{
    _m0 : vec4f,
    _m1 : vec4i,
}

@group(0) @binding(0) var<uniform> _161 : UBO;

var<private> vA : vec4f;
var<private> vB : vec4f;
var<private> vI : vec4i;
var<private> vU : vec4u;
var<private> FragColor : vec4f;

fn frag_main()
{
    let _62 = (vA < vB);
    let _153 = (((((((((vA - vB * floor(vA / vB)) + fract(vA)) + inverseSqrt(vA)) + atan2(vA, vB)) + round(vA)) + vec4f(sign(vA.x), reflect(vec2f(vA.y, 0.0f), vec2f(vB.y, 0.0f)).x, refract(vec2f(vA.z, 0.0f), vec2f(vB.z, 0.0f), 0.5f).x, faceForward(vec2f(vA.w, 0.0f), vec2f(vB.w, 0.0f), vec2f(1.0f, 0.0f)).x)) + (((select(vec4f(0.0f), vec4f(1.0f), _62) + select(vec4f(0.0f), vec4f(1.0f), vI != bitcast<vec4i>(vU))) + select(vec4f(0.0f), vec4f(1.0f), (bitcast<vec4u>(vA) & vec4u(0x7fffffffu)) > vec4u(0x7f800000u))) + select(vec4f(0.0f), vec4f(1.0f), (bitcast<vec4u>(vB) & vec4u(0x7fffffffu)) == vec4u(0x7f800000u)))) + ((select(vA, vB, _62) + mix(vA, vB, vec4f(0.25f))) + vec4f(select(0.0f, 1.0f, any(vA > vB))))) + (((vec4f(abs(vI) % vec4i(3)) + vec4f(vU / vec4u(2u))) + vec4f(sign(vI))) + vec4f(min(vI, vec4i(2)) + max(vI, vec4i(-2))))) + (vec4f(clamp(vU, vec4u(1u), vec4u(4u))) + (bitcast<vec4f>(vI) + bitcast<vec4f>(vU)));
    let _220_modf = modf(vA);
    let _220 = ResType(_220_modf.fract, _220_modf.whole);
    let _231_frexp = frexp(vB);
    let _231 = ResType_1(_231_frexp.fract, _231_frexp.exp);
    let _271 = (((((_153 + (((spvInverse4x4(_161.m) * vA) + vec4f(spvInverse3x3(_161.m3) * vA.xyz, 0.0f)) + vec4f(spvInverse2x2(mat2x2f(_161.m2[0].xy, _161.m2[1].xy)) * vA.xy, 0.0f, 0.0f))) + ((vec4f(determinant(_161.m) + determinant(_161.m3)) + (transpose(_161.m) * vB)) + (mat4x4f(vA * vB.x, vA * vB.y, vA * vB.z, vA * vB.w) * vA))) + (_220._m0 + _220._m1)) + ((_231._m0 + vec4f(_231._m1)) + ldexp(vA, vec4i(2)))) + ((vec4f(f32(pack4x8snorm(vA) + pack2x16unorm(vB.xy))) + unpack4x8unorm(vU.x)) + vec4f(unpack2x16float(vU.y), 0.0f, 0.0f))) + fma(vA, vB, vA);
    FragColor = ((_271 + smoothstep(vA, vB, vec4f(0.5f))) + step(vA, vB)) + pow(abs(vA), vB);
}

struct SPIRV_Cross_Input
{
    @location(0) vA : vec4f,
    @location(1) vB : vec4f,
    @location(2) @interpolate(flat) vI : vec4i,
    @location(3) @interpolate(flat) vU : vec4u,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    vI = stage_input.vI;
    vU = stage_input.vU;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
