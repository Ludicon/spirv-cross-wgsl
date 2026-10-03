@group(0) @binding(0) var pointLightShadowMap : texture_depth_cube;
@group(0) @binding(1) var shadowSamplerPCF : sampler_comparison;

var<private> _entryPointOutput : f32;

fn _main() -> f32
{
    let _29 = vec4f(0.100000001490116119384765625f, 0.100000001490116119384765625f, 0.100000001490116119384765625f, 0.5f);
    return textureSampleCompareLevel(pointLightShadowMap, shadowSamplerPCF, _29.xyz, _29.w);
}

fn frag_main()
{
    _entryPointOutput = _main();
}

struct SPIRV_Cross_Output
{
    @location(0) _entryPointOutput : f32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._entryPointOutput = _entryPointOutput;
    return stage_output;
}
