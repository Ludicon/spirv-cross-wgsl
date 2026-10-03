var<private> FragColor : vec4f;
var<private> vIn : vec4f;

fn frag_main()
{
    FragColor = vIn;
}

struct SPIRV_Cross_Input
{
    @location(0) vIn : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vIn = stage_input.vIn;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
