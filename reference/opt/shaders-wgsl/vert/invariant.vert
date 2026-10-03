var<private> gl_Position : vec4f;
var<private> vInput0 : vec4f;
var<private> vInput1 : vec4f;
var<private> vInput2 : vec4f;
var<private> vColor : vec4f;

fn vert_main()
{
    var _20 : vec4f = vInput1 * vInput2;
    var _21 : vec4f = vInput0 + _20;
    gl_Position = _21;
    var _27 : vec4f = vInput0 - vInput1;
    var _29 : vec4f = _27 * vInput2;
    vColor = _29;
}

struct SPIRV_Cross_Input
{
    @location(0) vInput0 : vec4f,
    @location(1) vInput1 : vec4f,
    @location(2) vInput2 : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) vColor : vec4f,
    @builtin(position) @invariant gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vInput0 = stage_input.vInput0;
    vInput1 = stage_input.vInput1;
    vInput2 = stage_input.vInput2;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.vColor = vColor;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
