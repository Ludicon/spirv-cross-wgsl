diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var Samp : texture_2d<f32>;
@group(0) @binding(16) var Samp_sampler : sampler;

var<private> vUV : vec2f;
var<private> FragColor : vec4f;
var<private> gl_FragCoord : vec4f;

fn frag_main()
{
    let a = textureSample(Samp, Samp_sampler, vUV);
    let b = textureSample(Samp, Samp_sampler, vUV);
    FragColor = select(b, a, (gl_FragCoord.x < 100.0f));
}

struct SPIRV_Cross_Input
{
    @location(0) vUV : vec2f,
    @builtin(position) gl_FragCoord : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vUV = stage_input.vUV;
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
