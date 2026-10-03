struct VSInput
{
    position : vec4f,
}

struct VSOutput
{
    position : vec4f,
}

struct VSOutput_1
{
    empty_struct_member : i32,
}

var<private> position : vec4f;
var<private> gl_Position : vec4f;
var<private> _entryPointOutput : VSOutput_1;

fn _main(_input : VSInput) -> VSOutput
{
    var _out : VSOutput;
    _out.position = _input.position;
    return _out;
}

fn vert_main()
{
    var _input : VSInput;
    _input.position = position;
    let param = _input;
    gl_Position = _main(param).position;
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
