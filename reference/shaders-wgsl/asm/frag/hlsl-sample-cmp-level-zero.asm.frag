@group(0) @binding(0) var ShadowMap : texture_depth_2d_array;
@group(0) @binding(1) var ShadowSamplerPCF : sampler_comparison;

var<private> texCoords : vec2f;
var<private> cascadeIndex : f32;
var<private> fragDepth : f32;
var<private> _entryPointOutput : vec4f;

fn _main(texCoords_1 : vec2f, cascadeIndex_1 : f32, fragDepth_1 : f32) -> vec4f
{
    var _39 : vec4f = vec4f(vec3f(texCoords_1, cascadeIndex_1), fragDepth_1);
    var c : f32 = textureSampleCompareLevel(ShadowMap, ShadowSamplerPCF, _39.xy, i32(round(_39.z)), _39.w);
    return vec4f(c, c, c, c);
}

fn frag_main()
{
    var texCoords_1 : vec2f = texCoords;
    var cascadeIndex_1 : f32 = cascadeIndex;
    var fragDepth_1 : f32 = fragDepth;
    var param : vec2f = texCoords_1;
    var param_1 : f32 = cascadeIndex_1;
    var param_2 : f32 = fragDepth_1;
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
