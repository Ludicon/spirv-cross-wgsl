diagnostic(off, derivative_uniformity);

struct UBO
{
    tint : vec4f,
}

struct SSBO
{
    values : array<vec4f>,
}

@group(0) @binding(4) var<uniform> _27 : UBO;
@group(1) @binding(2) var<storage, read> _47 : SSBO;
@group(0) @binding(0) var uAlbedo : texture_2d<f32>;
@group(0) @binding(5) var uAlbedo_sampler : sampler;
@group(0) @binding(2) var uNormal : texture_2d<f32>;
@group(0) @binding(3) var uNormal_sampler : sampler;
@group(1) @binding(0) var uSeparate : texture_2d<f32>;
@group(1) @binding(1) var uSeparateSampler : sampler;
@group(0) @binding(1) var uUnused : texture_2d<f32>;
@group(0) @binding(6) var uUnused_sampler : sampler;

var<private> FragColor : vec4f;
var<private> vUV : vec2f;

fn frag_main()
{
    FragColor = (textureSample(uAlbedo, uAlbedo_sampler, vUV) * textureSample(uNormal, uNormal_sampler, vUV)) * _27.tint;
    FragColor += (textureSample(uSeparate, uSeparateSampler, vUV) + _47.values[0]);
}

struct SPIRV_Cross_Input
{
    @location(0) vUV : vec2f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vUV = stage_input.vUV;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
