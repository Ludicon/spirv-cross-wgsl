var<private> vInput1 : vec4f;
var<private> gl_Position : vec4f;

fn vert_main()
{
    gl_Position = vec4f(10.0f) + vInput1;
}

struct SPIRV_Cross_Input
{
    @location(1) vInput1 : vec4f,
}

struct SPIRV_Cross_Output
{
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vInput1 = stage_input.vInput1;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
