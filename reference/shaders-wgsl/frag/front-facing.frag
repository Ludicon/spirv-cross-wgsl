var<private> gl_FrontFacing : bool;
var<private> FragColor : vec4f;
var<private> vA : vec4f;
var<private> vB : vec4f;

fn frag_main()
{
    if (gl_FrontFacing)
    {
        FragColor = vA;
    }
    else
    {
        FragColor = vB;
    }
}

struct SPIRV_Cross_Input
{
    @location(0) vA : vec4f,
    @location(1) vB : vec4f,
    @builtin(front_facing) gl_FrontFacing : bool,
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
    gl_FrontFacing = stage_input.gl_FrontFacing;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
