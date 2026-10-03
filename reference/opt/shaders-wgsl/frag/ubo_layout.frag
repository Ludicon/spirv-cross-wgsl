struct Str
{
    foo : mat4x4f,
}

struct UBO1
{
    foo : Str,
}

struct Str_1
{
    foo : mat4x4f,
}

struct UBO2
{
    foo : Str_1,
}

@group(0) @binding(0) var<uniform> ubo1 : UBO1;
@group(0) @binding(1) var<uniform> ubo0 : UBO2;

var<private> FragColor : vec4f;

fn frag_main()
{
    FragColor = vec4f(ubo1.foo.foo[0][0], ubo1.foo.foo[1][0], ubo1.foo.foo[2][0], ubo1.foo.foo[3][0]) + ubo0.foo.foo[0];
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
