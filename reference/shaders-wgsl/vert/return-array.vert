var<private> vInput0 : vec4f;
var<private> vInput1 : vec4f;
var<private> gl_Position : vec4f;

fn test() -> array<vec4f, 2>
{
    return array<vec4f, 2>(vec4f(10.0f), vec4f(20.0f));
}

fn test2() -> array<vec4f, 2>
{
    var foobar : array<vec4f, 2>;
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    return foobar;
}

fn vert_main()
{
    gl_Position = test()[0] + test2()[1];
}

struct SPIRV_Cross_Input
{
    @location(0) vInput0 : vec4f,
    @location(1) vInput1 : vec4f,
}

struct SPIRV_Cross_Output
{
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vInput0 = stage_input.vInput0;
    vInput1 = stage_input.vInput1;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
