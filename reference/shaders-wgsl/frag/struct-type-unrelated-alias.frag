struct T
{
    a : f32,
}

struct T_1
{
    b : f32,
}

var<private> FragColor : f32;

fn frag_main()
{
    var foo : T;
    foo.a = 10.0f;
    var bar : T_1;
    bar.b = 20.0f;
    FragColor = foo.a + bar.b;
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
