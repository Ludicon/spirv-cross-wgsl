diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uTex : texture_2d<f32>;
@group(0) @binding(16) var uTex_sampler : sampler;
@group(0) @binding(1) var uShadow : texture_depth_2d;
@group(0) @binding(17) var uShadow_sampler : sampler_comparison;

var<private> FragColor : vec4f;
var<private> vUV : vec2f;

fn frag_main()
{
    var _122 : vec4f;
    _122 = vec4f(0.0f);
    for (var _121 : i32 = -2; _121 <= 2; )
    {
        _122 += textureSample(uTex, uTex_sampler, (vUV + vec2f(f32(_121) * 0.00999999977648258209228515625f, 0.0f)));
        _121++;
        continue;
    }
    FragColor = (_122 * vec4f(0.20000000298023223876953125f)) * textureSampleCompare(uShadow, uShadow_sampler, vec3f(vUV, 0.25f).xy, 0.25f);
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
