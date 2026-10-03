@group(0) @binding(0) var ShadowMap : texture_depth_2d_array;
@group(0) @binding(1) var ShadowSamplerPCF : sampler_comparison;

var<private> texCoords : vec2f;
var<private> cascadeIndex : f32;
var<private> fragDepth : f32;
var<private> _entryPointOutput : vec4f;

fn frag_main()
{
    _entryPointOutput = vec4f(textureSampleCompareLevel(ShadowMap, ShadowSamplerPCF, vec4f(texCoords, cascadeIndex, fragDepth).xy, i32(round(vec4f(texCoords, cascadeIndex, fragDepth).z)), fragDepth));
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
