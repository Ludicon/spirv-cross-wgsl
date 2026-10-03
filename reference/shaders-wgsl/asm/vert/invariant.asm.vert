var<private> gl_Position : vec4f;

fn _main() -> vec4f
{
    return vec4f(1.0f);
}

fn vert_main()
{
    let _17 = _main();
    gl_Position = _17;
}

struct SPIRV_Cross_Output
{
    @builtin(position) @invariant gl_Position : vec4f,
}

@vertex
fn main() -> SPIRV_Cross_Output
{
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
