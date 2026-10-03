struct VSOutput
{
    empty_struct_member : i32,
}

var<private> position : vec4f;
var<private> gl_Position : vec4f;
var<private> _entryPointOutput : VSOutput;

fn vert_main()
{
    gl_Position = position;
}

struct SPIRV_Cross_Input
{
    @location(0) position : vec4f,
}

struct SPIRV_Cross_Output
{
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    position = stage_input.position;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
