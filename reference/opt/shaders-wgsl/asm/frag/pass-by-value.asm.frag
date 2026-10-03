struct Registers
{
    foo : f32,
}

@group(0) @binding(0) var<uniform> registers : Registers;

var<private> FragColor : f32;

fn frag_main()
{
    FragColor = 10.0f + registers.foo;
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
