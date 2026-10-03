var<private> FragColor : vec4f;
var<private> vA : vec4i;
var<private> vB : vec4i;

fn frag_main()
{
    FragColor = vec4f(vA - vB * (vA / vB));
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) vA : vec4i,
    @location(1) @interpolate(flat) vB : vec4i,
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
