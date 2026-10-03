diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uTex : texture_2d<f32>;
@group(0) @binding(16) var uTex_sampler : sampler;
@group(0) @binding(1) var uTexArray : texture_2d_array<f32>;
@group(0) @binding(17) var uTexArray_sampler : sampler;
@group(0) @binding(2) var uCube : texture_cube<f32>;
@group(0) @binding(18) var uCube_sampler : sampler;
@group(0) @binding(3) var uVolume : texture_3d<f32>;
@group(0) @binding(19) var uVolume_sampler : sampler;
@group(0) @binding(4) var uShadow : texture_depth_2d;
@group(0) @binding(20) var uShadow_sampler : sampler_comparison;
@group(0) @binding(5) var uShadowArray : texture_depth_2d_array;
@group(0) @binding(21) var uShadowArray_sampler : sampler_comparison;
@group(0) @binding(6) var uMSTex : texture_multisampled_2d<f32>;
@group(0) @binding(22) var uMSTex_sampler : sampler;
@group(0) @binding(7) var uTex1D : texture_1d<f32>;
@group(0) @binding(23) var uTex1D_sampler : sampler;

var<private> vUV : vec2f;
var<private> vDir : vec3f;
var<private> FragColor : vec4f;

fn frag_main()
{
    var _64 : vec3f = vec3f(vUV, 2.0f);
    var _78 : vec2i = vec2i(vUV * 64.0f);
    var _149 : vec3f = vec3f(vUV, 0.5f);
    FragColor = ((((((((((((((((((((textureSample(uTex, uTex_sampler, vUV) + textureSampleBias(uTex, uTex_sampler, vUV, 1.0f)) + textureSampleLevel(uTex, uTex_sampler, vUV, 2.0f)) + textureSampleGrad(uTex, uTex_sampler, vUV, dpdx(vUV), dpdy(vUV))) + textureSample(uTex, uTex_sampler, vUV, vec2i(1, -1))) + textureSampleLevel(uTex, uTex_sampler, vUV, 1.0f, vec2i(-2, 3))) + textureSample(uTex, uTex_sampler, _64.xy / _64.z)) + textureGather(2, uTex, uTex_sampler, vUV)) + textureLoad(uTex, _78, 1)) + textureLoad(uTex, _78 + vec2i(1), 0)) + textureSample(uTexArray, uTexArray_sampler, _64.xy, i32(round(_64.z)))) + textureSampleLevel(uTexArray, uTexArray_sampler, vec3f(vUV, 1.0f).xy, i32(round(vec3f(vUV, 1.0f).z)), 0.0f)) + textureSample(uCube, uCube_sampler, vDir)) + textureSampleLevel(uCube, uCube_sampler, vDir, 3.0f)) + textureSample(uVolume, uVolume_sampler, vDir)) + vec4f(textureSampleCompare(uShadow, uShadow_sampler, _149.xy, 0.5f))) + vec4f(textureSampleCompareLevel(uShadow, uShadow_sampler, _149.xy, 0.5f))) + vec4f(textureSampleCompare(uShadowArray, uShadowArray_sampler, vec4f(vUV, 1.0f, 0.5f).xy, i32(round(vec4f(vUV, 1.0f, 0.5f).z)), 0.5f))) + textureGatherCompare(uShadow, uShadow_sampler, vUV, 0.5f)) + textureLoad(uMSTex, _78, 2)) + textureSample(uTex1D, uTex1D_sampler, vUV.x)) + vec4f(vec2f((vec2i(textureDimensions(uTex, 0)) + vec3i(vec2i(textureDimensions(uTexArray, 1)), i32(textureNumLayers(uTexArray))).xy) + vec2i(textureDimensions(uMSTex))), f32(i32(textureNumLevels(uTex)) + i32(textureNumSamples(uMSTex))), 0.0f);
}

struct SPIRV_Cross_Input
{
    @location(0) vUV : vec2f,
    @location(1) vDir : vec3f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vUV = stage_input.vUV;
    vDir = stage_input.vDir;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
