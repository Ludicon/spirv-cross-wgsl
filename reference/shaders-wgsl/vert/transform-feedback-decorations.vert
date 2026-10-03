struct VertOut
{
    vBar : vec4f,
}

var<private> gl_Position : vec4f;
var<private> vFoo : vec4f;
var<private> _22 : VertOut;

fn vert_main()
{
    gl_Position = vec4f(1.0f);
    vFoo = vec4f(3.0f);
    _22.vBar = vec4f(5.0f);
}

struct SPIRV_Cross_Output
{
    @location(0) vFoo : vec4f,
    @location(1) _22_vBar : vec4f,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main() -> SPIRV_Cross_Output
{
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.vFoo = vFoo;
    stage_output._22_vBar = _22.vBar;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
