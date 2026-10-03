diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uTexture : texture_2d<f32>;
@group(0) @binding(1) var uSampler : sampler;
@group(1) @binding(0) var uDepth : texture_depth_2d;
@group(1) @binding(1) var uShadowSampler : sampler_comparison;

var<private> FragColor : vec4f;
var<private> vUV : vec2f;

fn sample_texture(t : texture_2d<f32>, s : sampler, uv : vec2f) -> vec4f
{
    return textureSample(t, s, uv);
}

fn sample_shadow(t : texture_depth_2d, s : sampler_comparison, uvz : vec3f) -> f32
{
    return textureSampleCompare(t, s, uvz.xy, uvz.z);
}

fn frag_main()
{
    var param : vec2f = vUV;
    var param_1 : vec3f = vec3f(vUV, 0.5f);
    FragColor = sample_texture(uTexture, uSampler, param) * sample_shadow(uDepth, uShadowSampler, param_1);
    FragColor += textureLoad(uTexture, vec2i(vUV), 0);
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
