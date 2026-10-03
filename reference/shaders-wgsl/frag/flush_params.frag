struct Structy
{
    c : vec4f,
}

var<private> FragColor : vec4f;

fn foo2(f : ptr<function, Structy>)
{
    (*f).c = vec4f(10.0f);
}

fn foo() -> Structy
{
    var param : Structy;
    foo2(&param);
    var f : Structy = param;
    return f;
}

fn frag_main()
{
    var s : Structy = foo();
    FragColor = s.c;
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
