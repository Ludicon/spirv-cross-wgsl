@group(0) @binding(0) var ShadowMap : texture_depth_2d_array;
@group(0) @binding(1) var ShadowSamplerPCF : sampler_comparison;

var<private> texCoords : vec2f;
var<private> cascadeIndex : f32;
var<private> fragDepth : f32;
var<private> _entryPointOutput : vec4f;

fn _main(texCoords_1 : vec2f, cascadeIndex_1 : f32, fragDepth_1 : f32) -> vec4f
{
    let _39 = vec4f(vec3f(texCoords_1, cascadeIndex_1), fragDepth_1);
    let c = textureSampleCompareLevel(ShadowMap, ShadowSamplerPCF, _39.xy, i32(round(_39.z)), _39.w);
    return vec4f(c, c, c, c);
}

fn frag_main()
{
    let texCoords_1 = texCoords;
    let cascadeIndex_1 = cascadeIndex;
    let fragDepth_1 = fragDepth;
    let param = texCoords_1;
    let param_1 = cascadeIndex_1;
    let param_2 = fragDepth_1;
    _entryPointOutput = _main(param, param_1, param_2);
}

struct SPIRV_Cross_Input
{
    @location(0) texCoords : vec2f,
    @location(1) cascadeIndex : f32,
    @location(2) fragDepth : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) _entryPointOutput : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    texCoords = stage_input.texCoords;
    cascadeIndex = stage_input.cascadeIndex;
    fragDepth = stage_input.fragDepth;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._entryPointOutput = _entryPointOutput;
    return stage_output;
}
