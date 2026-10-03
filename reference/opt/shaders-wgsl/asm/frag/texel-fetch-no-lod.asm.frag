@group(0) @binding(0) var uTexture : texture_2d<f32>;
@group(0) @binding(16) var uTexture_sampler : sampler;

var<private> FragColor : vec4f;
var<private> gl_FragCoord : vec4f;

fn frag_main()
{
    FragColor = textureLoad(uTexture, vec2i(gl_FragCoord.xy), 0);
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
