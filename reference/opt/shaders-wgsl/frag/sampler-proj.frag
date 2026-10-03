diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uTex : texture_2d<f32>;
@group(0) @binding(16) var uTex_sampler : sampler;

var<private> FragColor : vec4f;
var<private> vTex : vec4f;

fn frag_main()
{
    var _17 = vTex;
    _17.z = _17.w;
    FragColor = textureSample(uTex, uTex_sampler, _17.xy / _17.z);
}

struct SPIRV_Cross_Input
{
    @location(0) vTex : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vTex = stage_input.vTex;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
