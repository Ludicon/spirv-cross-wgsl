var<private> gl_Position : vec4f;

fn vert_main()
{
    gl_Position = vec4f(1.0f);
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
