@group(0) @binding(0) var uSampler : texture_multisampled_2d<f32>;
@group(0) @binding(16) var uSampler_sampler : sampler;

var<private> gl_FragCoord : vec4f;
var<private> FragColor : vec4f;

fn frag_main()
{
    let _17 = vec2i(gl_FragCoord.xy);
    FragColor = ((textureLoad(uSampler, _17, 0) + textureLoad(uSampler, _17, 1)) + textureLoad(uSampler, _17, 2)) + textureLoad(uSampler, _17, 3);
}

struct SPIRV_Cross_Input
{
    @builtin(position) gl_FragCoord : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
