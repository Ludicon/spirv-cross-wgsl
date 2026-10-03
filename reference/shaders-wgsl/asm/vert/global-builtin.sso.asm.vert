struct VSOut
{
    a : f32,
    pos : vec4f,
}

struct VSOut_1
{
    a : f32,
}

var<private> _entryPointOutput : VSOut_1;
var<private> gl_Position : vec4f;

fn _main() -> VSOut
{
    var vout : VSOut;
    vout.a = 40.0f;
    vout.pos = vec4f(1.0f);
    return vout;
}

fn vert_main()
{
    var flattenTemp : VSOut = _main();
    _entryPointOutput.a = flattenTemp.a;
    gl_Position = flattenTemp.pos;
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
