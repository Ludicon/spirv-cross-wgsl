struct VSOut
{
    a : f32,
}

var<private> _entryPointOutput : VSOut;
var<private> gl_Position : vec4f;

fn vert_main()
{
    _entryPointOutput.a = 40.0f;
    gl_Position = vec4f(1.0f);
}

struct SPIRV_Cross_Output
{
    @location(0) _entryPointOutput_a : f32,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main() -> SPIRV_Cross_Output
{
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._entryPointOutput_a = _entryPointOutput.a;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
