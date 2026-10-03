var<private> FragColor : vec4f;
var<private> vA : vec4f;
var<private> vB : f32;

fn frag_main()
{
    FragColor = round(vA);
    FragColor *= round(vB);
}

struct SPIRV_Cross_Input
{
    @location(0) vA : vec4f,
    @location(1) vB : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
