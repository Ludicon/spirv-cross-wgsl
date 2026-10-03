var<private> foo : vec2f;
var<private> FooOut : f32;

fn frag_main()
{
    FooOut = foo.x + foo.y;
}

struct SPIRV_Cross_Input
{
    @location(0) foo : vec2f,
}

struct SPIRV_Cross_Output
{
    @location(0) FooOut : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    foo = stage_input.foo;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FooOut = FooOut;
    return stage_output;
}
