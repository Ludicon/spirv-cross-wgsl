const _16 : array<f32, 4> = array<f32, 4>(1.0f, 4.0f, 3.0f, 2.0f);

struct Foo
{
    a : f32,
    b : f32,
}

const _28 : array<Foo, 2> = array<Foo, 2>(Foo(10.0f, 20.0f), Foo(30.0f, 40.0f));

var<private> FragColor : vec4f;
var<private> line : i32;

fn frag_main()
{
    FragColor = vec4f(_16[line]);
    FragColor += vec4f(_28[line].a * _28[1 - line].a);
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) line : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    line = stage_input.line;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
