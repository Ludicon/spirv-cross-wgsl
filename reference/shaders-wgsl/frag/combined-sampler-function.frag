diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uTex : texture_2d<f32>;
@group(0) @binding(16) var uTex_sampler : sampler;
@group(0) @binding(1) var uShadow : texture_depth_2d;
@group(0) @binding(17) var uShadow_sampler : sampler_comparison;

var<private> FragColor : vec4f;
var<private> vUV : vec2f;

fn blur(tex : texture_2d<f32>, tex_sampler : sampler, uv : vec2f) -> vec4f
{
    var c = vec4f(0.0f);
    for (var i = -2; i <= 2; i++)
    {
        c += textureSample(tex, tex_sampler, (uv + vec2f(f32(i) * 0.00999999977648258209228515625f, 0.0f)));
    }
    return c / vec4f(5.0f);
}

fn pcf(tex : texture_depth_2d, tex_sampler : sampler_comparison, uvz : vec3f) -> f32
{
    return textureSampleCompare(tex, tex_sampler, uvz.xy, uvz.z);
}

fn frag_main()
{
    let param = vUV;
    let param_1 = vec3f(vUV, 0.25f);
    FragColor = blur(uTex, uTex_sampler, param) * pcf(uShadow, uShadow_sampler, param_1);
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
