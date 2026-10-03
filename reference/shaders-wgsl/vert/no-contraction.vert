var<private> vA : vec4f;
var<private> vB : vec4f;
var<private> vC : vec4f;
var<private> gl_Position : vec4f;

fn vert_main()
{
    let mul = vA * vB;
    let add = vA + vB;
    let sub = vA - vB;
    let mad = (vA * vB) + vC;
    let summed = ((mul + add) + sub) + mad;
    gl_Position = summed;
}

struct SPIRV_Cross_Input
{
    @location(0) vA : vec4f,
    @location(1) vB : vec4f,
    @location(2) vC : vec4f,
}

struct SPIRV_Cross_Output
{
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    vC = stage_input.vC;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
